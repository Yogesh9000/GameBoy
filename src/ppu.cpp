#include "ppu.hpp"

Ppu::Ppu(MemoryManagementUnit &mmu, Display &display)
    : _ly(0), _dotsThisLine(0), _mmu(mmu), _display(display)
{
}

void Ppu::Tick()
{
  _dotsThisLine += 4;
  if (_dotsThisLine >= 204)
  {
    _dotsThisLine = 0;
    if (_ly <= 143)
    {
      DrawCurrentLine();
    }
    if (_ly == 144)
    {
      _display.UpdateFrame();
    }
    ++_ly;
    if (_ly >= 153)
    {
      _ly = 0;
    }
  }
}

bool Ppu::Contains(std::uint16_t addr) const
{
  return addr == LY_REGISTER_ADDRESS;
}

std::uint8_t Ppu::Read(std::uint16_t addr) const
{
  if (addr == LY_REGISTER_ADDRESS)
  {
    return _ly;
  }
  // if address is not presesnt, return dummy value
  return 0xFF;
}

void Ppu::Write(std::uint16_t addr, std::uint8_t data)
{
  // Nothing to do here
}

void Ppu::DrawCurrentLine()
{
  constexpr int BACKGROUND_TILE_MAP_ADDRESS{0x9800};
  constexpr int TILE_DATA_ADDRESS{0x8000};
  constexpr int DISPLAY_WIDTH{160};

  int tileRow = _ly / 8;
  int pixelRowInTile = _ly % 8;

  for (int tileX = 0; tileX < DISPLAY_WIDTH / 8; ++tileX)
  {
    // Which tile is used at this position
    int mapIndex = tileRow * 32 + tileX;
    int tileIndex = _mmu.Read(BACKGROUND_TILE_MAP_ADDRESS + mapIndex);

    // Each tile = 16 bytes
    int tileAddr = TILE_DATA_ADDRESS + (tileIndex * 16);

    // Row inside tile = 2 bytes
    int lineAddr = tileAddr + (pixelRowInTile * 2);
    uint8_t byte1 = _mmu.Read(lineAddr);
    uint8_t byte2 = _mmu.Read(lineAddr + 1);

    for (int j = 0; j < 8; ++j)
    {
      int bitIndex = 7 - j;
      int bit1 = (byte1 >> bitIndex) & 1;
      int bit2 = (byte2 >> bitIndex) & 1;
      int colorId = (bit2 << 1) | bit1;

      Color color{};
      switch (colorId)
      {
        case 0b00:
          color = {.red=0x34, .green=0x3d, .blue=0x37, .alpha=0xff};
          break;
        case 0b01:
          color = {.red=0x55, .green=0x64, .blue=0x5a, .alpha=0xff};
          break;
        case 0b10:
          color = {.red=0x8b, .green=0xa3, .blue=0x94, .alpha=0xff};
          break;
        case 0b11:
          color = {.red=0xe0, .green=0xf0, .blue=0xe7, .alpha=0xff};
          break;
      }

      unsigned int screenX = tileX * 8 + j;
      auto pixel = Pixel{.x = screenX, .y = _ly, .color = color};
      _display.Draw(pixel);
    }
  }
}
