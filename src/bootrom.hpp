#pragma once
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

#include "common.hpp"
#include "concretememoryrange.hpp"
#include "memoryrange.hpp"

class BootRom : public MemoryRange
{
public:
  static BootRom Create(const std::string &romPath)
  {
    // open boot rom
    std::ifstream romFile{romPath, std::ios::binary | std::ios::ate};
    if (!romFile.is_open())
    {
      throw std::runtime_error(std::format("Failed to open rom: {}", romPath));
    }
    auto size = romFile.tellg();
    romFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> bootRom(static_cast<std::size_t>(size));

    // read boot rom to memory
    romFile.read(reinterpret_cast<char *>(bootRom.data()), size);
    if (bootRom.empty())
    {
      throw std::runtime_error("Failed to read rom");
    }

    // return BootRom
    return BootRom(bootRom);
  }

public:
  explicit BootRom(std::vector<std::uint8_t> bootRom)
      : _bootRom(std::move(bootRom), BootRomOffset), _enabled(true)
  {
  }

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override
  {
    return _enabled && _bootRom.Contains(addr);
  }

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override
  {
    return _bootRom.Read(addr);
  }

  void Write(std::uint16_t addr, std::uint8_t data) override
  {
    // writes to bootRom are ignored
    Info(std::format(
        "Ignoring write to BootRom address: {:#06X}, data: {:#04X}\n", addr,
        data));
  }

private:
  static constexpr int BootRomOffset{0x00};
  ConcreteMemoryRange _bootRom;
  bool _enabled{};
};
