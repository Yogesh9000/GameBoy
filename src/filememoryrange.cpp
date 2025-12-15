#include "filememoryrange.hpp"
#include <fstream>
#include <format>

void FileMemoryRange::Load(const std::string& filePath, int offset)
{
  _offset = offset;

  // open file
  std::ifstream file{filePath, std::ios::binary | std::ios::ate};
  if (!file.is_open())
  {
    throw std::runtime_error(std::format("Failed to open rom: {}", filePath));
  }
  auto size = file.tellg();
  file.seekg(0, std::ios::beg);
  _memory.resize(static_cast<std::size_t>(size));

  // read file to memory
  file.read(reinterpret_cast<char *>(_memory.data()), size);
  if (_memory.empty())
  {
    throw std::runtime_error("Failed to read rom");
  }
}

FileMemoryRange::FileMemoryRange()
  : _memory{}, _offset{}
{
}

bool FileMemoryRange::Contains(std::uint16_t addr) const
{
  return addr >= _offset && addr < (_offset + _memory.size());
}

std::uint8_t FileMemoryRange::Read(std::uint16_t addr) const
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

void FileMemoryRange::Write(std::uint16_t addr, std::uint8_t data)
{
  // write to memory only if address is contained in memory range
  if (Contains(addr))
  {
    // adjust the addr by substracting the offset
    _memory[addr - _offset] = data;
  }
}
