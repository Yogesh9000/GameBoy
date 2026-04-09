#include "mmu.hpp"
#include "logmanager.hpp"
#include <spdlog/spdlog.h>

MemoryManagementUnit::MemoryManagementUnit()
{
  _logger = LogManager::GetLogger("Mmu");
}

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
  if (addr < 0xFEA0 || addr > 0xFEFF)
  {
    // only log for invalid access outside forbidden region (0xFEA0 - 0xFEFF)
    LOG_WARN(_logger, "Read: No registered memory region found that contains address: " "{:#06X}", addr);
  }
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
  if (addr < 0xFEA0 || addr > 0xFEFF)
  {
    // only log for invalid access outside forbidden region (0xFEA0 - 0xFEFF)
    LOG_WARN(_logger, "Write(data: {:#04X}): No registered memory region found that contains address: {:#06X}", data, addr);
  }
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
  LOG_WARN(_logger, "Read: No registered memory region found that contains address: " "{:#06X}\n", addr);
  static std::uint8_t dummy;
  dummy = 0xFF;
  return dummy;
}

void MemoryManagementUnit::AddMemoryRange(
    std::shared_ptr<MemoryRange> memoryRange)
{
  _memoryRanges.emplace_back(std::move(memoryRange));
}
