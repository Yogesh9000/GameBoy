#include <SDL3/SDL.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <numbers>
#include <stdexcept>
#include <vector>

// #define DEBUG_CPU

// logging
namespace
{

void Info(const std::string &msg)
{
  std::clog << std::format("\033[32m[info]\033[0m {}", msg);
}

void Warning(const std::string &msg)
{
  std::clog << std::format("\033[33m[warn]\033[0m {}", msg);
}

void Error(const std::string &msg)
{
  std::clog << std::format("\033[31m[error]\033[0m {}", msg);
}

}  // namespace

// Interface for a memory area in gameboy memory map
struct MemoryRange
{
  MemoryRange(const MemoryRange &) = default;
  MemoryRange &operator=(const MemoryRange &) = default;
  MemoryRange(MemoryRange &&) = default;
  MemoryRange &operator=(MemoryRange &&) = default;
  virtual ~MemoryRange() = default;
  MemoryRange() = default;

  [[nodiscard]]
  virtual bool Contains(std::uint16_t) const = 0;
  [[nodiscard]]
  virtual std::uint8_t Read(std::uint16_t addr) const = 0;
  virtual void Write(std::uint16_t addr, std::uint8_t data) = 0;
};

class ConcreteMemoryRange : public MemoryRange
{
public:
  ConcreteMemoryRange(std::vector<std::uint8_t> memory, std::size_t offset)
      : _memory(std::move(memory)), _offset(offset)
  {
  }

  ConcreteMemoryRange(std::size_t size, std::size_t offset)
      : _memory(size), _offset(offset)
  {
  }

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override
  {
    return addr >= _offset && addr <= (_offset + _memory.size());
  }

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override
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

  void Write(std::uint16_t addr, std::uint8_t data) override
  {
    // write to memory only if address is contained in memory range
    if (Contains(addr))
    {
      // adjust the addr by substracting the offset
      _memory[addr - _offset] = data;
    }
  }

private:
  std::vector<std::uint8_t> _memory;
  std::size_t _offset;
};

class BootRom : public MemoryRange
{
public:
  static BootRom Create(const std::string &romPath)
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

public:
  explicit BootRom(std::vector<std::uint8_t> bootRom)
      : _bootRom(std::move(bootRom), BootRomOffset), _enabled(true)
  {
  }

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override
  {
    return _enabled && _bootRom.Contains(addr);
  }

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override
  {
    return _bootRom.Read(addr);
  }

  void Write(std::uint16_t addr, std::uint8_t data) override
  {
    // writes to bootRom are ignored
    Info(std::format(
        "Ignoring write to BootRom address: {:#06X}, data: {:#04X}\n", addr,
        data));
  }

private:
  static constexpr int BootRomOffset{0x00};
  ConcreteMemoryRange _bootRom;
  bool _enabled{};
};

class MemoryManagementUnit
{
private:
  // utility
  [[nodiscard]]
  auto GetMemoryRange(std::uint16_t addr) const
  {
    return std::find_if(_memoryRanges.cbegin(), _memoryRanges.cend(),
        [=](const auto &range) { return range->Contains(addr); });
  }

  auto GetMemoryRange(std::uint16_t addr)
  {
    return std::find_if(_memoryRanges.begin(), _memoryRanges.end(),
        [=](const auto &range) { return range->Contains(addr); });
  }

public:
  // TODO: try eliminate checking if a memory range is present in every method

  [[nodiscard]]
  bool Contains(std::uint16_t addr) const  // TODO: Do I need this
  {
    auto memoryRange = GetMemoryRange(addr);
    return memoryRange != _memoryRanges.cend();
  }

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const
  {
    auto memoryRange = GetMemoryRange(addr);
    // If we found a memory range read from it
    if (memoryRange != _memoryRanges.cend())
    {
      return memoryRange->get()->Read(addr);
    }

    // return garbage value if no memory range found that contains addr
    Warning(std::format(
        "Read: No registered memory region found that contains address: "
        "{:#06X}\n",
        addr));
    return 0xFF;
  }

  void Write(std::uint16_t addr, std::uint8_t data)
  {
    auto memoryRange = GetMemoryRange(addr);
    // if memory region found write to it
    if (memoryRange != _memoryRanges.end())
    {
      memoryRange->get()->Write(addr, data);
      return;
    }
    Warning(std::format(
        "Write: No registered memory region found that contains address: {:#06X}\n",
        addr));
  }

  void AddMemoryRange(std::shared_ptr<MemoryRange> memoryRange)
  {
    _memoryRanges.emplace_back(std::move(memoryRange));
  }

private:
  std::vector<std::shared_ptr<MemoryRange>> _memoryRanges;
};

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

