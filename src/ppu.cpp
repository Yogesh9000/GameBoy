#include "ppu.hpp"

#include "common.hpp"

Ppu::Ppu(MemoryManagementUnit &mmu, Display &display)
    : _oamRam{OAM_SIZE, OAM_START_ADDRESS},
      _ly(0),
      _mmu(mmu),
      _display(display),
      _oamPhase(mmu),
      _phase(&_oamPhase)
{
  _phase->Start();
}

void Ppu::Tick()
{
  if (_phase && _phase->Tick())
  {
  }
  else
  {
    // switch phase here
    _phase = nullptr;
  }
}

bool Ppu::Contains(std::uint16_t addr) const
{
  return addr == LY_REGISTER_ADDRESS || _oamRam.Contains(addr);
}

std::uint8_t Ppu::Read(std::uint16_t addr) const
{
  if (addr == LY_REGISTER_ADDRESS)
  {
    return _ly;
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
  if (_oamRam.Contains(addr))
  {
    _oamRam.Write(addr, data);
  }
}
