#include "timer.hpp"

#include <stdexcept>

#include "common.hpp"
#include "logmanager.hpp"
#include "mmu.hpp"

Timer::Timer(MemoryManagementUnit &mmu) : _mmu(mmu)
{
  _logger = LogManager::GetLogger("timer");
}

void Timer::Tick(int cycles)
{
  UpdateTimers(cycles);
}

bool Timer::Contains(std::uint16_t addr) const
{
  return (addr == TIMA) || (addr == TMA) || (addr == TAC);
}

std::uint8_t Timer::Read(std::uint16_t addr) const
{
  if (addr == TIMA)
  {
    return _tima;
  }
  else if (addr == TMA)
  {
    return _tma;
  }
  else if (addr == TAC)
  {
    return _tac;
  }

  LOG_TRACE(
      _logger, "Trying to read invalid address: {}, returning 0xFF", addr);
  return 0xFF;
}

void Timer::Write(std::uint16_t addr, std::uint8_t data)
{
  if (addr == DIV)
  {
    _div = 0;
    return;
  }
  else if (addr == TIMA)
  {
    _tima = data;
  }
  else if (addr == TMA)
  {
    _tma = data;
  }
  else if (addr == TAC)
  {
    _tac = data;
  }
  LOG_TRACE(_logger, "Ignoring write to invalid address: {}", addr);
}

std::uint8_t &Timer::Address(std::uint16_t addr)
{
  if (addr == DIV)
  {
    return _div;
  }
  else if (addr == TIMA)
  {
    return _tima;
  }
  else if (addr == TMA)
  {
    return _tma;
  }
  else if (addr == TAC)
  {
    return _tac;
  }

  // if address is not presesnt, return dummy value
  LOG_TRACE(
      _logger, "Trying to read invalid address: {}, returning 0xFF", addr);
  static std::uint8_t dummy;
  dummy = 0xFF;
  return dummy;
}

void Timer::UpdateDividerRegister(int cycles)
{
  _dividerCounter += cycles;
  if (_dividerCounter > 255)
  {
    _dividerCounter = 0;
    ++_div;
  }
}

void Timer::UpdateTimers(int cycles)
{
  UpdateDividerRegister(cycles);

  // the clock must be enabled to update the clock
  if (IsClockEnabled())
  {
    _timerCounter += cycles;

    // enough cpu clock cycles have happened to update the timer
    if (_timerCounter >= GetClockFreq())
    {
      if (_tima >= 255)
      {
        _tima = _tma;
        _mmu.RequestInterrupt(2);
      }
      else
      {
        ++_tima;
      }
    }
  }
}

int Timer::GetClockFreq() const
{
  auto freq = _tac & 0x03U;
  switch (freq)
  {
    case 0:
      return 1024;
      break;
    case 1:
      return 16;
      break;
    case 2:
      return 64;
      break;
    case 3:
      return 256;
      break;
    default:
      throw std::runtime_error("Unknown Timer Freq");
      break;
  }
}

bool Timer::IsClockEnabled()
{
  constexpr uint16_t TAC{0xFF07U};
  return (_mmu.Read(TAC) & (1U << 2U)) != 0;
}
