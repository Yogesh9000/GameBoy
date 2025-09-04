#pragma once

#include <string>

#include "concretememoryrange.hpp"
#include "memoryrange.hpp"

class BootRom : public MemoryRange
{
public:
  static BootRom Create(const std::string &romPath);

public:
  explicit BootRom(std::vector<std::uint8_t> bootRom);

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

private:
  static constexpr int BootRomOffset{0x00};
  ConcreteMemoryRange _bootRom;
  bool _enabled{};
};
