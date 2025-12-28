#pragma once

#include <cstdint>

// Interface for a memory area in gameboy memory map
struct MemoryRange
{
  MemoryRange(const MemoryRange &) = default;
  MemoryRange &operator=(const MemoryRange &) = default;
  MemoryRange(MemoryRange &&) = default;
  MemoryRange &operator=(MemoryRange &&) = default;
  virtual ~MemoryRange() = default;
  MemoryRange() = default;

  [[nodiscard]]
  virtual bool Contains(std::uint16_t) const = 0;
  [[nodiscard]]
  virtual std::uint8_t Read(std::uint16_t addr) const = 0;
  virtual void Write(std::uint16_t addr, std::uint8_t data) = 0;
  virtual std::uint8_t& Address(std::uint16_t addr) = 0;
};
