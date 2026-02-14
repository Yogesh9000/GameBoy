#pragma once

#include <cstdint>
#include <deque>

#include "bitutils.hpp"
#include "common.hpp"
#include "mmu.hpp"

class Fetcher
{
public:
  struct PixelFifoEntry
  {
    unsigned int x;
    unsigned int y;
    std::uint8_t color;
    std::uint8_t palette;
    std::uint8_t bgPriority;
  };

  enum class FetcherState : std::uint8_t
  {
    GetTile,
    GetTileData0,
    GetTileData1,
    PushPixel
  };

public:
  explicit Fetcher(
      MemoryManagementUnit &mmu, std::deque<PixelFifoEntry> &bgWinFiFo)
      : _mmu{mmu}, _bgWinFifo(bgWinFiFo)
  {
  }

  void Start()
  {
    _state = FetcherState::GetTile;
    _bgWinFifo.clear();
    _clockDivider = 2;
    _fetcherX = 0;
    _ly = _mmu.Read(LY_REGISTER_ADDRESS);
    _droppedInitialTile = false;
  }

  void Tick()
  {
    // Fetcher is two times slower than PPU, so we use _clockDivider to properly
    // sync Fetcher and PPU
    --_clockDivider;
    if (_clockDivider > 0)
    {
      return;
    }
    else
    {
      // Reset _clockDivider
      _clockDivider = 2;
    }
    auto scx = _mmu.Read(SCX_REGISTER_ADDRESS);
    auto scy = _mmu.Read(SCY_REGISTER_ADDRESS);
    auto lcdc = _mmu.Read(LCDC_REGISTER_ADDRESS);
    switch (_state)
    {
      case FetcherState::GetTile:
      {
        // TODO: Also consider window and sprites
        auto tileToFetchX = ((scx / 8U) + _fetcherX) & 0x1FU;  // 0x1F = 31
        auto tileToFetchY = (((_ly + scy) & 0xFFU) / 8);         // 0xFF = 255
        auto bgtileIdx =
            (BG_WIN_TILEMAP_ROW_SIZE * tileToFetchY) + tileToFetchX;
        if (BitUtils::Test<3>(
                lcdc))  // if LCDC bit 3 is set use tile map 0x9C00-0x9FFF
        {
          _tileIdx = _mmu.Read(
              static_cast<std::uint16_t>(BG_WIN_TILEMAP_ADDRESS1 + bgtileIdx));
        }
        else
        {
          _tileIdx = _mmu.Read(
              static_cast<std::uint16_t>(BG_WIN_TILEMAP_ADDRESS0 + bgtileIdx));
        }
        _state = FetcherState::GetTileData0;
        break;
      }
      case FetcherState::GetTileData0:
      {
        // TODO: Take Window into consideration
        std::uint16_t tileDataLowAddress{};
        if (BitUtils::Test<4>(lcdc))  // if lcdc bit 4 is set use tile data map
                                      // starting at 0x8000 - 0x8FFF
        {
          tileDataLowAddress = static_cast<std::uint16_t>(
              BG_WIN_TILEDATA_ADDRESS1 + (_tileIdx * TILE_DATA_SIZE) + (((_ly + scy) & 0x7U) * 2));
        }
        else  // else use tile data map at 0x8800 - 0x97FF
        {
          tileDataLowAddress = static_cast<std::uint16_t>(
              BG_WIN_TILEDATA_ADDRESS0
              + (static_cast<int>(_tileIdx)
                  * static_cast<int>(TILE_DATA_SIZE)) + static_cast<int>(((_ly + scy) & 0x7U)));
        }
        _tileDataLow = _mmu.Read(tileDataLowAddress);
        _state = FetcherState::GetTileData1;
        break;
      }
      case FetcherState::GetTileData1:
      {
        // TODO: Take Window into consideration
        std::uint16_t tileDataHighAddress{};
        if (BitUtils::Test<4>(lcdc))  // if lcdc bit 4 is set use tile data map
                                      // starting at 0x8000 - 0x8FFF
        {
          tileDataHighAddress = static_cast<std::uint16_t>(
              BG_WIN_TILEDATA_ADDRESS1 + (_tileIdx * TILE_DATA_SIZE) + (((_ly + scy) & 0x7U) * 2) + 1);
        }
        else  // else use tile data map at 0x8800 - 0x97FF
        {
          tileDataHighAddress = static_cast<std::uint16_t>(
              BG_WIN_TILEDATA_ADDRESS0
              + (static_cast<int>(_tileIdx)
                  * static_cast<int>(TILE_DATA_SIZE)) + static_cast<int>((((_ly + scy) & 0x7U) * 2)) + 1);
        }
        _tileDataHigh = _mmu.Read(tileDataHighAddress);
        if (_bgWinFifo.size() <= 8)
        {
          PushPixelToBgWinFifo();
          if (_droppedInitialTile)
          {
            ++_fetcherX;
          }
          else
          {
            _droppedInitialTile = true;
          }
          _state = FetcherState::GetTile;
        }
        else
        {
          _state = FetcherState::PushPixel;
        }
        break;
      }
      case FetcherState::PushPixel:
      {
        // TODO:
        if (_bgWinFifo.size() <= 8)
        {
          PushPixelToBgWinFifo();
          if (_droppedInitialTile)
          {
            ++_fetcherX;
          }
          else
          {
            _droppedInitialTile = true;
          }
          _state = FetcherState::GetTile;
        }
        break;
      }
    }
  }

private:
  void PushPixelToBgWinFifo()
  {
    if (_bgWinFifo.size() > 8)
    {
      return;
    }
    for (unsigned int i{0}; i < 8; ++i)
    {
      unsigned int bitIndex = 7U - i;
      unsigned int bit1 =
          (static_cast<unsigned int>(_tileDataLow) >> bitIndex) & 1U;
      unsigned int bit2 =
          (static_cast<unsigned int>(_tileDataHigh) >> bitIndex) & 1U;
      auto colorId = static_cast<std::uint8_t>((bit2 << 1U) | bit1);
      unsigned int screenX = (_fetcherX * 8) + i;
      auto entry = PixelFifoEntry{
          .x = screenX,
          .y = _ly,
          .color = colorId,
          .palette = {},
          .bgPriority = {}
      };
      _bgWinFifo.emplace_back(entry);
    }
  }

  MemoryManagementUnit &_mmu;
  std::deque<PixelFifoEntry> &_bgWinFifo;
  int _clockDivider{};
  unsigned int _fetcherX{};
  unsigned int _ly{};
  FetcherState _state{};
  // Index of the tile to fetch from background/window tile map
  unsigned int _tileIdx{};
  std::uint8_t _tileDataLow{};
  std::uint8_t _tileDataHigh{};
  bool _droppedInitialTile{};
};
