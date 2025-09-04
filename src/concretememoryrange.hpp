#pragma once
#include <vector>

#include "memoryrange.hpp"

class ConcreteMemoryRange : public MemoryRange
{
public:
  ConcreteMemoryRange(std::vector<std::uint8_t> memory, std::size_t offset);

  ConcreteMemoryRange(std::size_t size, std::size_t offset);

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

private:
  std::vector<std::uint8_t> _memory;
  std::size_t _offset;
};
