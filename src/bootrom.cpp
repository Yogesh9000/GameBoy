#include "bootrom.hpp"

#include <spdlog/spdlog.h>

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

#include "common.hpp"
#include "logmanager.hpp"

BootRom::BootRom() : _enabled{true}
{
  _logger = LogManager::GetLogger("BootRom");
}

bool BootRom::Contains(std::uint16_t addr) const
{
  return _enabled
         && (FileMemoryRange::Contains(addr) || addr == BOOTROM_ENABLE_ADDRESS);
}

void BootRom::Write(std::uint16_t addr, [[maybe_unused]] std::uint8_t data)
{
  // BootRom writes to address 0xFF50 at the end to disable itself
  if (addr == BOOTROM_ENABLE_ADDRESS)
  {
    LOG_INFO(_logger, "Detected Write to address: {:#04X}, Disabling BootRom",
        BOOTROM_ENABLE_ADDRESS);
    _enabled = false;
    return;
  }
  // writes to bootRom are ignored
  LOG_WARN(_logger,
      "Ignoring write to BootRom address: {:#06X}, data: {:#04X}\n", addr,
      data);
}

void BootRom::Load(const std::string &filePath)
{
  FileMemoryRange::Load(filePath, BootRomOffset);
}
