#include "bootrom.hpp"

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <spdlog/spdlog.h>

#include "common.hpp"

BootRom BootRom::Create(const std::string &romPath)
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

BootRom::BootRom(std::vector<std::uint8_t> bootRom)
    : _bootRom(std::move(bootRom), BootRomOffset), _enabled(true)
{
}

bool BootRom::Contains(std::uint16_t addr) const
{
  return _enabled && _bootRom.Contains(addr);
}

std::uint8_t BootRom::Read(std::uint16_t addr) const
{
  return _bootRom.Read(addr);
}

void BootRom::Write(std::uint16_t addr, std::uint8_t data)
{
  // writes to bootRom are ignored
  SPDLOG_WARN("Ignoring write to BootRom address: {:#06X}, data: {:#04X}\n", addr, data);
}
