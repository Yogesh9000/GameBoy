#include "concretememoryrange.hpp"

#include <utility>

ConcreteMemoryRange::ConcreteMemoryRange(
    std::vector<std::uint8_t> memory, std::size_t offset)
    : _memory(std::move(memory)), _offset(offset)
{
}

ConcreteMemoryRange::ConcreteMemoryRange(std::size_t size, std::size_t offset)
    : _memory(size), _offset(offset)
{
}

bool ConcreteMemoryRange::Contains(std::uint16_t addr) const
{
  return addr >= _offset && addr < (_offset + _memory.size());
}

std::uint8_t ConcreteMemoryRange::Read(std::uint16_t addr) const
{
  // return data from memroy if memory range contains the address
  if (Contains(addr))
  {
    // adjust address by substracting the offset
    return _memory[addr - _offset];
  }

  // return dummy value if memory range does not contain addr
  return 0xFF;
}

void ConcreteMemoryRange::Write(std::uint16_t addr, std::uint8_t data)
{
  // write to memory only if address is contained in memory range
  if (Contains(addr))
  {
    // adjust the addr by substracting the offset
    _memory[addr - _offset] = data;
  }
}
