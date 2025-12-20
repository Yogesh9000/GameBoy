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
  return _enabled && (FileMemoryRange::Contains(addr) || addr == BOOTROM_ENABLE_ADDRESS);
}

void BootRom::Write(std::uint16_t addr, std::uint8_t data)
{
  // BootRom writes to address 0xFF50 at the end to disable itself
  if (addr == BOOTROM_ENABLE_ADDRESS)
  {
    SPDLOG_INFO("Detected Write to address: {:#04X}, Disabling BootRom", BOOTROM_ENABLE_ADDRESS);
    _enabled = false;
    return;
  }
  // writes to bootRom are ignored
  SPDLOG_WARN("Ignoring write to BootRom address: {:#06X}, data: {:#04X}\n", addr, data);
}

void BootRom::Load(const std::string& filePath)
{
  FileMemoryRange::Load(filePath, BootRomOffset);
}
