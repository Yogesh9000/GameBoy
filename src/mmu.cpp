#include "mmu.hpp"
#include <format>
#include "common.hpp"
#include <spdlog/spdlog.h>

auto MemoryManagementUnit::GetMemoryRange(std::uint16_t addr) const
{
  return std::find_if(_memoryRanges.cbegin(), _memoryRanges.cend(),
      [=](const auto &range) { return range->Contains(addr); });
}

auto MemoryManagementUnit::GetMemoryRange(std::uint16_t addr)
{
  return std::find_if(_memoryRanges.begin(), _memoryRanges.end(),
      [=](const auto &range) { return range->Contains(addr); });
}

bool MemoryManagementUnit::Contains(
    std::uint16_t addr) const  // TODO: Do I need this
{
  auto memoryRange = GetMemoryRange(addr);
  return memoryRange != _memoryRanges.cend();
}

std::uint8_t MemoryManagementUnit::Read(std::uint16_t addr) const
{
  auto memoryRange = GetMemoryRange(addr);
  // If we found a memory range read from it
  if (memoryRange != _memoryRanges.cend())
  {
    return memoryRange->get()->Read(addr);
  }

  // return garbage value if no memory range found that contains addr
  SPDLOG_WARN("Read: No registered memory region found that contains address: " "{:#06X}\n", addr);
  return 0xFF;
}

void MemoryManagementUnit::Write(std::uint16_t addr, std::uint8_t data)
{
  auto memoryRange = GetMemoryRange(addr);
  // if memory region found write to it
  if (memoryRange != _memoryRanges.end())
  {
    memoryRange->get()->Write(addr, data);
    return;
  }
  SPDLOG_WARN("Write(data: {:#04X}): No registered memory region found that contains address: {:#06X}", data, addr);
}

std::uint8_t& MemoryManagementUnit::Address(std::uint16_t addr)
{

  auto memoryRange = GetMemoryRange(addr);
  // If we found a memory range read from it
  if (memoryRange != _memoryRanges.cend())
  {
    return memoryRange->get()->Address(addr);
  }

  // return garbage value if no memory range found that contains addr
  SPDLOG_WARN("Read: No registered memory region found that contains address: " "{:#06X}\n", addr);
  static std::uint8_t dummy;
  dummy = 0xFF;
  return dummy;
}

void MemoryManagementUnit::AddMemoryRange(
    std::shared_ptr<MemoryRange> memoryRange)
{
  _memoryRanges.emplace_back(std::move(memoryRange));
}
