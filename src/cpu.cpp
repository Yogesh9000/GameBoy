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
  case 0x01:
    LdRrU16(BC);
    break;
  case 0x02:
    LdIRrA(BC);
    break;
  case 0x03:
    IncRr(BC);
    break;
  case 0x04:
    IncR(BC.high);
    break;
  case 0x05:
    DecR(BC.high);
    break;
  case 0x06:
    LdRU8(BC.high);
    break;
  case 0x07:
    Rlca();
    break;
  case 0x08:
    LdDU16Sp();
    break;
  case 0x09:
    AddHlRr(BC);
    break;
  case 0x0A:
    LdAIRr(BC);
    break;
  case 0x0B:
    DecRr(BC);
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
  case 0x0F:
    Rrca();
    break;
  case 0x10:
    Stop();
    break;
  case 0x11:
    LdRrU16(DE);
    break;
  case 0x12:
    LdIRrA(DE);
    break;
  case 0x13:
    IncRr(DE);
    break;
  case 0x14:
    IncR(DE.high);
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
  case 0x19:
    AddHlRr(DE);
    break;
  case 0x1A:
    LdAIRr(DE);
    break;
  case 0x1B:
    DecRr(DE);
    break;
  case 0x1C:
    IncR(DE.low);
    break;
  case 0x1D:
    DecR(DE.low);
    break;
  case 0x1E:
    LdRU8(DE.low);
    break;
  case 0x1F:
    Rra();
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
    IncRr(HL);
    break;
  case 0x24:
    IncR(HL.high);
    break;
  case 0x25:
    DecR(HL.high);
    break;
  case 0x26:
    LdRU8(HL.high);
    break;
  case 0x27:
    Daa();
    break;
  case 0x28:
    JrCCI8(GetZ());
    break;
  case 0x29:
    AddHlRr(HL);
    break;
  case 0x2A:
    LdAHlP();
    break;
  case 0x2B:
    DecRr(HL);
    break;
  case 0x2C:
    IncR(HL.low);
    break;
  case 0x2D:
    DecR(HL.low);
    break;
  case 0x2E:
    LdRU8(HL.low);
    break;
  case 0x2F:
    Cpl();
    break;
  case 0x30:
    JrCCI8(!GetCY());
    break;
  case 0x31:
    LdRrU16(SP);
    break;
  case 0x32:
    LdHlMA();
    break;
  case 0x33:
    IncRr(SP);
    break;
  case 0x34:
    IncIHl();
    break;
  case 0x35:
    DecHl();
    break;
  case 0x36:
    LdHlU8();
    break;
  case 0x37:
    Scf();
    break;
  case 0x38:
    JrCCI8(GetCY());
    break;
  case 0x39:
    AddHlRr(SP);
    break;
  case 0x3A:
    LdAHlN();
    break;
  case 0x3B:
    DecRr(SP);
    break;
  case 0x3C:
    IncR(AF.high);
    break;
  case 0x3D:
    DecR(AF.high);
    break;
  case 0x3E:
    LdRU8(AF.high);
    break;
  case 0x3F:
    Ccf();
    break;
  case 0x40:
    LdRR(BC.high, BC.high);
    break;
  case 0x41:
    LdRR(BC.high, BC.low);
    break;
  case 0x42:
    LdRR(BC.high, DE.high);
    break;
  case 0x43:
    LdRR(BC.high, DE.low);
    break;
  case 0x44:
    LdRR(BC.high, HL.high);
    break;
  case 0x45:
    LdRR(BC.high, HL.low);
    break;
  case 0x46:
    LdRIHl(BC.high);
    break;
  case 0x47:
    LdRR(BC.high, AF.high);
    break;
  case 0x48:
    LdRR(BC.low, BC.high);
    break;
  case 0x49:
    LdRR(BC.low, BC.low);
    break;
  case 0x4A:
    LdRR(BC.low, DE.high);
    break;
  case 0x4B:
    LdRR(BC.low, DE.low);
    break;
  case 0x4C:
    LdRR(BC.low, HL.high);
    break;
  case 0x4D:
    LdRR(BC.low, HL.low);
    break;
  case 0x4E:
    LdRIHl(BC.low);
    break;
  case 0x4F:
    LdRR(BC.low, AF.high);
    break;
  case 0x50:
    LdRR(DE.high, BC.high);
    break;
  case 0x51:
    LdRR(DE.high, BC.low);
    break;
  case 0x52:
    LdRR(DE.high, DE.high);
    break;
  case 0x53:
    LdRR(DE.high, DE.low);
    break;
  case 0x54:
    LdRR(DE.high, HL.high);
    break;
  case 0x55:
    LdRR(DE.high, HL.low);
    break;
  case 0x56:
    LdRIHl(DE.high);
    break;
  case 0x57:
    LdRR(DE.high, AF.high);
    break;
  case 0x58:
    LdRR(DE.low, BC.high);
    break;
  case 0x59:
    LdRR(DE.low, BC.low);
    break;
  case 0x5A:
    LdRR(DE.low, DE.high);
    break;
  case 0x5B:
    LdRR(DE.low, DE.low);
    break;
  case 0x5C:
    LdRR(DE.low, HL.high);
    break;
  case 0x5D:
    LdRR(DE.low, HL.low);
    break;
  case 0x5E:
    LdRIHl(DE.low);
    break;
  case 0x5F:
    LdRR(DE.low, AF.high);
    break;
  case 0x60:
    LdRR(HL.high, BC.high);
    break;
  case 0x61:
    LdRR(HL.high, BC.low);
    break;
  case 0x62:
    LdRR(HL.high, DE.high);
    break;
  case 0x63:
    LdRR(HL.high, DE.low);
    break;
  case 0x64:
    LdRR(HL.high, HL.high);
    break;
  case 0x65:
    LdRR(HL.high, HL.low);
    break;
  case 0x66:
    LdRIHl(HL.high);
    break;
  case 0x67:
    LdRR(HL.high, AF.high);
    break;
  case 0x68:
    LdRR(HL.low, BC.high);
    break;
  case 0x69:
    LdRR(HL.low, BC.low);
    break;
  case 0x6A:
    LdRR(HL.low, DE.high);
    break;
  case 0x6B:
    LdRR(HL.low, DE.low);
    break;
  case 0x6C:
    LdRR(HL.low, HL.high);
    break;
  case 0x6D:
    LdRR(HL.low, HL.low);
    break;
  case 0x6E:
    LdRIHl(HL.low);
    break;
  case 0x6F:
    LdRR(HL.low, AF.high);
    break;
  case 0x70:
    LdIHlR(BC.high);
    break;
  case 0x71:
    LdIHlR(BC.low);
    break;
  case 0x72:
    LdIHlR(DE.high);
    break;
  case 0x73:
    LdIHlR(DE.low);
    break;
  case 0x74:
    LdIHlR(HL.high);
    break;
  case 0x75:
    LdIHlR(HL.low);
    break;
  case 0x76:
    Halt();
    break;
  case 0x77:
    LdIHlR(AF.high);
    break;
  case 0x78:
    LdRR(AF.high, BC.high);
    break;
  case 0x79:
    LdRR(AF.high, BC.low);
    break;
  case 0x7A:
    LdRR(AF.high, DE.high);
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
  case 0x7E:
    LdRIHl(AF.high);
    break;
  case 0x7F:
    LdRR(AF.high, AF.high);
    break;
  case 0x80:
    AddR(BC.high);
    break;
  case 0x81:
    AddR(BC.low);
    break;
  case 0x82:
    AddR(DE.high);
    break;
  case 0x83:
    AddR(DE.low);
    break;
  case 0x84:
    AddR(HL.high);
    break;
  case 0x85:
    AddR(HL.low);
    break;
  case 0x86:
    AddAHl();
    break;
  case 0x87:
    AddR(AF.high);
    break;
  case 0x88:
    AdcR(BC.high);
    break;
  case 0x89:
    AdcR(BC.low);
    break;
  case 0x8A:
    AdcR(DE.high);
    break;
  case 0x8B:
    AdcR(DE.low);
    break;
  case 0x8C:
    AdcR(HL.high);
    break;
  case 0x8D:
    AdcR(HL.low);
    break;
  case 0x8E:
    AdcIHl();
    break;
  case 0x8F:
    AdcR(AF.high);
    break;
  case 0x90:
    SubR(BC.high);
    break;
  case 0x91:
    SubR(BC.low);
    break;
  case 0x92:
    SubR(DE.high);
    break;
  case 0x93:
    SubR(DE.low);
    break;
  case 0x94:
    SubR(HL.high);
    break;
  case 0x95:
    SubR(HL.low);
    break;
  case 0x96:
    SubIHl();
    break;
  case 0x97:
    SubR(AF.high);
    break;
  case 0x98:
    SbcR(BC.high);
    break;
  case 0x99:
    SbcR(BC.low);
    break;
  case 0x9A:
    SbcR(DE.high);
    break;
  case 0x9B:
    SbcR(DE.low);
    break;
  case 0x9C:
    SbcR(HL.high);
    break;
  case 0x9D:
    SbcR(HL.low);
    break;
  case 0x9E:
    SbcIHl();
    break;
  case 0x9F:
    SbcR(AF.high);
    break;
  case 0xA0:
    AndR(BC.high);
    break;
  case 0xA1:
    AndR(BC.low);
    break;
  case 0xA2:
    AndR(DE.high);
    break;
  case 0xA3:
    AndR(DE.low);
    break;
  case 0xA4:
    AndR(HL.high);
    break;
  case 0xA5:
    AndR(HL.low);
    break;
  case 0xA6:
    AndIHl();
    break;
  case 0xA7:
    AndR(AF.high);
    break;
  case 0xA8:
    XorR(BC.high);
    break;
  case 0xA9:
    XorR(BC.low);
    break;
  case 0xAA:
    XorR(DE.high);
    break;
  case 0xAB:
    XorR(DE.low);
    break;
  case 0xAC:
    XorR(HL.high);
    break;
  case 0xAD:
    XorR(HL.low);
    break;
  case 0xAE:
    XorIHl();
    break;
  case 0xAF:
    XorR(AF.high);
    break;
  case 0xB0:
    OrR(BC.high);
    break;
  case 0xB1:
    OrR(BC.low);
    break;
  case 0xB2:
    OrR(DE.high);
    break;
  case 0xB3:
    OrR(DE.low);
    break;
  case 0xB4:
    OrR(HL.high);
    break;
  case 0xB5:
    OrR(HL.low);
    break;
  case 0xB6:
    OrIHl();
    break;
  case 0xB7:
    OrR(AF.high);
    break;
  case 0xB8:
    CpR(BC.high);
    break;
  case 0xB9:
    CpR(BC.low);
    break;
  case 0xBA:
    CpR(DE.high);
    break;
  case 0xBB:
    CpR(DE.low);
    break;
  case 0xBC:
    CpR(HL.high);
    break;
  case 0xBD:
    CpR(HL.low);
    break;
  case 0xBE:
    CpIHl();
    break;
  case 0xBF:
    CpR(AF.high);
    break;
  case 0xC0:
    RetCc(!GetZ());
    break;
  case 0xC1:
    PopRr(BC);
    break;
  case 0xC2:
    JpCcU16(!GetZ());
    break;
  case 0xC3:
    JpCcU16(true); // unconditional jump
    break;
  case 0xC4:
    CAllCcU16(!GetZ());
    break;
  case 0xC5:
    PushRr(BC);
    break;
  case 0xC6:
    AddU8();
    break;
  case 0xC7:
    RstU8(0x00);
    break;
  case 0xC8:
    RetCc(GetZ());
    break;
  case 0xC9:
    RetCc(true); // unconditional return
    break;
  case 0xCA:
    JpCcU16(GetZ());
    break;
  case 0xCB:
    TickExtended();
    break;
  case 0xCC:
    CAllCcU16(GetZ());
    break;
  case 0xCD:
    CAllCcU16(true); // unconditional call
    break;
  case 0xCE:
    AdcU8();
    break;
  case 0xCF:
    RstU8(0x08);
    break;
  case 0xD0:
    RetCc(!GetCY());
    break;
  case 0xD1:
    PopRr(DE);
    break;
  case 0xD2:
    JpCcU16(!GetCY());
    break;
  case 0xD4:
    CAllCcU16(!GetCY());
    break;
  case 0xD5:
    PushRr(DE);
    break;
  case 0xD6:
    SubU8();
    break;
  case 0xD7:
    RstU8(0x10);
    break;
  case 0xD8:
    RetCc(GetCY());
    break;
  case 0xD9:
    RetI();
    break;
  case 0xDA:
    JpCcU16(GetCY());
    break;
  case 0xDC:
    CAllCcU16(GetCY());
    break;
  case 0xDE:
    SbcU8();
    break;
  case 0xDF:
    RstU8(0x18);
    break;
  case 0xE0:
    LdhU8A();
    break;
  case 0xE1:
    PopRr(HL);
    break;
  case 0xE2:
    LdhCA();
    break;
  case 0xE5:
    PushRr(HL);
    break;
  case 0xE6:
    AndU8();
    break;
  case 0xE7:
    RstU8(0x20);
    break;
  case 0xE8:
    AddSpS8();
    break;
  case 0xE9:
    JpHl();
    break;
  case 0xEA:
    LdU16A();
    break;
  case 0xEE:
    XorU8();
    break;
  case 0xEF:
    RstU8(0x28);
    break;
  case 0xF0:
    LdhAU8();
    break;
  case 0xF1:
  {
    PopRr(AF);
    AF.low &= 0xF0U; // clear unused lower nibble
  } break;
  case 0xF2:
    LdAC();
    break;
  case 0xF3:
    Di();
    break;
  case 0xF5:
    PushRr(AF);
    break;
  case 0xF6:
    OrU8();
    break;
  case 0xF7:
    RstU8(0x30);
    break;
  case 0xF8:
    LdHlS8();
    break;
  case 0xF9:
    LdSpHl();
    break;
  case 0xFA:
    LdAU16();
    break;
  case 0xFB:
    Ei();
    break;
  case 0xFE:
    CpU8();
    break;
  case 0xFF:
    RstU8(0x38U);
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
  case 0x00:
    Rlc(BC.high);
    break;
  case 0x01:
    Rlc(BC.low);
    break;
  case 0x02:
    Rlc(DE.high);
    break;
  case 0x03:
    Rlc(DE.low);
    break;
  case 0x04:
    Rlc(HL.high);
    break;
  case 0x05:
    Rlc(HL.low);
    break;
  case 0x06:
    Rlc(_mmu.Address(HL.reg));
    break;
  case 0x07:
    Rlc(AF.high);
    break;
  case 0x08:
    Rrc(BC.high);
    break;
  case 0x09:
    Rrc(BC.low);
    break;
  case 0x0A:
    Rrc(DE.high);
    break;
  case 0x0B:
    Rrc(DE.low);
    break;
  case 0x0C:
    Rrc(HL.high);
    break;
  case 0x0D:
    Rrc(HL.low);
    break;
  case 0x0E:
    Rrc(_mmu.Address(HL.reg));
    break;
  case 0x0F:
    Rrc(AF.high);
    break;
  case 0x10:
    Rl(BC.high);
    break;
  case 0x11:
    Rl(BC.low);
    break;
  case 0x12:
    Rl(DE.high);
    break;
  case 0x13:
    Rl(DE.low);
    break;
  case 0x14:
    Rl(HL.high);
    break;
  case 0x15:
    Rl(HL.low);
    break;
  case 0x16:
    Rl(_mmu.Address(HL.reg));
    break;
  case 0x17:
    Rl(AF.high); break;
  case 0x18:
    Rr(BC.high);
    break;
  case 0x19:
    Rr(BC.low);
    break;
  case 0x1A:
    Rr(DE.high);
    break;
  case 0x1B:
    Rr(DE.low);
    break;
  case 0x1C:
    Rr(HL.high);
    break;
  case 0x1D:
    Rr(HL.low);
    break;
  case 0x1E:
    Rr(_mmu.Address(HL.reg));
    break;
  case 0x1F:
    Rr(AF.high);
    break;
  case 0x20:
    Sla(BC.high);
    break;
  case 0x21:
    Sla(BC.low);
    break;
  case 0x22:
    Sla(DE.high);
    break;
  case 0x23:
    Sla(DE.low);
    break;
  case 0x24:
    Sla(HL.high);
    break;
  case 0x25:
    Sla(HL.low);
    break;
  case 0x26:
    Sla(_mmu.Address(HL.reg));
    break;
  case 0x27:
    Sla(AF.high);
    break;
  case 0x28:
    Sra(BC.high);
    break;
  case 0x29:
    Sra(BC.low);
    break;
  case 0x2A:
    Sra(DE.high);
    break;
  case 0x2B:
    Sra(DE.low);
    break;
  case 0x2C:
    Sra(HL.high);
    break;
  case 0x2D:
    Sra(HL.low);
    break;
  case 0x2E:
    Sra(_mmu.Address(HL.reg));
    break;
  case 0x2F:
    Sra(AF.high);
    break;
  case 0x30:
    Swap(BC.high);
    break;
  case 0x31:
    Swap(BC.low);
    break;
  case 0x32:
    Swap(DE.high);
    break;
  case 0x33:
    Swap(DE.low);
    break;
  case 0x34:
    Swap(HL.high);
    break;
  case 0x35:
    Swap(HL.low);
    break;
  case 0x36:
    Swap(_mmu.Address(HL.reg));
    break;
  case 0x37:
    Swap(AF.high);
    break;
  case 0x38:
    Srl(BC.high);
    break;
  case 0x39:
    Srl(BC.low);
    break;
  case 0x3A:
    Srl(DE.high);
    break;
  case 0x3B:
    Srl(DE.low);
    break;
  case 0x3C:
    Srl(HL.high);
    break;
  case 0x3D:
    Srl(HL.low);
    break;
  case 0x3E:
    Srl(_mmu.Address(HL.reg));
    break;
  case 0x3F:
    Srl(AF.high);
    break;
  case 0x40:
    Bit(BC.high, 0);
    break;
  case 0x41:
    Bit(BC.low, 0);
    break;
  case 0x42:
    Bit(DE.high, 0);
    break;
  case 0x43:
    Bit(DE.low, 0);
    break;
  case 0x44:
    Bit(HL.high, 0);
    break;
  case 0x45:
    Bit(HL.low, 0);
    break;
  case 0x46:
    Bit(_mmu.Read(HL.reg), 0);
    break;
  case 0x47:
    Bit(AF.high, 0);
    break;
  case 0x48:
    Bit(BC.high, 1);
    break;
  case 0x49:
    Bit(BC.low, 1);
    break;
  case 0x4A:
    Bit(DE.high, 1);
    break;
  case 0x4B:
    Bit(DE.low, 1);
    break;
  case 0x4C:
    Bit(HL.high, 1);
    break;
  case 0x4D:
    Bit(HL.low, 1);
    break;
  case 0x4E:
    Bit(_mmu.Read(HL.reg), 1);
    break;
  case 0x4F:
    Bit(AF.high, 1);
    break;
  case 0x50:
    Bit(BC.high, 2);
    break;
  case 0x51:
    Bit(BC.low, 2);
    break;
  case 0x52:
    Bit(DE.high, 2);
    break;
  case 0x53:
    Bit(DE.low, 2);
    break;
  case 0x54:
    Bit(HL.high, 2);
    break;
  case 0x55:
    Bit(HL.low, 2);
    break;
  case 0x56:
    Bit(_mmu.Read(HL.reg), 2);
    break;
  case 0x57:
    Bit(AF.high, 2);
    break;
  case 0x58:
    Bit(BC.high, 3);
    break;
  case 0x59:
    Bit(BC.low, 3);
    break;
  case 0x5A:
    Bit(DE.high, 3);
    break;
  case 0x5B:
    Bit(DE.low, 3);
    break;
  case 0x5C:
    Bit(HL.high, 3);
    break;
  case 0x5D:
    Bit(HL.low, 3);
    break;
  case 0x5E:
    Bit(_mmu.Read(HL.reg), 3);
    break;
  case 0x5F:
    Bit(AF.high, 3);
    break;
  case 0x60:
    Bit(BC.high, 4);
    break;
  case 0x61:
    Bit(BC.low, 4);
    break;
  case 0x62:
    Bit(DE.high, 4);
    break;
  case 0x63:
    Bit(DE.low, 4);
    break;
  case 0x64:
    Bit(HL.high, 4);
    break;
  case 0x65:
    Bit(HL.low, 4);
    break;
  case 0x66:
    Bit(_mmu.Read(HL.reg), 4);
    break;
  case 0x67:
    Bit(AF.high, 4);
    break;
  case 0x68:
    Bit(BC.high, 5);
    break;
  case 0x69:
    Bit(BC.low, 5);
    break;
  case 0x6A:
    Bit(DE.high, 5);
    break;
  case 0x6B:
    Bit(DE.low, 5);
    break;
  case 0x6C:
    Bit(HL.high, 5);
    break;
  case 0x6D:
    Bit(HL.low, 5);
    break;
  case 0x6E:
    Bit(_mmu.Read(HL.reg), 5);
    break;
  case 0x6F:
    Bit(AF.high, 5);
    break;
  case 0x70:
    Bit(BC.high, 6);
    break;
  case 0x71:
    Bit(BC.low, 6);
    break;
  case 0x72:
    Bit(DE.high, 6);
    break;
  case 0x73:
    Bit(DE.low, 6);
    break;
  case 0x74:
    Bit(HL.high, 6);
    break;
  case 0x75:
    Bit(HL.low, 6);
    break;
  case 0x76:
    Bit(_mmu.Read(HL.reg), 6);
    break;
  case 0x77:
    Bit(AF.high, 6);
    break;
  case 0x78:
    Bit(BC.high, 7);
    break;
  case 0x79:
    Bit(BC.low, 7);
    break;
  case 0x7A:
    Bit(DE.high, 7);
    break;
  case 0x7B:
    Bit(DE.low, 7);
    break;
  case 0x7C:
    Bit(HL.high, 7);
    break;
  case 0x7D:
    Bit(HL.low, 7);
    break;
  case 0x7E:
    Bit(_mmu.Read(HL.reg), 7);
    break;
  case 0x7F:
    Bit(AF.high, 7);
    break;
  case 0x80:
    Res(BC.high, 0);
    break;
  case 0x81:
    Res(BC.low, 0);
    break;
  case 0x82:
    Res(DE.high, 0);
    break;
  case 0x83:
    Res(DE.low, 0);
    break;
  case 0x84:
    Res(HL.high, 0);
    break;
  case 0x85:
    Res(HL.low, 0);
    break;
  case 0x86:
    Res(_mmu.Address(HL.reg), 0);
    break;
  case 0x87:
    Res(AF.high, 0);
    break;
  case 0x88:
    Res(BC.high, 1);
    break;
  case 0x89:
    Res(BC.low, 1);
    break;
  case 0x8A:
    Res(DE.high, 1);
    break;
  case 0x8B:
    Res(DE.low, 1);
    break;
  case 0x8C:
    Res(HL.high, 1);
    break;
  case 0x8D:
    Res(HL.low, 1);
    break;
  case 0x8E:
    Res(_mmu.Address(HL.reg), 1);
    break;
  case 0x8F:
    Res(AF.high, 1);
    break;
  case 0x90:
    Res(BC.high, 2);
    break;
  case 0x91:
    Res(BC.low, 2);
    break;
  case 0x92:
    Res(DE.high, 2);
    break;
  case 0x93:
    Res(DE.low, 2);
    break;
  case 0x94:
    Res(HL.high, 2);
    break;
  case 0x95:
    Res(HL.low, 2);
    break;
  case 0x96:
    Res(_mmu.Address(HL.reg), 2);
    break;
  case 0x97:
    Res(AF.high, 2);
    break;
  case 0x98:
    Res(BC.high, 3);
    break;
  case 0x99:
    Res(BC.low, 3);
    break;
  case 0x9A:
    Res(DE.high, 3);
    break;
  case 0x9B:
    Res(DE.low, 3);
    break;
  case 0x9C:
    Res(HL.high, 3);
    break;
  case 0x9D:
    Res(HL.low, 3);
    break;
  case 0x9E:
    Res(_mmu.Address(HL.reg), 3);
    break;
  case 0x9F:
    Res(AF.high, 3);
    break;
  case 0xA0:
    Res(BC.high, 4);
    break;
  case 0xA1:
    Res(BC.low, 4);
    break;
  case 0xA2:
    Res(DE.high, 4);
    break;
  case 0xA3:
    Res(DE.low, 4);
    break;
  case 0xA4:
    Res(HL.high, 4);
    break;
  case 0xA5:
    Res(HL.low, 4);
    break;
  case 0xA6:
    Res(_mmu.Address(HL.reg), 4);
    break;
  case 0xA7:
    Res(AF.high, 4);
    break;
  case 0xA8:
    Res(BC.high, 5);
    break;
  case 0xA9:
    Res(BC.low, 5);
    break;
  case 0xAA:
    Res(DE.high, 5);
    break;
  case 0xAB:
    Res(DE.low, 5);
    break;
  case 0xAC:
    Res(HL.high, 5);
    break;
  case 0xAD:
    Res(HL.low, 5);
    break;
  case 0xAE:
    Res(_mmu.Address(HL.reg), 5);
    break;
  case 0xAF:
    Res(AF.high, 5);
    break;
  case 0xB0:
    Res(BC.high, 6);
    break;
  case 0xB1:
    Res(BC.low, 6);
    break;
  case 0xB2:
    Res(DE.high, 6);
    break;
  case 0xB3:
    Res(DE.low, 6);
    break;
  case 0xB4:
    Res(HL.high, 6);
    break;
  case 0xB5:
    Res(HL.low, 6);
    break;
  case 0xB6:
    Res(_mmu.Address(HL.reg), 6);
    break;
  case 0xB7:
    Res(AF.high, 6);
    break;
  case 0xB8:
    Res(BC.high, 7);
    break;
  case 0xB9:
    Res(BC.low, 7);
    break;
  case 0xBA:
    Res(DE.high, 7);
    break;
  case 0xBB:
    Res(DE.low, 7);
    break;
  case 0xBC:
    Res(HL.high, 7);
    break;
  case 0xBD:
    Res(HL.low, 7);
    break;
  case 0xBE:
    Res(_mmu.Address(HL.reg), 7);
    break;
  case 0xBF:
    Res(AF.high, 7);
    break;
  case 0xC0:
    Set(BC.high, 0);
    break;
  case 0xC1:
    Set(BC.low, 0);
    break;
  case 0xC2:
    Set(DE.high, 0);
    break;
  case 0xC3:
    Set(DE.low, 0);
    break;
  case 0xC4:
    Set(HL.high, 0);
    break;
  case 0xC5:
    Set(HL.low, 0);
    break;
  case 0xC6:
    Set(_mmu.Address(HL.reg), 0);
    break;
  case 0xC7:
    Set(AF.high, 0);
    break;
  case 0xC8:
    Set(BC.high, 1);
    break;
  case 0xC9:
    Set(BC.low, 1);
    break;
  case 0xCA:
    Set(DE.high, 1);
    break;
  case 0xCB:
    Set(DE.low, 1);
    break;
  case 0xCC:
    Set(HL.high, 1);
    break;
  case 0xCD:
    Set(HL.low, 1);
    break;
  case 0xCE:
    Set(_mmu.Address(HL.reg), 1);
    break;
  case 0xCF:
    Set(AF.high, 1);
    break;
  case 0xD0:
    Set(BC.high, 2);
    break;
  case 0xD1:
    Set(BC.low, 2);
    break;
  case 0xD2:
    Set(DE.high, 2);
    break;
  case 0xD3:
    Set(DE.low, 2);
    break;
  case 0xD4:
    Set(HL.high, 2);
    break;
  case 0xD5:
    Set(HL.low, 2);
    break;
  case 0xD6:
    Set(_mmu.Address(HL.reg), 2);
    break;
  case 0xD7:
    Set(AF.high, 2);
    break;
  case 0xD8:
    Set(BC.high, 3);
    break;
  case 0xD9:
    Set(BC.low, 3);
    break;
  case 0xDA:
    Set(DE.high, 3);
    break;
  case 0xDB:
    Set(DE.low, 3);
    break;
  case 0xDC:
    Set(HL.high, 3);
    break;
  case 0xDD:
    Set(HL.low, 3);
    break;
  case 0xDE:
    Set(_mmu.Address(HL.reg), 3);
    break;
  case 0xDF:
    Set(AF.high, 3);
    break;
  case 0xE0:
    Set(BC.high, 4);
    break;
  case 0xE1:
    Set(BC.low, 4);
    break;
  case 0xE2:
    Set(DE.high, 4);
    break;
  case 0xE3:
    Set(DE.low, 4);
    break;
  case 0xE4:
    Set(HL.high, 4);
    break;
  case 0xE5:
    Set(HL.low, 4);
    break;
  case 0xE6:
    Set(_mmu.Address(HL.reg), 4);
    break;
  case 0xE7:
    Set(AF.high, 4);
    break;
  case 0xE8:
    Set(BC.high, 5);
    break;
  case 0xE9:
    Set(BC.low, 5);
    break;
  case 0xEA:
    Set(DE.high, 5);
    break;
  case 0xEB:
    Set(DE.low, 5);
    break;
  case 0xEC:
    Set(HL.high, 5);
    break;
  case 0xED:
    Set(HL.low, 5);
    break;
  case 0xEE:
    Set(_mmu.Address(HL.reg), 5);
    break;
  case 0xEF:
    Set(AF.high, 5);
    break;
  case 0xF0:
    Set(BC.high, 6);
    break;
  case 0xF1:
    Set(BC.low, 6);
    break;
  case 0xF2:
    Set(DE.high, 6);
    break;
  case 0xF3:
    Set(DE.low, 6);
    break;
  case 0xF4:
    Set(HL.high, 6);
    break;
  case 0xF5:
    Set(HL.low, 6);
    break;
  case 0xF6:
    Set(_mmu.Address(HL.reg), 6);
    break;
  case 0xF7:
    Set(AF.high, 6);
    break;
  case 0xF8:
    Set(BC.high, 7);
    break;
  case 0xF9:
    Set(BC.low, 7);
    break;
  case 0xFA:
    Set(DE.high, 7);
    break;
  case 0xFB:
    Set(DE.low, 7);
    break;
  case 0xFC:
    Set(HL.high, 7);
    break;
  case 0xFD:
    Set(HL.low, 7);
    break;
  case 0xFE:
    Set(_mmu.Address(HL.reg), 7);
    break;
  case 0xFF:
    Set(AF.high, 7);
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
  _mmu.Write(SP.reg, PC.high);
  SP.reg--;
  _mmu.Write(SP.reg, PC.low);

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

