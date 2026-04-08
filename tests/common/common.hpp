#pragma once

#include "cpu.hpp"


bool operator==(const CpuState& a, const CpuState& b);
std::ostream& operator<<(std::ostream& os, const CpuState& state);


CpuState CreateStateFromJson(const auto& cpuState);

CpuState CreateInitialStateFromJson(const auto& test);
CpuState CreateFinalStateFromJson(const auto& test);
MemoryManagementUnit CreateAndInitializeBusFromJson(const auto& test);
void TestInstruction(int test_num, bool extended = false);
