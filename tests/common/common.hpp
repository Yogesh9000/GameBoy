#pragma once
#include <gtest/gtest.h>

#include <format>
#include <iostream>

#include "cpu.hpp"

static bool operator==(const CpuState& a, const CpuState& b)
{
  return a.AF.low == b.AF.low && a.AF.high == b.AF.high && a.BC.low == b.BC.low
         && a.BC.high == b.BC.high && a.DE.low == b.DE.low
         && a.DE.high == b.DE.high && a.HL.low == b.HL.low
         && a.HL.high == b.HL.high && a.SP.reg == b.SP.reg
         && a.PC.reg == b.PC.reg;
}

static std::ostream& operator<<(std::ostream& os, const CpuState& state)
{
  std::cout << std::format(
      "{{A={:02X}, B={:02X}, C={:02X}, D={:02X}, E={:02X}, H={:02X}, L={:02X}, F={:02X}, SP={:02X}, PC={:04X}",
      state.AF.high, state.BC.high, state.BC.low, state.DE.high, state.DE.low,
      state.HL.high, state.HL.low, state.AF.low, state.SP.reg, state.PC.reg);
  return os;
}
