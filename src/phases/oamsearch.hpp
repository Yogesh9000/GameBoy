#pragma once

#include <array>
#include <cstdint>

#include "../bitutils.hpp"
#include "../common.hpp"
#include "../mmu.hpp"
#include "ppuphase.hpp"

class OamSearch : public PpuPhase
{
private:
  enum class OamSearchPhase : std::uint8_t
  {
    ReadingTileY,
    ReadingTileX
  };

  struct OamEntryPosition
  {
    int x;
    int y;
    std::uint16_t address;
  };

public:
  explicit OamSearch(MemoryManagementUnit &mmu)
      : _mmu(mmu),
        _phase{OamSearch::OamSearchPhase::ReadingTileY},
        _tilesScanned(0),
        _tileY(0),
        _tileX(0),
        _spritesOnCurrentLine{},
        _nextSprite(0)
  {
  }

  void Start() override
  {
    _phase = OamSearchPhase::ReadingTileY;
    _tilesScanned = 0;
    _tileY = 0;
    _tileX = 0;
    _spritesOnCurrentLine.fill({.x = -1, .y = -1, .address = 0});
    _nextSprite = 0;
  }

  bool Tick() override
  {
    auto tileAddress = static_cast<std::uint16_t>(
        OAM_START_ADDRESS + (OAM_ENTRY_SIZE * _tilesScanned));

    switch (_phase)
    {
      case OamSearchPhase::ReadingTileY:
      {
        _tileY = _mmu.Read(tileAddress);
        _phase = OamSearchPhase::ReadingTileX;
        break;
      }
      case OamSearchPhase::ReadingTileX:
      {
        _tileX = _mmu.Read(static_cast<std::uint16_t>(tileAddress + 1));
        int ly = _mmu.Read(LY_REGISTER_ADDRESS);
        int objHeight =
            BitUtils::Test<2>(_mmu.Read(LCDC_REGISTER_ADDRESS)) ? 16 : 8;
        if ((_tileY <= (ly + 16)) && ((_tileY + objHeight) > (ly + 16)))
        {
          TryAddSpriteEntry(_tileX, _tileY, tileAddress);
        }
        _phase = OamSearchPhase::ReadingTileY;
        ++_tilesScanned;
        break;
      }
    }
    return _tilesScanned < 40;
  }

private:
  void TryAddSpriteEntry(int x, int y, std::uint16_t address)
  {
    if (_nextSprite < 10)
    {
      _spritesOnCurrentLine[_nextSprite].x = x;
      _spritesOnCurrentLine[_nextSprite].y = y;
      _spritesOnCurrentLine[_nextSprite].address = address;
    }
  }

  constexpr static std::uint16_t OAM_START_ADDRESS{0xFE00};
  constexpr static int OAM_ENTRY_SIZE{4};  // in bytes
  MemoryManagementUnit &_mmu;
  OamSearchPhase _phase;
  int _tilesScanned;
  std::uint8_t _tileY;
  std::uint8_t _tileX;
  std::array<OamEntryPosition, 10> _spritesOnCurrentLine;
  unsigned long _nextSprite;
};
