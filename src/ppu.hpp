#pragma once

#include "display.hpp"
#include "mmu.hpp"

class Ppu : public MemoryRange
{
private:
  constexpr static int LY_REGISTER_ADDRESS{0xFF44};

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
  void DrawCurrentLine();

private:
  std::uint8_t _ly;
  int _dotsThisLine;
  MemoryManagementUnit &_mmu;
  Display &_display;
};