void Cpu::DecHl()
{
  uint8_t data = _mmu.Read(HL.reg);
  uint16_t res = data - 1;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((data & 0x0FU) - 1U) > 0x0F);

  _mmu.Write(HL.reg, (res & 0xFFU));
}

void Cpu::LdAU16()
{
  uint8_t lsb = _mmu.Read(PC.reg++);
  uint8_t msb = _mmu.Read(PC.reg++);
  AF.high = _mmu.Read(ToU16(msb, lsb));
}

void Cpu::LdSpHl()
{
  SP.reg = HL.reg;
}

void Cpu::LdHlS8()
{
  auto i8 = static_cast<int8_t>(_mmu.Read(PC.reg++));
  uint16_t res = SP.reg + i8;
  SetZ(false);
  SetN(false);
  SetH(((SP.reg ^ i8 ^ res) & 0x10) != 0);
  SetCY(((SP.reg ^ i8 ^ res) & 0x100) != 0);

  HL.reg = res;
}

void Cpu::OrU8()
{
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint16_t res = static_cast<uint16_t>(AF.high) | static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::LdAC()
{
  AF.high = _mmu.Read(0xFF00 + BC.low);
}

void Cpu::AndU8()
{
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint8_t res = AF.high & u8;
  SetZ(res == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  AF.high = res;
}

void Cpu::AddSpS8()
{
  auto i8 = static_cast<int8_t>(_mmu.Read(PC.reg++));
  uint16_t res = SP.reg + i8;

  SetZ(false);
  SetN(false);
  SetH(((SP.reg ^ i8 ^ res) & 0x10) != 0); // Half-carry detection
  SetCY(((SP.reg ^ i8 ^ res) & 0x100) != 0);

  SP.reg = res;
}

void Cpu::JpHl()
{
  PC.reg = HL.reg;
}

void Cpu::XorU8()
{
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint8_t res = AF.high ^ u8;
  SetZ(res == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  AF.high = res;
}

void Cpu::RetI()
{
  uint8_t lsb = _mmu.Read(SP.reg++);
  uint8_t msb = _mmu.Read(SP.reg++);
  PC.reg = ToU16(msb, lsb);
  _interrupt->_ime = true;
}

void Cpu::SubU8()
{
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::SbcU8()
{
  auto c = static_cast<uint16_t>((AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::AddU8()
{
  uint8_t u8 = _mmu.Read(PC.reg++);
  AddR(u8);
}

void Cpu::AdcU8()
{
  auto c = static_cast<uint16_t>((AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(PC.reg++);
  uint16_t res = static_cast<uint16_t>(AF.high) + static_cast<uint16_t>(u8) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((AF.high & 0xFU) + (u8 & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::AndR(std::uint8_t reg)
{
  uint16_t res = static_cast<uint16_t>(AF.high) & static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::AndIHl()
{
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) & static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::XorR(std::uint8_t reg)
{
  uint16_t res = static_cast<uint16_t>(AF.high) ^ static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::XorIHl()
{
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) ^ static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::OrR(std::uint8_t reg)
{
  uint16_t res = static_cast<uint16_t>(AF.high) | static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::OrIHl()
{
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) | static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  AF.high = (res & 0xFFU);
}

void Cpu::CpR(std::uint8_t reg)
{
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::CpIHl()
{
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::SubIHl()
{
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::SbcIHl()
{
  auto c = static_cast<uint16_t>((AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(u8) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (u8 & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::SbcR(std::uint8_t reg)
{
  auto c = static_cast<uint16_t>((AF.low & (1U << 4U)) >> 4U);
  uint16_t res = static_cast<uint16_t>(AF.high) - static_cast<uint16_t>(reg) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((AF.high & 0xFU) - (reg & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::AdcIHl()
{
  auto c = static_cast<uint16_t>((AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(HL.reg);
  uint16_t res = static_cast<uint16_t>(AF.high) + static_cast<uint16_t>(u8) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((AF.high & 0xFU) + (u8 & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::AddR(std::uint8_t reg)
{
  uint16_t res = static_cast<uint16_t>(AF.high) + static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((AF.high ^ reg ^ res) & 0x10) != 0);
  SetCY(((AF.high ^ reg ^ res) & 0x100) != 0);

  AF.high = (res & 0xFFU);
}

void Cpu::AdcR(std::uint8_t reg)
{
  auto c = static_cast<uint16_t>((AF.low & (1U << 4U)) >> 4U);
  uint16_t res = static_cast<uint16_t>(AF.high) + static_cast<uint16_t>(reg) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((AF.high & 0xFU) + (reg & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  AF.high = (res & 0xFFU);
}

void Cpu::Halt()
{
  throw std::runtime_error(std::format("Unimplemented instruction: 0x76 (HALT)"));
}

void Cpu::Ccf()
{
  SetN(false);
  SetH(false);

  AF.low = AF.low ^ (1U << 4U);
}

void Cpu::LdAHlN()
{
  _mmu.Write(HL.reg, AF.high);
  HL.reg = HL.reg - 1;
}

void Cpu::Scf()
{
  SetN(false);
  SetH(false);
  SetCY(true);
}

void Cpu::IncIHl()
{
  uint8_t data = _mmu.Read(HL.reg);
  uint16_t res = data + 1;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((data & 0x0FU) + 1U) > 0x0F);

  _mmu.Write(HL.reg, (res & 0xFFU));
}

void Cpu::Cpl()
{
  SetN(true);
  SetH(true);

  AF.high = ~AF.high;
}

// DAA (https://forums.nesdev.org/viewtopic.php?p=196282&sid=a1cdd6adc0b01ea3d77f61aee9527449#p196282)
void Cpu::Daa()
{
  if (!(AF.low & (1U << 6U)))
  {
    if ((AF.low & (1U << 4U)) || AF.high > 0x99)
    {
      AF.high += 0x60;
      AF.low = (AF.low & ~(1U << 4U) | (1U << 4U));
    }
    if ((AF.low & (1U << 5U)) || (AF.high & 0x0FU) > 0x09)
    {
      AF.high += 0x06;
    }
  }
  else
  {
    if (AF.low & (1U << 4U))
    {
      AF.high -= 0x60;
    }
    if (AF.low & (1U << 5U))
    {
      AF.high -= 0x06;
    }
  }

  SetZ(AF.high == 0);
  SetH(false);
}

void Cpu::Rra()
{
  uint8_t oldCY = (AF.low & (1U << 4U)) >> 4U;

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY((AF.high & (1U << 0U)));

  AF.high = AF.high >> 1U;
  AF.high = (AF.high & ~(1U << 7U)) | static_cast<uint8_t>(oldCY << 7U);
}

void Cpu::Stop()
{
  // TODO: check what needs to be done here
  SPDLOG_WARN(std::format("Unimplemented instruction: 0x10 (STOP)"));
}

void Cpu::Rrca()
{
  uint8_t b0 = (AF.high & (1U << 0U));

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY(b0);

  AF.high = AF.high >> 1U;
  AF.high = (AF.high & ~(1U << 7U)) | static_cast<uint8_t>(b0 << 7U);
}

void Cpu::LdDU16Sp()
{
  uint8_t lsb = _mmu.Read(PC.reg++);
  uint8_t msb = _mmu.Read(PC.reg++);
  uint16_t nn = ToU16(msb, lsb);
  _mmu.Write(nn, SP.low);
  ++nn;
  _mmu.Write(nn, SP.high);
}

void Cpu::Rlca()
{
  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY((AF.high & (1U << 7U)) >> 7U);

  AF.high = AF.high << 1U;
  AF.high = ((AF.high & ~(1U << 0U)) | ((AF.low & (1U << 4U)) >> 4U));
}

void Cpu::LdIRrA(Register& reg)
{
  _mmu.Write(reg.reg, AF.high);
}

void Cpu::AddHlRr(Register& reg)
{
  uint32_t res = static_cast<uint32_t>(HL.reg) + static_cast<uint32_t>(reg.reg);

  SetN(false);
  SetH(((HL.reg & 0xFFFU) + (reg.reg & 0xFFFU)) > 0xFFFU);
  SetCY(res > 0xFFFFU);

  HL.reg = res & 0xFFFFU;
}

void Cpu::RstU8(std::uint8_t addr)
{
  SP.reg--;
  _mmu.Write(SP.reg, PC.high);
  SP.reg--;
  _mmu.Write(SP.reg, PC.low);
  PC.reg = ToU16(addr, 0x00);
}

void Cpu::LdRIHl(std::uint8_t& reg)
{
  reg = _mmu.Read(HL.reg);
}

void Cpu::Ei()
{
  _interrupt->_enableRequested = true;
  SPDLOG_DEBUG("Enable interrupt requested");
}

void Cpu::DecRr(Register& reg)
{
  reg.reg--;
}

void Cpu::LdAHlP()
{
  AF.high = _mmu.Read(HL.reg);
  HL.reg++;
}

void Cpu::LdHlU8()
{
  auto data = _mmu.Read(PC.reg++);
  _mmu.Write(HL.reg, data);
}

void Cpu::Di()
{
  _interrupt->_enableRequested = false;
  _interrupt->_ime = false;
  SPDLOG_DEBUG("Interrupt disabled");
}

void Cpu::JpCcU16(bool cc)
{
  if (cc)
  {
    auto lsb = _mmu.Read(PC.reg++);
    auto msb = _mmu.Read(PC.reg++);
    PC.reg = ToU16(lsb, msb);
  }
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
void Cpu::CpU8()
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

void Cpu::PopRr(Register& reg)
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

void Cpu::PushRr(Register& reg)
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

void Cpu::CAllCcU16(bool cc)
{
  if (cc)
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
}

void Cpu::RetCc(bool cc)
{
  if (cc)
  {
    auto low = _mmu.Read(SP.reg);
    ++SP.reg;
    auto high = _mmu.Read(SP.reg);
    ++SP.reg;
    PC.reg = ToU16(low, high);
  }
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

void Cpu::LdAIRr(Register& reg)
{
  AF.high = _mmu.Read(reg.reg);
}

void Cpu::LdHlMA()
{
  _mmu.Write(HL.reg, AF.high);
  --HL.reg;
}

void Cpu::LdIHlR(std::uint8_t& reg)
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

void Cpu::IncRr(Register& reg)
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

// extended opcodes
// NOLINTBEGIN(readability-convert-member-functions-to-static)
void Cpu::Rlc(uint8_t& reg)
{
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  reg = reg << 1U;
  reg = (reg & ~(1U << 0U)) | (bit7);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
}

void Cpu::Rrc(uint8_t& reg)
{
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;
  reg = (reg & ~(1U << 7U)) | (bit0 << 7U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
}

void Cpu::Rl(uint8_t& reg)
{
  uint8_t oldCY = (AF.low & (1U << 4U)) >> 4U;
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  reg = reg << 1U;
  reg = (reg & ~(1U << 0U)) | (oldCY);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
}

void Cpu::Rr(uint8_t& reg)
{
  uint8_t oldCY = (AF.low & (1U << 4U)) >> 4U;
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;
  reg = (reg & ~(1U << 7U)) | (oldCY << 7U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
}

void Cpu::Sla(uint8_t& reg)
{
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  reg = reg << 1U;

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
}

void Cpu::Sra(uint8_t& reg)
{
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;
  reg = (reg & ~(1U << 7U)) | (bit7 << 7U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
}

void Cpu::Srl(uint8_t& reg)
{
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
}

void Cpu::Swap(uint8_t& reg)
{
  uint8_t lowerNibble = (reg & 0x0FU);
  reg = reg >> 4U;
  reg = (reg & (0x0FU)) | (lowerNibble << 4U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(false);
}


void Cpu::Bit(uint8_t reg, uint8_t bit)
{
  uint8_t bitX = (reg & (1U << bit)) >> bit;
  AF.low = (AF.low & ~(1U << 7U)) | ((!bitX) << 7U);
  SetN(false);
  SetH(true);
}

void Cpu::Res(uint8_t& reg, uint8_t bit)
{
  reg = (reg & ~(1U << bit));
}

void Cpu::Set(uint8_t& reg, uint8_t bit)
{
  reg = (reg & ~(1U << bit)) | (1U << bit);
}

// NOLINTEND(readability-convert-member-functions-to-static)

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
