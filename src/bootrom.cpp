#include "bootrom.hpp"

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <spdlog/spdlog.h>

#include "common.hpp"

BootRom::BootRom()
  : _enabled{true}
{
}

bool BootRom::Contains(std::uint16_t addr) const
{
  return _enabled && FileMemoryRange::Contains(addr);
}

void BootRom::Write(std::uint16_t addr, std::uint8_t data)
{
  // writes to bootRom are ignored
  SPDLOG_WARN("Ignoring write to BootRom address: {:#06X}, data: {:#04X}\n", addr, data);
}

void BootRom::Load(const std::string& filePath)
{
  FileMemoryRange::Load(filePath, BootRomOffset);
}
