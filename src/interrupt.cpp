#include "interrupt.hpp"
#include "spdlog/spdlog.h"
#include "common.hpp"

Interrupt::Interrupt()
  : _ime{ false }, _enableRequested{ false }, _ie{ 0 }, _if{ 0 }
{
}

[[nodiscard]]
bool Interrupt::Contains(std::uint16_t addr) const
{
  return addr == INTERRUPT_ENABLE || addr == INTERRUPT_FLAG;
}

[[nodiscard]]
std::uint8_t Interrupt::Read(std::uint16_t addr) const
{
  if (addr == INTERRUPT_ENABLE)
  {
    return _ie;
  }
  else if (addr == INTERRUPT_FLAG)
  {
    return _if;
  }

  SPDLOG_TRACE("Trying to read invalid address: {}, returning 0xFF", addr);
  return 0xFF;
}

void Interrupt::Write(std::uint16_t addr, std::uint8_t data)
{
  if (addr == INTERRUPT_ENABLE)
  {
    _ie = data;
    return;
  }
  else if (addr == INTERRUPT_FLAG)
  {
    _if = data;
    return;
  }
  SPDLOG_TRACE("Ignoring write to invalid address: {}", addr);
}