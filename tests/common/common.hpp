#pragma once

#include "cpu.hpp"

#include <gtest/gtest.h>

#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "concretememoryrange.hpp"
#include "mmu.hpp"

using json = nlohmann::json;

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

static CpuState CreateStateFromJson(const auto& cpuState)
{
  CpuState state{};
  state.AF.low = cpuState["f"];
  state.AF.high = cpuState["a"];
  state.BC.low = cpuState["c"];
  state.BC.high = cpuState["b"];
  state.DE.low = cpuState["e"];
  state.DE.high = cpuState["d"];
  state.HL.low = cpuState["l"];
  state.HL.high = cpuState["h"];
  state.SP.reg = cpuState["sp"];
  state.PC.reg = cpuState["pc"];
  return state;
}

static CpuState CreateInitialStateFromJson(const auto& test)
{
  return CreateStateFromJson(test["initial"]);
}

static CpuState CreateFinalStateFromJson(const auto& test)
{
  return CreateStateFromJson(test["final"]);
}

template <typename BusType>
static MemoryManagementUnit CreateAndInitializeBusFromJson(const auto& test)
{
  MemoryManagementUnit mmu{};
  mmu.AddMemoryRange(std::make_shared<BusType>(0x10000, 0x00));

  for (const auto& memState : test["ram"])
  {
    uint16_t loc = memState[0];
    uint8_t data = memState[1];
    mmu.Write(loc, data);
  }
  return mmu;
}

template <typename BusType = ConcreteMemoryRange>
static void TestInstruction(int test_num, bool extended = false)
{
  std::string filePath;
  if (!extended)
  {
    filePath = std::format("v1/{:02x}.json", test_num);
  }
  else
  {
    filePath = std::format("v1/cb {:02x}.json", test_num);
  }
  std::ifstream file{filePath};
  ASSERT_TRUE(file.is_open());
  json tests = json::parse(file);
  for (const auto& test : tests)
  {
    auto initialState = CreateInitialStateFromJson(test);
    auto finalState = CreateFinalStateFromJson(test);
    auto mmu = CreateAndInitializeBusFromJson<BusType>(test["initial"]);
    Cpu cpu{initialState, mmu};
    cpu.Tick();
    ASSERT_EQ(cpu.GetCpuState(), finalState)
        << std::format("Test Name: {} [Final State does not match]",
               static_cast<std::string>(test["name"]));
    for (const auto& memState : test["final"]["ram"])
    {
      uint16_t loc = memState[0];
      uint8_t data = memState[1];
      ASSERT_EQ(mmu.Read(loc), data)
          << std::format("Test Name: {} [Memory State Does not Match]",
                 static_cast<std::string>(test["name"]));
    }
  }
}
