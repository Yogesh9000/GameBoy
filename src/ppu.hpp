#pragma once

#include <cstdint>
#include "concretememoryrange.hpp"
#include "display.hpp"
#include "mmu.hpp"
#include "phases/hblank.hpp"
#include "phases/oamsearch.hpp"
#include "phases/pixelrendering.hpp"
#include "phases/ppuphase.hpp"
#include "phases/vblank.hpp"

class Ppu : public MemoryRange
{
public:
  enum class PpuMode : std::uint8_t
  {
    OamSearch,
    PixelRendering,
    HBlank,
    VBlank
  };

public:
  Ppu(MemoryManagementUnit &mmu, Display &display);

  void Tick();

public:
  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

private:
  ConcreteMemoryRange _oamRam;
  std::uint8_t _ly;
  std::uint8_t _lcdc;
  std::uint8_t _scx;
  std::uint8_t _scy;
  MemoryManagementUnit &_mmu;
  Display &_display;
  OamSearch _oamPhase;
  PixelRendering _pixelrenderingPhase;
  HBlank _hblankPhase;
  VBlank _vblankPhase;
  PpuPhase *_phase;
  PpuMode _mode;

  unsigned int _dotsThisLine{};
};
