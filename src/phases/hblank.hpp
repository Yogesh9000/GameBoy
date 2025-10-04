#pragma once

#include "ppuphase.hpp"

class HBlank : public PpuPhase
{
public:
  void SetHBlankModeLength(unsigned int modeLength)
  {
    _modeLength = modeLength;
  }

  void Start() override
  {
  }

  bool Tick() override
  {
    --_modeLength;
    return _modeLength <= 0;
  }

private:
  unsigned int _modeLength{};
};
