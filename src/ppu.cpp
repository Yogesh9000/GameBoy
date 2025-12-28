#include "ppu.hpp"

#include "common.hpp"
#include "phases/hblank.hpp"
#include "phases/oamsearch.hpp"
#include "phases/vblank.hpp"

Ppu::Ppu(MemoryManagementUnit &mmu, Display &display)
    : _oamRam{OAM_SIZE, OAM_START_ADDRESS},
      _ly(0),
      _mmu(mmu),
      _display(display),
      _oamPhase(_mmu),
      _pixelrenderingPhase(_mmu, _display),
      _hblankPhase(),
      _vblankPhase(_mmu, _ly),
      _phase(&_oamPhase),
      _mode(PpuMode::OamSearch)
{
  _phase->Start();
}

void Ppu::Tick()
{
  ++_dotsThisLine;
  if (_phase && _phase->Tick())
  {
  }
  else
  {
    // switch phase here
    _phase = nullptr;
    switch (_mode)
    {
      case PpuMode::OamSearch:
      {
        _mode = PpuMode::PixelRendering;
        _phase = &_pixelrenderingPhase;
        _pixelrenderingPhase.Start();
        break;
      }
      case PpuMode::PixelRendering:
      {
        _mode = PpuMode::HBlank;
        _phase = &_hblankPhase;
        auto hblankLength = 456 - _dotsThisLine;
        _hblankPhase.SetHBlankModeLength(hblankLength);
        _phase->Start();
        break;
      }
      case PpuMode::HBlank:
      {
        ++_ly;
        if (_ly < 144)
        {
          _mode = PpuMode::OamSearch;
          _phase = &_oamPhase;
          _phase->Start();
        }
        else
        {
          _mode = PpuMode::VBlank;
          _phase = &_vblankPhase;
          _phase->Start();
        }
        break;
      }
      case PpuMode::VBlank:
      {
        _display.UpdateFrame();
        _ly = 0;
        _mode = PpuMode::OamSearch;
        _phase = &_oamPhase;
        _oamPhase.Start();
        break;
      }
    }
  }
  if (_dotsThisLine >= MAX_DOTS_PER_SCANLINE)
  {
    _dotsThisLine = 0;
  }
}

bool Ppu::Contains(std::uint16_t addr) const
{
  return addr == LY_REGISTER_ADDRESS || addr == LCDC_REGISTER_ADDRESS
         || addr == SCX_REGISTER_ADDRESS || addr == SCY_REGISTER_ADDRESS
         || _oamRam.Contains(addr);
}

std::uint8_t Ppu::Read(std::uint16_t addr) const
{
  if (addr == LY_REGISTER_ADDRESS)
  {
    return _ly;
  }
  else if (addr == LCDC_REGISTER_ADDRESS)
  {
    return _lcdc;
  }
  else if (addr == SCX_REGISTER_ADDRESS)
  {
    return _scx;
  }
  else if (addr == SCY_REGISTER_ADDRESS)
  {
    return _scy;
  }
  else if (_oamRam.Contains(addr))
  {
    return _oamRam.Read(addr);
  }
  // if address is not presesnt, return dummy value
  return 0xFF;
}

void Ppu::Write(std::uint16_t addr, std::uint8_t data)
{
  if (addr == LCDC_REGISTER_ADDRESS)
  {
    _lcdc = data;
  }
  else if (addr == SCX_REGISTER_ADDRESS)
  {
    _scx = data;
  }
  else if (addr == SCY_REGISTER_ADDRESS)
  {
    _scy = data;
  }
  else if (_oamRam.Contains(addr))
  {
    _oamRam.Write(addr, data);
  }
}

std::uint8_t& Ppu::Address(std::uint16_t addr)
{
  if (addr == LY_REGISTER_ADDRESS)
  {
    return _ly;
  }
  else if (addr == LCDC_REGISTER_ADDRESS)
  {
    return _lcdc;
  }
  else if (addr == SCX_REGISTER_ADDRESS)
  {
    return _scx;
  }
  else if (addr == SCY_REGISTER_ADDRESS)
  {
    return _scy;
  }
  else if (_oamRam.Contains(addr))
  {
    return _oamRam.Address(addr);
  }

  // if address is not presesnt, return dummy value
  static std::uint8_t dummy;
  dummy = 0xFF;
  return dummy;
}
