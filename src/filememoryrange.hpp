#pragma once

#include "memoryrange.hpp"
#include "concretememoryrange.hpp"
#include <vector>
#include <string>

class FileMemoryRange : public MemoryRange
{
public:
  explicit FileMemoryRange();

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

  std::uint8_t& Address(std::uint16_t addr) override;

  void Load(const std::string &filePath, int offset);

private:
  std::vector<std::uint8_t> _memory;
  std::size_t _offset;
};
