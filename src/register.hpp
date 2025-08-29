#pragma once

#include <cstdint>
union Register
{
  // Access register as a 16bit value
  std::uint16_t reg;
  struct
  {
    // Access individual register
    // For a register like AF: high = A and low = F
    std::uint8_t low;
    std::uint8_t high;
  };
};