class Cpu
{
public:
  explicit Cpu(MemoryManagementUnit &mmu) : _mmu(mmu)
  {
  }

  void Tick()
  {
    auto opcode = _mmu.Read(PC.reg++);
#ifdef DEBUG_CPU
    if (opcode != 0xCB)
    {
      std::cout << std::format(
          "PC: {:#06X}, Trying to execute instruction {:#04X}\n", PC.reg - 1,
          opcode);
    }
#endif
    switch (opcode)
    {
      case 0x05:
        DecR(BC.high);
        break;
      case 0x04:
        IncR(BC.high);
        break;
      case 0x06:
        LdRU8(BC.high);
        break;
      case 0x0C:
        IncR(BC.low);
        break;
      case 0x0D:
        DecR(BC.low);
        break;
      case 0x0E:
        LdRU8(BC.low);
        break;
      case 0x11:
        LdRrU16(DE);
        break;
      case 0x13:
        IncRR(DE);
        break;
      case 0x15:
        DecR(DE.high);
        break;
      case 0x16:
        LdRU8(DE.high);
        break;
      case 0x17:
        RlA();
        break;
      case 0x18:
        JrCCI8(true);
        break;
      case 0x1A:
        LdADe();
        break;
      case 0x1D:
        DecR(DE.low);
        break;
      case 0x1E:
        LdRU8(DE.low);
        break;
      case 0x20:
        JrCCI8(!GetZ());
        break;
      case 0x21:
        LdRrU16(HL);
        break;
      case 0x22:
        LdHlPA();
        break;
      case 0x23:
        IncRR(HL);
        break;
      case 0x24:
        IncR(HL.high);
        break;
      case 0x28:
        JrCCI8(GetZ());
        break;
      case 0x2E:
        LdRU8(HL.low);
        break;
      case 0x31:
        LdRrU16(SP);
        break;
      case 0x32:
        LdHlMA();
        break;
      case 0x3D:
        DecR(AF.high);
        break;
      case 0x3E:
        LdRU8(AF.high);
        break;
      case 0x57:
        LdRR(DE.high, AF.high);
        break;
      case 0x67:
        LdRR(HL.high, AF.high);
        break;
      case 0x4F:
        LdRR(BC.low, AF.high);
        break;
      case 0x77:
        LdHlR(AF.high);
        break;
      case 0x7B:
        LdRR(AF.high, DE.low);
        break;
      case 0x7C:
        LdRR(AF.high, HL.high);
        break;
      case 0x90:
        SubR(BC.high);
        break;
      case 0xAF:
        XorAA();
        break;
      case 0xBE:
        CpAHl();
        break;
      case 0xC1:
        PopRR(BC);
        break;
      case 0xCB:
        TickExtended();
        break;
      case 0xCD:
        CAllU16();
        break;
      case 0xC5:
        PushRR(BC);
        break;
      case 0xC9:
        Ret();
        break;
      case 0xE0:
        LdhU8A();
        break;
      case 0xE2:
        LdhCA();
        break;
      case 0xEA:
        LdU16A();
        break;
      case 0xF0:
        LdhAU8();
        break;
      case 0xFE:
        CpN();
        break;
      default:
        throw std::runtime_error(std::format(
            "PC: {:#06X}, failed to execute instruction \033[31m{:#04X}\033[0m",
            PC.reg - 1, opcode));
    }
  }

private:
  void TickExtended()
  {
    auto opcode = _mmu.Read(PC.reg++);
#ifdef DEBUG_CPU
    std::cout << std::format(
        "[CB] PC: {:#06X}, Trying to execute instruction {:#02X}\n", PC.reg - 1,
        opcode);
#endif
    switch (opcode)
    {
      case 0x11:
        RlR(BC.low);
        break;
      case 0x7C:
        BitBR(7U, HL.high);
        break;
      default:
        throw std::runtime_error(std::format(
            "[CB] PC: {:#06X}, failed to execute instruction \033[31m{:#04X}\033[0m",
            PC.reg - 1, opcode));
    }
  }

