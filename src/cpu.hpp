#pragma once

// #define DEBUG_CPU

#include "mmu.hpp"
#include "register.hpp"
#include <format>
#include <iostream>

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

  void BitBR(unsigned int bit, std::uint8_t reg)
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
