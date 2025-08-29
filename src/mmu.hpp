#pragma once

#include <cstdint>
#include <format>
#include <memory>
#include <vector>
#include "common.hpp"
#include "memoryrange.hpp"
class MemoryManagementUnit
{
private:
  // utility
  [[nodiscard]]
  auto GetMemoryRange(std::uint16_t addr) const
  {
    return std::find_if(_memoryRanges.cbegin(), _memoryRanges.cend(),
        [=](const auto &range) { return range->Contains(addr); });
  }

  auto GetMemoryRange(std::uint16_t addr)
  {
    return std::find_if(_memoryRanges.begin(), _memoryRanges.end(),
        [=](const auto &range) { return range->Contains(addr); });
  }

public:
  // TODO: try eliminate checking if a memory range is present in every method

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const  // TODO: Do I need this
  {
    auto memoryRange = GetMemoryRange(addr);
    return memoryRange != _memoryRanges.cend();
  }

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const
  {
    auto memoryRange = GetMemoryRange(addr);
    // If we found a memory range read from it
    if (memoryRange != _memoryRanges.cend())
    {
      return memoryRange->get()->Read(addr);
    }

    // return garbage value if no memory range found that contains addr
    Warning(std::format(
        "Read: No registered memory region found that contains address: "
        "{:#06X}\n",
        addr));
    return 0xFF;
  }

  void Write(std::uint16_t addr, std::uint8_t data)
  {
    auto memoryRange = GetMemoryRange(addr);
    // if memory region found write to it
    if (memoryRange != _memoryRanges.end())
    {
      memoryRange->get()->Write(addr, data);
      return;
    }
    Warning(std::format(
        "Write: No registered memory region found that contains address: {:#06X}\n",
        addr));
  }

  void AddMemoryRange(std::shared_ptr<MemoryRange> memoryRange)
  {
    _memoryRanges.emplace_back(std::move(memoryRange));
  }

private:
  std::vector<std::shared_ptr<MemoryRange>> _memoryRanges;
};