  // opcodes
  void SubR(std::uint8_t reg)
  {
    uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(reg);

    SetZ((res & 0xFFU) == 0);
    SetN(true);
    SetH(((AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
    SetCY(res > 0xFFU);

    AF.high = (res & 0xFFU);
  }
  void LdU16A()
  {
    auto low = _mmu.Read(PC.reg++);
    auto high = _mmu.Read(PC.reg++);
    _mmu.Write(ToU16(low, high), AF.high);
  }
  void CpN()
  {
    uint8_t u8 = _mmu.Read(PC.reg++);
    uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

    SetZ((res & 0xFFU) == 0);
    SetN(true);
    SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
    SetCY(res > 0xFFU);
  }

  void CpAHl()
  {
    uint8_t u8 = _mmu.Read(HL.reg);
    uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

    SetZ((res & 0xFFU) == 0);
    SetN(true);
    SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
    SetCY(res > 0xFFU);
  }

  void LdHlPA()
  {
    _mmu.Write(HL.reg, AF.high);
    ++HL.reg;
  }

  void DecR(std::uint8_t &reg)
  {
    std::uint8_t res = reg - 1;

    SetZ(res == 0);
    SetN(true);
    SetH(((reg & 0xFU) - 1) > 0xFU);

    reg = res;
  }

  void PopRR(Register &reg)
  {
    reg.low = _mmu.Read(SP.reg);
    ++SP.reg;
    reg.high = _mmu.Read(SP.reg);
    ++SP.reg;
  }

  void RlA()
  {
    uint8_t oldCY = (AF.low & (1U << 4U)) >> 4U;

    SetZ(false);
    SetN(false);
    SetH(false);
    SetCY(((AF.high & (1U << 7U)) >> 7U) == 1U);

    AF.high = static_cast<uint8_t>(AF.high << 1U);
    AF.high = static_cast<uint8_t>((AF.high & ~(1U << 0U)) | oldCY);
  }

  void PushRR(Register &reg)
  {
    --SP.reg;
    _mmu.Write(SP.reg, reg.high);
    --SP.reg;
    _mmu.Write(SP.reg, reg.low);
  }

  void LdRR(std::uint8_t &reg1, std::uint8_t &reg2)
  {
    reg1 = reg2;
  }

  void CAllU16()
  {
    auto low = _mmu.Read(PC.reg++);
    auto high = _mmu.Read(PC.reg++);

    --SP.reg;
    _mmu.Write(SP.reg, PC.high);
    --SP.reg;
    _mmu.Write(SP.reg, PC.low);

    PC.low = low;
    PC.high = high;
  }

  void Ret()
  {
    auto low = _mmu.Read(SP.reg);
    ++SP.reg;
    auto high = _mmu.Read(SP.reg);
    ++SP.reg;
    PC.reg = ToU16(low, high);
  }

  void LdRrU16(Register &reg)
  {
    reg.low = _mmu.Read(PC.reg++);
    reg.high = _mmu.Read(PC.reg++);
  }

  void LdRU8(std::uint8_t &reg)
  {
    reg = _mmu.Read(PC.reg++);
  }

  void XorAA()
  {
    AF.high = 0;
    SetZ(true);
  }

  void LdADe()
  {
    AF.high = _mmu.Read(DE.reg);
  }

  void LdHlMA()
  {
    _mmu.Write(HL.reg, AF.high);
    --HL.reg;
  }

  void LdHlR(std::uint8_t &reg)
  {
    _mmu.Write(HL.reg, reg);
  }

  void JrCCI8(bool cc)
  {
    auto i8 = static_cast<std::int8_t>(_mmu.Read(PC.reg++));
    if (cc)
    {
      PC.reg = static_cast<std::uint16_t>(PC.reg + i8);
    }
  }

  void LdhAU8()
  {
    auto low = _mmu.Read(PC.reg++);
    auto addr = ToU16(low, 0xFF);
    AF.high = _mmu.Read(addr);
  }

  void LdhCA()
  {
    auto addr = ToU16(BC.low, 0xFF);
    _mmu.Write(addr, AF.high);
  }

  void LdhU8A()
  {
    auto addr = ToU16(_mmu.Read(PC.reg++), 0xFF);
    _mmu.Write(addr, AF.high);
  }

  void IncR(std::uint8_t &reg)
  {
    uint16_t res = reg + 1;

    SetZ((res & 0xFFU) == 0);
    SetN(false);
    SetH(((reg & 0x0FU) + 1) > 0x0FU);

    reg = (res & 0xFFU);
  }

  void IncRR(Register &reg)
  {
    ++reg.reg;
  }

  // extended opcodes
  void RlR(std::uint8_t &reg)
  {
    uint8_t oldCY = (AF.low & (1U << 4U)) >> 4U;
    uint8_t bit7 = (reg & (1U << 7U)) >> 7U;
    reg = static_cast<uint8_t>(reg << 1U);
    reg = static_cast<std::uint8_t>((reg & ~(1U << 0U)) | oldCY);

    SetZ(reg == 0);
    SetN(false);
    SetH(false);
    SetCY(bit7 == 1U);
  }

  void BitBR(uint bit, std::uint8_t reg)
  {
    auto bitValue = static_cast<std::uint8_t>((reg & (1U << bit)) >> bit);
    SetZ(bitValue == 0);
    SetN(false);
    SetH(true);
  }

  // utility

  // Set zero flag
  void SetZ(bool value)
  {
    AF.low = static_cast<std::uint8_t>(
        (AF.low & ~(1U << 7U)) | static_cast<uint8_t>(value << 7U));
  }

  // Get value of zero flag
  [[nodiscard]]
  bool GetZ() const
  {
    return ((AF.low & (1U << 7U)) >> 7U) == 1;
  }

  // Set negative flag
  void SetN(bool value)
  {
    AF.low = static_cast<std::uint8_t>(
        (AF.low & ~(1U << 6U)) | static_cast<uint8_t>(value << 6U));
  }

  // Get value of negative flag
  [[nodiscard]]
  bool GetN() const
  {
    return ((AF.low & (1U << 6U)) >> 6U) == 1;
  }

  // Set half carry flag
  void SetH(bool value)
  {
    AF.low = static_cast<std::uint8_t>(
        (AF.low & ~(1U << 5U)) | static_cast<uint8_t>(value << 5U));
  }

  // Get value of half carry flag
  [[nodiscard]]
  bool GetH() const
  {
    return ((AF.low & (1U << 5U)) >> 5U) == 1;
  }

  // Set carry flag
  void SetCY(bool value)
  {
    AF.low = static_cast<std::uint8_t>(
        (AF.low & ~(1U << 4U)) | static_cast<uint8_t>(value << 4U));
  }

  // Get value of carry flag
  [[nodiscard]]
  bool GetCY() const
  {
    return ((AF.low & (1U << 4U)) >> 4U) == 1;
  }

  static std::uint16_t ToU16(std::uint8_t lsb, std::uint8_t msb)
  {
    return static_cast<std::uint16_t>(msb << 8U) | lsb;
  }

  // state
  Register AF{};
  Register BC{};
  Register DE{};
  Register HL{};
  Register SP{};
  Register PC{};

private:
  MemoryManagementUnit _mmu;
};

struct Color
{
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
  std::uint8_t alpha;
};

struct Pixel
{
  unsigned int x;
  unsigned int y;
  Color color;
};

struct Display
{
  Display(const Display &) = default;
  Display &operator=(const Display &) = default;
  Display(Display &&) = default;
  Display &operator=(Display &&) = default;
  virtual ~Display() = default;
  Display() = default;

  virtual void Draw(const Pixel &) = 0;
  virtual void UpdateFrame() = 0;
};

class SdlDisplay : public Display
{
public:
  constexpr static int LCD_WIDTH{160};
  constexpr static int LCD_HEIGHT{144};
  constexpr static int SCALE{4};
  constexpr static int PIXEL_SIZE{3};

public:
  SdlDisplay(const char *title)
      : _pixels(
            static_cast<unsigned long>(PIXEL_SIZE * LCD_WIDTH * LCD_HEIGHT), 0)
  {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(title, LCD_WIDTH * SCALE, LCD_HEIGHT * SCALE,
        SDL_WINDOW_RESIZABLE, &_window, &_renderer);
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);
  }

  ~SdlDisplay() override
  {
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
  }

  void Draw(const Pixel &pixel) override
  {
    auto idx = (LCD_WIDTH * PIXEL_SIZE * pixel.y) + (PIXEL_SIZE * pixel.x);
    if (idx + 2 <= _pixels.size())
    {
      _pixels[idx] = pixel.color.red;
      _pixels[idx + 1] = pixel.color.green;
      _pixels[idx + 2] = pixel.color.blue;
    }
  }

  void UpdateFrame() override
  {
    SDL_UpdateTexture(
        _texture, nullptr, _pixels.data(), LCD_WIDTH * PIXEL_SIZE);
    SDL_RenderClear(_renderer);
    SDL_RenderTexture(_renderer, _texture, nullptr, nullptr);
    SDL_RenderPresent(_renderer);
  }

private:
  SDL_Window *_window;
  SDL_Renderer *_renderer;
  SDL_Texture *_texture;
  std::vector<std::uint8_t> _pixels;
};

class Ppu : public MemoryRange
{
private:
  constexpr static int LY_REGISTER_ADDRESS{0xFF44};

public:
  Ppu(MemoryManagementUnit &mmu, Display &display)
      : _ly(0), _dotsThisLine(0), _mmu(mmu), _display(display)
  {
  }

  void Tick()
  {
    _dotsThisLine += 4;
    if (_dotsThisLine >= 204)
    {
      _dotsThisLine = 0;
      if (_ly <= 143)
      {
        DrawCurrentLine();
      }
      if (_ly == 144)
      {
        _display.UpdateFrame();
      }
      ++_ly;
      if (_ly >= 153)
      {
        _ly = 0;
      }
    }
  }

public:
  [[nodiscard]]
  bool Contains(std::uint16_t addr) const override
  {
    return addr == LY_REGISTER_ADDRESS;
  }

  [[nodiscard]]
  std::uint8_t Read(std::uint16_t addr) const override
  {
    if (addr == LY_REGISTER_ADDRESS)
    {
      return _ly;
    }
    // if address is not presesnt, return dummy value
    return 0xFF;
  }

  void Write(std::uint16_t addr, std::uint8_t data) override
  {
    // Nothing to do here
  }

private:
  void DrawCurrentLine()
  {
    constexpr int BACKGROUND_TILE_MAP_ADDRESS{0x9800};
    constexpr int TILE_DATA_ADDRESS{0x8000};
    constexpr int DISPLAY_WIDTH{160};

    int tileRow = _ly / 8;
    int pixelRowInTile = _ly % 8;

    for (int tileX = 0; tileX < DISPLAY_WIDTH / 8; ++tileX)
    {
      // Which tile is used at this position
      int mapIndex = tileRow * 32 + tileX;
      int tileIndex = _mmu.Read(BACKGROUND_TILE_MAP_ADDRESS + mapIndex);

      // Each tile = 16 bytes
      int tileAddr = TILE_DATA_ADDRESS + (tileIndex * 16);

      // Row inside tile = 2 bytes
      int lineAddr = tileAddr + (pixelRowInTile * 2);
      uint8_t byte1 = _mmu.Read(lineAddr);
      uint8_t byte2 = _mmu.Read(lineAddr + 1);

      for (int j = 0; j < 8; ++j)
      {
        int bitIndex = 7 - j;
        int bit1 = (byte1 >> bitIndex) & 1;
        int bit2 = (byte2 >> bitIndex) & 1;
        int colorId = (bit2 << 1) | bit1;

        Color color{};
        switch (colorId)
        {
          case 0b00:
            color = {0x34, 0x3d, 0x37, 0xff};
            break;
          case 0b01:
            color = {0x55, 0x64, 0x5a, 0xff};
            break;
          case 0b10:
            color = {0x8b, 0xa3, 0x94, 0xff};
            break;
          case 0b11:
            color = {0xe0, 0xf0, 0xe7, 0xff};
            break;
        }

        unsigned int screenX = tileX * 8 + j;
        auto pixel = Pixel{.x = screenX, .y = _ly, .color = color};
        _display.Draw(pixel);
      }
    }
  }

private:
  std::uint8_t _ly;
  int _dotsThisLine;
  MemoryManagementUnit &_mmu;
  Display &_display;
};

int main(int argc, char **argv)
{
  // early exit if no rom present
  if (argc != 2)
  {
    Error("No rom path provided\n");
    return 1;
  }
  char *romPath = argv[1];  // rom filepath

  MemoryManagementUnit mmu;

  // load the bootrom and add it to romPath
  mmu.AddMemoryRange(
      std::make_shared<BootRom>(std::move(BootRom::Create(romPath))));
  // add vram: 0x8000 - 0x9FFF
  constexpr int VRAM_SIZE{1024 * 8};
  constexpr int VRAM_START_ADDRESS{0x8000};
  mmu.AddMemoryRange(
      std::make_shared<ConcreteMemoryRange>(VRAM_SIZE, VRAM_START_ADDRESS));
  // add hram: 0xFF80 - 0xFFFE
  constexpr int HRAM_SIZE{0x7F};
  constexpr int HRAM_START_ADDRESS{0xFF80};
  mmu.AddMemoryRange(
      std::make_shared<ConcreteMemoryRange>(HRAM_SIZE, HRAM_START_ADDRESS));
  // add ppu: 0xFF44
  SdlDisplay display{"NoobBoy"};
  std::shared_ptr<Ppu> ppu{std::make_shared<Ppu>(mmu, display)};
  mmu.AddMemoryRange(ppu);

  Cpu cpu(mmu);

  // game loop
  try
  {
    SDL_Event event;
    bool quit{false};
    while (!quit)
    {
      while (SDL_PollEvent(&event))
      {
        if (event.type == SDL_EVENT_QUIT)
        {
          quit = true;
        }
      }
      cpu.Tick();
      ppu->Tick();
    }
  } catch (std::exception &ex)
  {
    Error(std::format("{}\n", ex.what()));
  }
  return 0;
}
