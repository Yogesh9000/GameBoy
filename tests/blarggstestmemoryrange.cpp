#include "blarggstestmemoryrange.hpp"

[[nodiscard]]
std::uint8_t BlarggsTestMemoryRange::Read(std::uint16_t addr) const
{
  if (addr == 0xFF44)
  {
    return 0x90;
  }
  else
  {
    return ConcreteMemoryRange::Read(addr);
  }
}

void BlarggsTestMemoryRange::Write(std::uint16_t addr, std::uint8_t data)
{
  // Blarggs test rom write result to serial port so we check for that
  if (addr == 0xFF02 && data == 0x81)
  {
    _message += static_cast<char>(Read(0xFF01));
    ConcreteMemoryRange::Write(addr, 0);
  }
  else
  {
    ConcreteMemoryRange::Write(addr, data);
  }
}

[[nodiscard]]
bool BlarggsTestMemoryRange::IsTestPassed() const
{
  return (_message.find("Passed") != std::string::npos);
}

[[nodiscard]]
bool BlarggsTestMemoryRange::IsTestCompleted() const
{
  return ((_message.find("Passed") != std::string::npos)
          || (_message.find("Failed") != std::string::npos));
}

[[nodiscard]]
std::string BlarggsTestMemoryRange::GetMessage() const
{
  return _message;
}
