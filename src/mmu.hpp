#pragma once

#include <spdlog/logger.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "memoryrange.hpp"
class MemoryManagementUnit
{
private:
  // utility
  [[nodiscard]]
  auto GetMemoryRange(std::uint16_t addr) const;

  auto GetMemoryRange(std::uint16_t addr);

public:
  MemoryManagementUnit();

  // TODO: try eliminate checking if a memory range is present in every method

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const;  // TODO: Do I need this

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const;

  void Write(std::uint16_t addr, std::uint8_t data);

  std::uint8_t &Address(std::uint16_t addr);

  void RequestInterrupt(uint8_t id);

  void AddMemoryRange(std::shared_ptr<MemoryRange> memoryRange);

private:
  std::vector<std::shared_ptr<MemoryRange>> _memoryRanges;
  std::shared_ptr<spdlog::logger> _logger{};
};
