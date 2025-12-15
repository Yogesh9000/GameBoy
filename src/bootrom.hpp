#pragma once

#include <string>

#include "concretememoryrange.hpp"
#include "memoryrange.hpp"
#include "filememoryrange.hpp"

class BootRom : public FileMemoryRange
{
public:
  explicit BootRom();

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

  void Load(const std::string &filePath);

private:
  static constexpr int BootRomOffset{0x00};
  bool _enabled{};
};
