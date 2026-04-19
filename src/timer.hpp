#pragma once

#include <spdlog/logger.h>

#include <cstdint>

#include "memoryrange.hpp"
#include "mmu.hpp"

class Timer : public MemoryRange
{
public:
  explicit Timer(MemoryManagementUnit &mmu);
  void Tick(int cycles);

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override;
  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;
  void Write(std::uint16_t addr, std::uint8_t data) override;
  std::uint8_t &Address(std::uint16_t addr) override;

private:
  void UpdateDividerRegister(int cycles);
  void UpdateTimers(int cycles);
  [[nodiscard]]
  int GetClockFreq() const;
  bool IsClockEnabled();

  MemoryManagementUnit &_mmu;

  int _timerCounter{0};
  int _dividerCounter{0};

  std::uint8_t _div{};
  std::uint8_t _tima{};
  std::uint8_t _tma{};
  std::uint8_t _tac{};

  std::shared_ptr<spdlog::logger> _logger;
};
