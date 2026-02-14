#pragma once

#include "../display.hpp"
#include "../fetcher.hpp"
#include "ppuphase.hpp"

class PixelRendering : public PpuPhase
{
public:
  PixelRendering(MemoryManagementUnit &mmu, Display &display)
      : _mmu(mmu), _display(display), _fetcher(mmu, _bgWinFifo)
  {
  }

  void Start() override
  {
    _fetcher.Start();
    auto scx = _mmu.Read(SCX_REGISTER_ADDRESS);
    _pixelsDrawn = 0;
    _pixelsToDrop =
        8U + (scx & 0b111U);  // Initial tile that's fetched is dropped +
                              // (scroll register & 7) tiles are dropped
    if (_pixelsToDrop > 8)
    {
      // TODO: add logging
    }
  }

  bool Tick() override
  {
    _fetcher.Tick();
    PushPixelToDisplay();
    // _pixelsDrawn is incremented each time we successfully push out a pixel,
    // if we reach _pixelsDrawn > 160 then we have already pushed out 160 pixels
    // (width of the screen) and we cannot push any more pixels to the screen in
    // current scanline
    return _pixelsDrawn < 160;
  }

private:
  void PushPixelToDisplay()
  {
    if (_bgWinFifo.size() <= 0)
    {
      return;
    }
    Fetcher::PixelFifoEntry entry = _bgWinFifo.front();
    _bgWinFifo.pop_front();
    if (_pixelsToDrop > 0)
    {
      --_pixelsToDrop;
      return;
    }

    if (_pixelsDrawn >= 160)
    {
      return;
    }

    Color color{};
    switch (entry.color)
    {
      case 0b00:
        color = {.red = 0x34, .green = 0x3d, .blue = 0x37, .alpha = 0xff};
        break;
      case 0b01:
        color = {.red = 0x55, .green = 0x64, .blue = 0x5a, .alpha = 0xff};
        break;
      case 0b10:
        color = {.red = 0x8b, .green = 0xa3, .blue = 0x94, .alpha = 0xff};
        break;
      case 0b11:
        color = {.red = 0xe0, .green = 0xf0, .blue = 0xe7, .alpha = 0xff};
        break;
      default:
        break;
    }
    _display.Draw(Pixel(entry.x, entry.y, color));
    ++_pixelsDrawn;  // Increment the number of pixels pushed to screen
  }

private:
  MemoryManagementUnit &_mmu;
  Display &_display;
  Fetcher _fetcher;
  std::deque<Fetcher::PixelFifoEntry> _bgWinFifo;
  unsigned int _pixelsDrawn{};
  unsigned int _pixelsToDrop{};
};
