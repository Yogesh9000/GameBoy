#pragma once

#include <spdlog/spdlog.h>

#include <string>

#include "concretememoryrange.hpp"

class BlarggsTestMemoryRange : public ConcreteMemoryRange
{
public:
  using ConcreteMemoryRange::ConcreteMemoryRange;

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override;

  void Write(std::uint16_t addr, std::uint8_t data) override;

  [[nodiscard]]
  bool IsTestPassed() const;

  [[nodiscard]]
  bool IsTestCompleted() const;

  [[nodiscard]]
  std::string GetMessage() const;

private:
  std::string _message{};
};
