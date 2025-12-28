#pragma once
#include "memoryrange.hpp"

class Cpu;

enum InterruptType
{
  VBLANK,
  LCD,
  TIMER,
  SERIAL,
  JOYPAD
};

class Interrupt : public MemoryRange
{
public:
  explicit Interrupt();

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

  std::uint8_t& Address(std::uint16_t addr) override;

private:
  friend class Cpu;

  // This field determines if interrupts are enabled or disabled
  bool _ime;
  bool _enableRequested;
  // IE register: https://gbdev.io/pandocs/Interrupts.html#ffff--ie-interrupt-enable
  // IF register: https://gbdev.io/pandocs/Interrupts.html#ff0f--if-interrupt-flag
  std::uint8_t _ie;
  std::uint8_t _if;
};

