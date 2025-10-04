#pragma once

#include <cstdint>
#include "ppuphase.hpp"
#include "../mmu.hpp"
#include "../common.hpp"


class VBlank : public PpuPhase
{
public:
  explicit VBlank(MemoryManagementUnit &mmu, std::uint8_t &ly) : _mmu(mmu), _ly(ly)
  {
  }

  void Start() override
  {
    _x = 0;
  }

  bool Tick() override
  {
    ++_x;
    if (_x >= 456)
    {
      _x = 0;
      ++_ly;
    }
    return _ly > 153;
  }

private:
  MemoryManagementUnit _mmu;
  unsigned int _x{};
  std::uint8_t &_ly;
};
