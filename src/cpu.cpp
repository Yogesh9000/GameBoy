#include "cpu.hpp"
#include "bitutils.hpp"
#include "common.hpp"
#include <format>
#include <spdlog/spdlog.h>
#include <memory>

// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

Cpu::Cpu(MemoryManagementUnit& mmu) : _mmu(mmu), _interrupt{std::make_shared<Interrupt>()}
{
  _mmu.AddMemoryRange(_interrupt);
}

// TODO: Implement HALT BUG
void Cpu::Tick()
{
  auto opcode = _mmu.Read(PC.reg++);
#ifdef _DEBUG
  if (opcode != 0xCB)
  {
    SPDLOG_TRACE("PC: {:#06X}, Trying to execute instruction {:#04X}\n", PC.reg - 1, opcode);
  }
#endif

  HandleInterruptsIfAny();

  if (_interrupt->_enableRequested)
  {
    _interrupt->_enableRequested = false;
    _interrupt->_ime = true;
    SPDLOG_DEBUG("Interrupt enabled");
  }

  switch (opcode)
  {
  case 0x00:
    Nop();
    break;
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
  case 0x78:
    LdRR(AF.high, BC.high);
    break;
  case 0x7B:
    LdRR(AF.high, DE.low);
    break;
  case 0x7C:
    LdRR(AF.high, HL.high);
    break;
  case 0x7D:
    LdRR(AF.high, HL.low);
    break;
  case 0x86:
    AddAHl();
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
  case 0xC3:
    JpU16();
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
  case 0xF3:
    Di();
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

void Cpu::TickExtended()
{
  auto opcode = _mmu.Read(PC.reg++);

  SPDLOG_TRACE("[CB] PC: {:#06X}, Trying to execute instruction {:#02X}\n", PC.reg - 1, opcode);

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

void Cpu::HandleInterruptsIfAny()
{
  // Check if interrupts are enabled
  if (_interrupt->_ime)
  {
    // Check if any interrupts are enabled and requested
    if ((_interrupt->_ie & _interrupt->_if) != 0)
    {
      // Check if VBlank interrupt is enabled and requested
      if (BitUtils::Test<InterruptType::VBLANK>(_interrupt->_ie) && BitUtils::Test<InterruptType::VBLANK>(_interrupt->_if))
      {
        SPDLOG_DEBUG("VBlank interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::VBLANK);
      }
      // Check if LCD interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::LCD>(_interrupt->_ie) && BitUtils::Test<InterruptType::LCD>(_interrupt->_if))
      {
        SPDLOG_DEBUG("LCD interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::LCD);
      }
      // Check if Timer interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::TIMER>(_interrupt->_ie) && BitUtils::Test<InterruptType::TIMER>(_interrupt->_if))
      {
        SPDLOG_DEBUG("Timer interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::TIMER);
      }
      // Check if Serial interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::SERIAL>(_interrupt->_ie) && BitUtils::Test<InterruptType::SERIAL>(_interrupt->_if))
      {
        SPDLOG_DEBUG("Serial interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::SERIAL);
      }
      // Check if Joypad interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::JOYPAD>(_interrupt->_ie) && BitUtils::Test<InterruptType::JOYPAD>(_interrupt->_if))
      {
        SPDLOG_DEBUG("Joypad interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::JOYPAD);
      }
    }
  }
}

// TODO: Check if I can Refactor this method
void Cpu::DisableInterruptAndJumpToInterruptHandler(InterruptType interruptType)
{
  SPDLOG_DEBUG("Disabling interrupt before jumping to interrupt handler");
  Di();
  SPDLOG_DEBUG("Saving PC on stack before jumping to interrupt handler");
  SP.reg--;
  _mmu.Write(SP.reg, PC.low);
  SP.reg--;
  _mmu.Write(SP.reg, PC.high);

  switch (interruptType)
  {
  case InterruptType::VBLANK:
    SPDLOG_DEBUG("Unset bit VBLANK (0) in Interrupt Flag register (IF: 0xFF0F)");
    BitUtils::Unset<InterruptType::VBLANK>(_interrupt->_if);
    SPDLOG_DEBUG("Jumping to VBLANK interrupt handler");
    PC.reg = VBLANK_INTERRUPT_HANDLER_ADDRESS;
    break;
  case InterruptType::LCD:
    SPDLOG_DEBUG("Unset bit LCD (1) in Interrupt Flag register (IF: 0xFF0F)");
    BitUtils::Unset<InterruptType::LCD>(_interrupt->_if);
    SPDLOG_DEBUG("Jumping to LCD interrupt handler");
    PC.reg = STAT_INTERRUPT_HANDLER_ADDRESS;
    break;
  case InterruptType::TIMER:
    SPDLOG_DEBUG("Unset bit TIMER (2) in Interrupt Flag register (IF: 0xFF0F)");
    BitUtils::Unset<InterruptType::TIMER>(_interrupt->_if);
    SPDLOG_DEBUG("Jumping to TIMER interrupt handler");
    PC.reg = TIMER_INTERRUPT_HANDLER_ADDRESS;
    break;
  case InterruptType::SERIAL:
    SPDLOG_DEBUG("Unset bit SERIAL (3) in Interrupt Flag register (IF: 0xFF0F)");
    BitUtils::Unset<InterruptType::SERIAL>(_interrupt->_if);
    SPDLOG_DEBUG("Jumping to SERIAL interrupt handler");
    PC.reg = SERIAL_INTERRUPT_HANDLER_ADDRESS;
    break;
  case InterruptType::JOYPAD:
    SPDLOG_DEBUG("Unset bit JOYPAD (4) in Interrupt Flag register (IF: 0xFF0F)");
    BitUtils::Unset<InterruptType::JOYPAD>(_interrupt->_if);
    SPDLOG_DEBUG("Jumping to JOYPAD interrupt handler");
    PC.reg = JOYPAD_INTERRUPT_HANDLER_ADDRESS;
    break;
  }
}

// opcodes
void Cpu::Di()
{
  _interrupt->_enableRequested = false;
  _interrupt->_ime = false;
  SPDLOG_DEBUG("Interrupt disabled");
}

void Cpu::JpU16()
{
  auto lsb = _mmu.Read(PC.reg++);
  auto msb = _mmu.Read(PC.reg++);
  PC.reg = ToU16(lsb, msb);
}

void Cpu::Nop()
{
}

void Cpu::SubR(std::uint8_t reg)
{
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::AddAHl()
{
  auto data = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) + static_cast<uint16_t>(data);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((AF.high ^ data ^ res) & 0x10) != 0);
  SetCY(((AF.high ^ data ^ res) & 0x100) != 0);

  AF.high = (res & 0xFFU);
}

void Cpu::LdU16A()
{
  auto low = _mmu.Read(PC.reg++);
  auto high = _mmu.Read(PC.reg++);
  _mmu.Write(ToU16(low, high), AF.high);
}
void Cpu::CpN()
{
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::CpAHl()
{
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::LdHlPA()
{
  _mmu.Write(HL.reg, AF.high);
  ++HL.reg;
}

void Cpu::DecR(std::uint8_t& reg)
{
  std::uint8_t res = reg - 1;

  SetZ(res == 0);
  SetN(true);
  SetH(((reg & 0xFU) - 1) > 0xFU);

  reg = res;
}

void Cpu::PopRR(Register& reg)
{
  reg.low = _mmu.Read(SP.reg);
  ++SP.reg;
  reg.high = _mmu.Read(SP.reg);
  ++SP.reg;
}

void Cpu::RlA()
{
  uint8_t oldCY = (AF.low & (1U << 4U)) >> 4U;

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY(((AF.high & (1U << 7U)) >> 7U) == 1U);

  AF.high = static_cast<uint8_t>(AF.high << 1U);
  AF.high = static_cast<uint8_t>((AF.high & ~(1U << 0U)) | oldCY);
}

void Cpu::PushRR(Register& reg)
{
  --SP.reg;
  _mmu.Write(SP.reg, reg.high);
  --SP.reg;
  _mmu.Write(SP.reg, reg.low);
}

void Cpu::LdRR(std::uint8_t& reg1, std::uint8_t& reg2)
{
  reg1 = reg2;
}

void Cpu::CAllU16()
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

void Cpu::Ret()
{
  auto low = _mmu.Read(SP.reg);
  ++SP.reg;
  auto high = _mmu.Read(SP.reg);
  ++SP.reg;
  PC.reg = ToU16(low, high);
}

void Cpu::LdRrU16(Register& reg)
{
  reg.low = _mmu.Read(PC.reg++);
  reg.high = _mmu.Read(PC.reg++);
}

void Cpu::LdRU8(std::uint8_t& reg)
{
  reg = _mmu.Read(PC.reg++);
}

void Cpu::XorAA()
{
  AF.high = 0;
  SetZ(true);
}

void Cpu::LdADe()
{
  AF.high = _mmu.Read(DE.reg);
}

void Cpu::LdHlMA()
{
  _mmu.Write(HL.reg, AF.high);
  --HL.reg;
}

void Cpu::LdHlR(std::uint8_t& reg)
{
  _mmu.Write(HL.reg, reg);
}

void Cpu::JrCCI8(bool cc)
{
  auto i8 = static_cast<std::int8_t>(_mmu.Read(PC.reg++));
  if (cc)
  {
    PC.reg = static_cast<std::uint16_t>(PC.reg + i8);
  }
}

void Cpu::LdhAU8()
{
  auto low = _mmu.Read(PC.reg++);
  auto addr = ToU16(low, 0xFF);
  AF.high = _mmu.Read(addr);
}

void Cpu::LdhCA()
{
  auto addr = ToU16(BC.low, 0xFF);
  _mmu.Write(addr, AF.high);
}

void Cpu::LdhU8A()
{
  auto addr = ToU16(_mmu.Read(PC.reg++), 0xFF);
  _mmu.Write(addr, AF.high);
}

void Cpu::IncR(std::uint8_t& reg)
{
  uint16_t res = reg + 1;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((reg & 0x0FU) + 1) > 0x0FU);

  reg = (res & 0xFFU);
}

void Cpu::IncRR(Register& reg)
{
  ++reg.reg;
}

// extended opcodes
void Cpu::RlR(std::uint8_t& reg)
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

void Cpu::BitBR(unsigned int bit, std::uint8_t reg)
{
  auto bitValue = static_cast<std::uint8_t>((reg & (1U << bit)) >> bit);
  SetZ(bitValue == 0);
  SetN(false);
  SetH(true);
}

// utility

// Set zero flag
void Cpu::SetZ(bool value)
{
  AF.low = static_cast<std::uint8_t>(
    (AF.low & ~(1U << 7U)) | static_cast<uint8_t>(value << 7U));
}

// Get value of zero flag
[[nodiscard]]
bool Cpu::GetZ() const
{
  return ((AF.low & (1U << 7U)) >> 7U) == 1;
}

// Set negative flag
void Cpu::SetN(bool value)
{
  AF.low = static_cast<std::uint8_t>(
    (AF.low & ~(1U << 6U)) | static_cast<uint8_t>(value << 6U));
}

// Get value of negative flag
[[nodiscard]]
bool Cpu::GetN() const
{
  return ((AF.low & (1U << 6U)) >> 6U) == 1;
}

// Set half carry flag
void Cpu::SetH(bool value)
{
  AF.low = static_cast<std::uint8_t>(
    (AF.low & ~(1U << 5U)) | static_cast<uint8_t>(value << 5U));
}

// Get value of half carry flag
[[nodiscard]]
bool Cpu::GetH() const
{
  return ((AF.low & (1U << 5U)) >> 5U) == 1;
}

// Set carry flag
void Cpu::SetCY(bool value)
{
  AF.low = static_cast<std::uint8_t>(
    (AF.low & ~(1U << 4U)) | static_cast<uint8_t>(value << 4U));
}

// Get value of carry flag
[[nodiscard]]
bool Cpu::GetCY() const
{
  return ((AF.low & (1U << 4U)) >> 4U) == 1;
}

std::uint16_t Cpu::ToU16(std::uint8_t lsb, std::uint8_t msb)
{
  return static_cast<std::uint16_t>(msb << 8U) | lsb;
}
