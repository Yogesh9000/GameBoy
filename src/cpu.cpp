#include "cpu.hpp"

#include <spdlog/spdlog.h>

#include <format>
#include <memory>
#include <utility>

#include "bitutils.hpp"
#include "common.hpp"
#include "logmanager.hpp"

// NOLINTBEGIN(readability-suspicious-call-argument, hicpp-signed-bitwise,
// readability-convert-member-functions-to-static)

Cpu::Cpu(MemoryManagementUnit& mmu) : _state{}, _mmu(mmu)
{
  _state._interrupt = std::make_shared<Interrupt>();
  _mmu.AddMemoryRange(_state._interrupt);
  _logger = LogManager::GetLogger("Cpu");
}

Cpu::Cpu(CpuState state, MemoryManagementUnit& mmu)
    : _state(std::move(state)), _mmu(mmu)
{
  if (!_state._interrupt)
  {
    _state._interrupt = std::make_shared<Interrupt>();
  }
  _mmu.AddMemoryRange(_state._interrupt);
  _logger = LogManager::GetLogger("Cpu");
}

[[nodiscard]]
CpuState Cpu::GetCpuState() const
{
  return _state;
}

// TODO: Implement HALT BUG
void Cpu::Tick()
{
  LOG_TRACE(_logger,
      "A:{:02X} F:{:02X} B:{:02X} C:{:02X} D:{:02X} E:{:02X} H:{:02X} L:{:02X} SP:{:04X} PC:{:04X} PCMEM:{:02X},{:02X},{:02X},{:02X}",
      _state.AF.high, _state.AF.low, _state.BC.high, _state.BC.low,
      _state.DE.high, _state.DE.low, _state.HL.high, _state.HL.low,
      _state.SP.reg, _state.PC.reg, _mmu.Read(_state.PC.reg),
      _mmu.Read(_state.PC.reg + 1), _mmu.Read(_state.PC.reg + 2),
      _mmu.Read(_state.PC.reg + 3));

  auto opcode = _mmu.Read(_state.PC.reg++);

  HandleInterruptsIfAny();

  if (_state._interrupt->_enableRequested)
  {
    _state._interrupt->_enableRequested = false;
    _state._interrupt->_ime = true;
    LOG_DEBUG(_logger, "Interrupt enabled");
  }

  switch (opcode)
  {
    case 0x00:
      Nop();
      break;
    case 0x01:
      LdRrU16(_state.BC);
      break;
    case 0x02:
      LdIRrA(_state.BC);
      break;
    case 0x03:
      IncRr(_state.BC);
      break;
    case 0x04:
      IncR(_state.BC.high);
      break;
    case 0x05:
      DecR(_state.BC.high);
      break;
    case 0x06:
      LdRU8(_state.BC.high);
      break;
    case 0x07:
      Rlca();
      break;
    case 0x08:
      LdDU16Sp();
      break;
    case 0x09:
      AddHlRr(_state.BC);
      break;
    case 0x0A:
      LdAIRr(_state.BC);
      break;
    case 0x0B:
      DecRr(_state.BC);
      break;
    case 0x0C:
      IncR(_state.BC.low);
      break;
    case 0x0D:
      DecR(_state.BC.low);
      break;
    case 0x0E:
      LdRU8(_state.BC.low);
      break;
    case 0x0F:
      Rrca();
      break;
    case 0x10:
      Stop();
      break;
    case 0x11:
      LdRrU16(_state.DE);
      break;
    case 0x12:
      LdIRrA(_state.DE);
      break;
    case 0x13:
      IncRr(_state.DE);
      break;
    case 0x14:
      IncR(_state.DE.high);
      break;
    case 0x15:
      DecR(_state.DE.high);
      break;
    case 0x16:
      LdRU8(_state.DE.high);
      break;
    case 0x17:
      RlA();
      break;
    case 0x18:
      JrCCI8(true);
      break;
    case 0x19:
      AddHlRr(_state.DE);
      break;
    case 0x1A:
      LdAIRr(_state.DE);
      break;
    case 0x1B:
      DecRr(_state.DE);
      break;
    case 0x1C:
      IncR(_state.DE.low);
      break;
    case 0x1D:
      DecR(_state.DE.low);
      break;
    case 0x1E:
      LdRU8(_state.DE.low);
      break;
    case 0x1F:
      Rra();
      break;
    case 0x20:
      JrCCI8(!GetZ());
      break;
    case 0x21:
      LdRrU16(_state.HL);
      break;
    case 0x22:
      LdHlPA();
      break;
    case 0x23:
      IncRr(_state.HL);
      break;
    case 0x24:
      IncR(_state.HL.high);
      break;
    case 0x25:
      DecR(_state.HL.high);
      break;
    case 0x26:
      LdRU8(_state.HL.high);
      break;
    case 0x27:
      Daa();
      break;
    case 0x28:
      JrCCI8(GetZ());
      break;
    case 0x29:
      AddHlRr(_state.HL);
      break;
    case 0x2A:
      LdAHlP();
      break;
    case 0x2B:
      DecRr(_state.HL);
      break;
    case 0x2C:
      IncR(_state.HL.low);
      break;
    case 0x2D:
      DecR(_state.HL.low);
      break;
    case 0x2E:
      LdRU8(_state.HL.low);
      break;
    case 0x2F:
      Cpl();
      break;
    case 0x30:
      JrCCI8(!GetCY());
      break;
    case 0x31:
      LdRrU16(_state.SP);
      break;
    case 0x32:
      LdHlMA();
      break;
    case 0x33:
      IncRr(_state.SP);
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
      AddHlRr(_state.SP);
      break;
    case 0x3A:
      LdAHlN();
      break;
    case 0x3B:
      DecRr(_state.SP);
      break;
    case 0x3C:
      IncR(_state.AF.high);
      break;
    case 0x3D:
      DecR(_state.AF.high);
      break;
    case 0x3E:
      LdRU8(_state.AF.high);
      break;
    case 0x3F:
      Ccf();
      break;
    case 0x40:
      LdRR(_state.BC.high, _state.BC.high);
      break;
    case 0x41:
      LdRR(_state.BC.high, _state.BC.low);
      break;
    case 0x42:
      LdRR(_state.BC.high, _state.DE.high);
      break;
    case 0x43:
      LdRR(_state.BC.high, _state.DE.low);
      break;
    case 0x44:
      LdRR(_state.BC.high, _state.HL.high);
      break;
    case 0x45:
      LdRR(_state.BC.high, _state.HL.low);
      break;
    case 0x46:
      LdRIHl(_state.BC.high);
      break;
    case 0x47:
      LdRR(_state.BC.high, _state.AF.high);
      break;
    case 0x48:
      LdRR(_state.BC.low, _state.BC.high);
      break;
    case 0x49:
      LdRR(_state.BC.low, _state.BC.low);
      break;
    case 0x4A:
      LdRR(_state.BC.low, _state.DE.high);
      break;
    case 0x4B:
      LdRR(_state.BC.low, _state.DE.low);
      break;
    case 0x4C:
      LdRR(_state.BC.low, _state.HL.high);
      break;
    case 0x4D:
      LdRR(_state.BC.low, _state.HL.low);
      break;
    case 0x4E:
      LdRIHl(_state.BC.low);
      break;
    case 0x4F:
      LdRR(_state.BC.low, _state.AF.high);
      break;
    case 0x50:
      LdRR(_state.DE.high, _state.BC.high);
      break;
    case 0x51:
      LdRR(_state.DE.high, _state.BC.low);
      break;
    case 0x52:
      LdRR(_state.DE.high, _state.DE.high);
      break;
    case 0x53:
      LdRR(_state.DE.high, _state.DE.low);
      break;
    case 0x54:
      LdRR(_state.DE.high, _state.HL.high);
      break;
    case 0x55:
      LdRR(_state.DE.high, _state.HL.low);
      break;
    case 0x56:
      LdRIHl(_state.DE.high);
      break;
    case 0x57:
      LdRR(_state.DE.high, _state.AF.high);
      break;
    case 0x58:
      LdRR(_state.DE.low, _state.BC.high);
      break;
    case 0x59:
      LdRR(_state.DE.low, _state.BC.low);
      break;
    case 0x5A:
      LdRR(_state.DE.low, _state.DE.high);
      break;
    case 0x5B:
      LdRR(_state.DE.low, _state.DE.low);
      break;
    case 0x5C:
      LdRR(_state.DE.low, _state.HL.high);
      break;
    case 0x5D:
      LdRR(_state.DE.low, _state.HL.low);
      break;
    case 0x5E:
      LdRIHl(_state.DE.low);
      break;
    case 0x5F:
      LdRR(_state.DE.low, _state.AF.high);
      break;
    case 0x60:
      LdRR(_state.HL.high, _state.BC.high);
      break;
    case 0x61:
      LdRR(_state.HL.high, _state.BC.low);
      break;
    case 0x62:
      LdRR(_state.HL.high, _state.DE.high);
      break;
    case 0x63:
      LdRR(_state.HL.high, _state.DE.low);
      break;
    case 0x64:
      LdRR(_state.HL.high, _state.HL.high);
      break;
    case 0x65:
      LdRR(_state.HL.high, _state.HL.low);
      break;
    case 0x66:
      LdRIHl(_state.HL.high);
      break;
    case 0x67:
      LdRR(_state.HL.high, _state.AF.high);
      break;
    case 0x68:
      LdRR(_state.HL.low, _state.BC.high);
      break;
    case 0x69:
      LdRR(_state.HL.low, _state.BC.low);
      break;
    case 0x6A:
      LdRR(_state.HL.low, _state.DE.high);
      break;
    case 0x6B:
      LdRR(_state.HL.low, _state.DE.low);
      break;
    case 0x6C:
      LdRR(_state.HL.low, _state.HL.high);
      break;
    case 0x6D:
      LdRR(_state.HL.low, _state.HL.low);
      break;
    case 0x6E:
      LdRIHl(_state.HL.low);
      break;
    case 0x6F:
      LdRR(_state.HL.low, _state.AF.high);
      break;
    case 0x70:
      LdIHlR(_state.BC.high);
      break;
    case 0x71:
      LdIHlR(_state.BC.low);
      break;
    case 0x72:
      LdIHlR(_state.DE.high);
      break;
    case 0x73:
      LdIHlR(_state.DE.low);
      break;
    case 0x74:
      LdIHlR(_state.HL.high);
      break;
    case 0x75:
      LdIHlR(_state.HL.low);
      break;
    case 0x76:
      Halt();
      break;
    case 0x77:
      LdIHlR(_state.AF.high);
      break;
    case 0x78:
      LdRR(_state.AF.high, _state.BC.high);
      break;
    case 0x79:
      LdRR(_state.AF.high, _state.BC.low);
      break;
    case 0x7A:
      LdRR(_state.AF.high, _state.DE.high);
      break;
    case 0x7B:
      LdRR(_state.AF.high, _state.DE.low);
      break;
    case 0x7C:
      LdRR(_state.AF.high, _state.HL.high);
      break;
    case 0x7D:
      LdRR(_state.AF.high, _state.HL.low);
      break;
    case 0x7E:
      LdRIHl(_state.AF.high);
      break;
    case 0x7F:
      LdRR(_state.AF.high, _state.AF.high);
      break;
    case 0x80:
      AddR(_state.BC.high);
      break;
    case 0x81:
      AddR(_state.BC.low);
      break;
    case 0x82:
      AddR(_state.DE.high);
      break;
    case 0x83:
      AddR(_state.DE.low);
      break;
    case 0x84:
      AddR(_state.HL.high);
      break;
    case 0x85:
      AddR(_state.HL.low);
      break;
    case 0x86:
      AddAHl();
      break;
    case 0x87:
      AddR(_state.AF.high);
      break;
    case 0x88:
      AdcR(_state.BC.high);
      break;
    case 0x89:
      AdcR(_state.BC.low);
      break;
    case 0x8A:
      AdcR(_state.DE.high);
      break;
    case 0x8B:
      AdcR(_state.DE.low);
      break;
    case 0x8C:
      AdcR(_state.HL.high);
      break;
    case 0x8D:
      AdcR(_state.HL.low);
      break;
    case 0x8E:
      AdcIHl();
      break;
    case 0x8F:
      AdcR(_state.AF.high);
      break;
    case 0x90:
      SubR(_state.BC.high);
      break;
    case 0x91:
      SubR(_state.BC.low);
      break;
    case 0x92:
      SubR(_state.DE.high);
      break;
    case 0x93:
      SubR(_state.DE.low);
      break;
    case 0x94:
      SubR(_state.HL.high);
      break;
    case 0x95:
      SubR(_state.HL.low);
      break;
    case 0x96:
      SubIHl();
      break;
    case 0x97:
      SubR(_state.AF.high);
      break;
    case 0x98:
      SbcR(_state.BC.high);
      break;
    case 0x99:
      SbcR(_state.BC.low);
      break;
    case 0x9A:
      SbcR(_state.DE.high);
      break;
    case 0x9B:
      SbcR(_state.DE.low);
      break;
    case 0x9C:
      SbcR(_state.HL.high);
      break;
    case 0x9D:
      SbcR(_state.HL.low);
      break;
    case 0x9E:
      SbcIHl();
      break;
    case 0x9F:
      SbcR(_state.AF.high);
      break;
    case 0xA0:
      AndR(_state.BC.high);
      break;
    case 0xA1:
      AndR(_state.BC.low);
      break;
    case 0xA2:
      AndR(_state.DE.high);
      break;
    case 0xA3:
      AndR(_state.DE.low);
      break;
    case 0xA4:
      AndR(_state.HL.high);
      break;
    case 0xA5:
      AndR(_state.HL.low);
      break;
    case 0xA6:
      AndIHl();
      break;
    case 0xA7:
      AndR(_state.AF.high);
      break;
    case 0xA8:
      XorR(_state.BC.high);
      break;
    case 0xA9:
      XorR(_state.BC.low);
      break;
    case 0xAA:
      XorR(_state.DE.high);
      break;
    case 0xAB:
      XorR(_state.DE.low);
      break;
    case 0xAC:
      XorR(_state.HL.high);
      break;
    case 0xAD:
      XorR(_state.HL.low);
      break;
    case 0xAE:
      XorIHl();
      break;
    case 0xAF:
      XorR(_state.AF.high);
      break;
    case 0xB0:
      OrR(_state.BC.high);
      break;
    case 0xB1:
      OrR(_state.BC.low);
      break;
    case 0xB2:
      OrR(_state.DE.high);
      break;
    case 0xB3:
      OrR(_state.DE.low);
      break;
    case 0xB4:
      OrR(_state.HL.high);
      break;
    case 0xB5:
      OrR(_state.HL.low);
      break;
    case 0xB6:
      OrIHl();
      break;
    case 0xB7:
      OrR(_state.AF.high);
      break;
    case 0xB8:
      CpR(_state.BC.high);
      break;
    case 0xB9:
      CpR(_state.BC.low);
      break;
    case 0xBA:
      CpR(_state.DE.high);
      break;
    case 0xBB:
      CpR(_state.DE.low);
      break;
    case 0xBC:
      CpR(_state.HL.high);
      break;
    case 0xBD:
      CpR(_state.HL.low);
      break;
    case 0xBE:
      CpIHl();
      break;
    case 0xBF:
      CpR(_state.AF.high);
      break;
    case 0xC0:
      RetCc(!GetZ());
      break;
    case 0xC1:
      PopRr(_state.BC);
      break;
    case 0xC2:
      JpCcU16(!GetZ());
      break;
    case 0xC3:
      JpCcU16(true);  // unconditional jump
      break;
    case 0xC4:
      CAllCcU16(!GetZ());
      break;
    case 0xC5:
      PushRr(_state.BC);
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
      RetCc(true);  // unconditional return
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
      CAllCcU16(true);  // unconditional call
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
      PopRr(_state.DE);
      break;
    case 0xD2:
      JpCcU16(!GetCY());
      break;
    case 0xD4:
      CAllCcU16(!GetCY());
      break;
    case 0xD5:
      PushRr(_state.DE);
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
      PopRr(_state.HL);
      break;
    case 0xE2:
      LdhCA();
      break;
    case 0xE5:
      PushRr(_state.HL);
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
      PopRr(_state.AF);
      _state.AF.low &= 0xF0U;  // clear unused lower nibble
    }
    break;
    case 0xF2:
      LdAC();
      break;
    case 0xF3:
      Di();
      break;
    case 0xF5:
      PushRr(_state.AF);
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
          _state.PC.reg - 1, opcode));
  }
}

void Cpu::TickExtended()
{
  auto opcode = _mmu.Read(_state.PC.reg++);

  switch (opcode)
  {
    case 0x00:
      Rlc(_state.BC.high);
      break;
    case 0x01:
      Rlc(_state.BC.low);
      break;
    case 0x02:
      Rlc(_state.DE.high);
      break;
    case 0x03:
      Rlc(_state.DE.low);
      break;
    case 0x04:
      Rlc(_state.HL.high);
      break;
    case 0x05:
      Rlc(_state.HL.low);
      break;
    case 0x06:
      Rlc(_mmu.Address(_state.HL.reg));
      break;
    case 0x07:
      Rlc(_state.AF.high);
      break;
    case 0x08:
      Rrc(_state.BC.high);
      break;
    case 0x09:
      Rrc(_state.BC.low);
      break;
    case 0x0A:
      Rrc(_state.DE.high);
      break;
    case 0x0B:
      Rrc(_state.DE.low);
      break;
    case 0x0C:
      Rrc(_state.HL.high);
      break;
    case 0x0D:
      Rrc(_state.HL.low);
      break;
    case 0x0E:
      Rrc(_mmu.Address(_state.HL.reg));
      break;
    case 0x0F:
      Rrc(_state.AF.high);
      break;
    case 0x10:
      Rl(_state.BC.high);
      break;
    case 0x11:
      Rl(_state.BC.low);
      break;
    case 0x12:
      Rl(_state.DE.high);
      break;
    case 0x13:
      Rl(_state.DE.low);
      break;
    case 0x14:
      Rl(_state.HL.high);
      break;
    case 0x15:
      Rl(_state.HL.low);
      break;
    case 0x16:
      Rl(_mmu.Address(_state.HL.reg));
      break;
    case 0x17:
      Rl(_state.AF.high);
      break;
    case 0x18:
      Rr(_state.BC.high);
      break;
    case 0x19:
      Rr(_state.BC.low);
      break;
    case 0x1A:
      Rr(_state.DE.high);
      break;
    case 0x1B:
      Rr(_state.DE.low);
      break;
    case 0x1C:
      Rr(_state.HL.high);
      break;
    case 0x1D:
      Rr(_state.HL.low);
      break;
    case 0x1E:
      Rr(_mmu.Address(_state.HL.reg));
      break;
    case 0x1F:
      Rr(_state.AF.high);
      break;
    case 0x20:
      Sla(_state.BC.high);
      break;
    case 0x21:
      Sla(_state.BC.low);
      break;
    case 0x22:
      Sla(_state.DE.high);
      break;
    case 0x23:
      Sla(_state.DE.low);
      break;
    case 0x24:
      Sla(_state.HL.high);
      break;
    case 0x25:
      Sla(_state.HL.low);
      break;
    case 0x26:
      Sla(_mmu.Address(_state.HL.reg));
      break;
    case 0x27:
      Sla(_state.AF.high);
      break;
    case 0x28:
      Sra(_state.BC.high);
      break;
    case 0x29:
      Sra(_state.BC.low);
      break;
    case 0x2A:
      Sra(_state.DE.high);
      break;
    case 0x2B:
      Sra(_state.DE.low);
      break;
    case 0x2C:
      Sra(_state.HL.high);
      break;
    case 0x2D:
      Sra(_state.HL.low);
      break;
    case 0x2E:
      Sra(_mmu.Address(_state.HL.reg));
      break;
    case 0x2F:
      Sra(_state.AF.high);
      break;
    case 0x30:
      Swap(_state.BC.high);
      break;
    case 0x31:
      Swap(_state.BC.low);
      break;
    case 0x32:
      Swap(_state.DE.high);
      break;
    case 0x33:
      Swap(_state.DE.low);
      break;
    case 0x34:
      Swap(_state.HL.high);
      break;
    case 0x35:
      Swap(_state.HL.low);
      break;
    case 0x36:
      Swap(_mmu.Address(_state.HL.reg));
      break;
    case 0x37:
      Swap(_state.AF.high);
      break;
    case 0x38:
      Srl(_state.BC.high);
      break;
    case 0x39:
      Srl(_state.BC.low);
      break;
    case 0x3A:
      Srl(_state.DE.high);
      break;
    case 0x3B:
      Srl(_state.DE.low);
      break;
    case 0x3C:
      Srl(_state.HL.high);
      break;
    case 0x3D:
      Srl(_state.HL.low);
      break;
    case 0x3E:
      Srl(_mmu.Address(_state.HL.reg));
      break;
    case 0x3F:
      Srl(_state.AF.high);
      break;
    case 0x40:
      Bit(_state.BC.high, 0);
      break;
    case 0x41:
      Bit(_state.BC.low, 0);
      break;
    case 0x42:
      Bit(_state.DE.high, 0);
      break;
    case 0x43:
      Bit(_state.DE.low, 0);
      break;
    case 0x44:
      Bit(_state.HL.high, 0);
      break;
    case 0x45:
      Bit(_state.HL.low, 0);
      break;
    case 0x46:
      Bit(_mmu.Read(_state.HL.reg), 0);
      break;
    case 0x47:
      Bit(_state.AF.high, 0);
      break;
    case 0x48:
      Bit(_state.BC.high, 1);
      break;
    case 0x49:
      Bit(_state.BC.low, 1);
      break;
    case 0x4A:
      Bit(_state.DE.high, 1);
      break;
    case 0x4B:
      Bit(_state.DE.low, 1);
      break;
    case 0x4C:
      Bit(_state.HL.high, 1);
      break;
    case 0x4D:
      Bit(_state.HL.low, 1);
      break;
    case 0x4E:
      Bit(_mmu.Read(_state.HL.reg), 1);
      break;
    case 0x4F:
      Bit(_state.AF.high, 1);
      break;
    case 0x50:
      Bit(_state.BC.high, 2);
      break;
    case 0x51:
      Bit(_state.BC.low, 2);
      break;
    case 0x52:
      Bit(_state.DE.high, 2);
      break;
    case 0x53:
      Bit(_state.DE.low, 2);
      break;
    case 0x54:
      Bit(_state.HL.high, 2);
      break;
    case 0x55:
      Bit(_state.HL.low, 2);
      break;
    case 0x56:
      Bit(_mmu.Read(_state.HL.reg), 2);
      break;
    case 0x57:
      Bit(_state.AF.high, 2);
      break;
    case 0x58:
      Bit(_state.BC.high, 3);
      break;
    case 0x59:
      Bit(_state.BC.low, 3);
      break;
    case 0x5A:
      Bit(_state.DE.high, 3);
      break;
    case 0x5B:
      Bit(_state.DE.low, 3);
      break;
    case 0x5C:
      Bit(_state.HL.high, 3);
      break;
    case 0x5D:
      Bit(_state.HL.low, 3);
      break;
    case 0x5E:
      Bit(_mmu.Read(_state.HL.reg), 3);
      break;
    case 0x5F:
      Bit(_state.AF.high, 3);
      break;
    case 0x60:
      Bit(_state.BC.high, 4);
      break;
    case 0x61:
      Bit(_state.BC.low, 4);
      break;
    case 0x62:
      Bit(_state.DE.high, 4);
      break;
    case 0x63:
      Bit(_state.DE.low, 4);
      break;
    case 0x64:
      Bit(_state.HL.high, 4);
      break;
    case 0x65:
      Bit(_state.HL.low, 4);
      break;
    case 0x66:
      Bit(_mmu.Read(_state.HL.reg), 4);
      break;
    case 0x67:
      Bit(_state.AF.high, 4);
      break;
    case 0x68:
      Bit(_state.BC.high, 5);
      break;
    case 0x69:
      Bit(_state.BC.low, 5);
      break;
    case 0x6A:
      Bit(_state.DE.high, 5);
      break;
    case 0x6B:
      Bit(_state.DE.low, 5);
      break;
    case 0x6C:
      Bit(_state.HL.high, 5);
      break;
    case 0x6D:
      Bit(_state.HL.low, 5);
      break;
    case 0x6E:
      Bit(_mmu.Read(_state.HL.reg), 5);
      break;
    case 0x6F:
      Bit(_state.AF.high, 5);
      break;
    case 0x70:
      Bit(_state.BC.high, 6);
      break;
    case 0x71:
      Bit(_state.BC.low, 6);
      break;
    case 0x72:
      Bit(_state.DE.high, 6);
      break;
    case 0x73:
      Bit(_state.DE.low, 6);
      break;
    case 0x74:
      Bit(_state.HL.high, 6);
      break;
    case 0x75:
      Bit(_state.HL.low, 6);
      break;
    case 0x76:
      Bit(_mmu.Read(_state.HL.reg), 6);
      break;
    case 0x77:
      Bit(_state.AF.high, 6);
      break;
    case 0x78:
      Bit(_state.BC.high, 7);
      break;
    case 0x79:
      Bit(_state.BC.low, 7);
      break;
    case 0x7A:
      Bit(_state.DE.high, 7);
      break;
    case 0x7B:
      Bit(_state.DE.low, 7);
      break;
    case 0x7C:
      Bit(_state.HL.high, 7);
      break;
    case 0x7D:
      Bit(_state.HL.low, 7);
      break;
    case 0x7E:
      Bit(_mmu.Read(_state.HL.reg), 7);
      break;
    case 0x7F:
      Bit(_state.AF.high, 7);
      break;
    case 0x80:
      Res(_state.BC.high, 0);
      break;
    case 0x81:
      Res(_state.BC.low, 0);
      break;
    case 0x82:
      Res(_state.DE.high, 0);
      break;
    case 0x83:
      Res(_state.DE.low, 0);
      break;
    case 0x84:
      Res(_state.HL.high, 0);
      break;
    case 0x85:
      Res(_state.HL.low, 0);
      break;
    case 0x86:
      Res(_mmu.Address(_state.HL.reg), 0);
      break;
    case 0x87:
      Res(_state.AF.high, 0);
      break;
    case 0x88:
      Res(_state.BC.high, 1);
      break;
    case 0x89:
      Res(_state.BC.low, 1);
      break;
    case 0x8A:
      Res(_state.DE.high, 1);
      break;
    case 0x8B:
      Res(_state.DE.low, 1);
      break;
    case 0x8C:
      Res(_state.HL.high, 1);
      break;
    case 0x8D:
      Res(_state.HL.low, 1);
      break;
    case 0x8E:
      Res(_mmu.Address(_state.HL.reg), 1);
      break;
    case 0x8F:
      Res(_state.AF.high, 1);
      break;
    case 0x90:
      Res(_state.BC.high, 2);
      break;
    case 0x91:
      Res(_state.BC.low, 2);
      break;
    case 0x92:
      Res(_state.DE.high, 2);
      break;
    case 0x93:
      Res(_state.DE.low, 2);
      break;
    case 0x94:
      Res(_state.HL.high, 2);
      break;
    case 0x95:
      Res(_state.HL.low, 2);
      break;
    case 0x96:
      Res(_mmu.Address(_state.HL.reg), 2);
      break;
    case 0x97:
      Res(_state.AF.high, 2);
      break;
    case 0x98:
      Res(_state.BC.high, 3);
      break;
    case 0x99:
      Res(_state.BC.low, 3);
      break;
    case 0x9A:
      Res(_state.DE.high, 3);
      break;
    case 0x9B:
      Res(_state.DE.low, 3);
      break;
    case 0x9C:
      Res(_state.HL.high, 3);
      break;
    case 0x9D:
      Res(_state.HL.low, 3);
      break;
    case 0x9E:
      Res(_mmu.Address(_state.HL.reg), 3);
      break;
    case 0x9F:
      Res(_state.AF.high, 3);
      break;
    case 0xA0:
      Res(_state.BC.high, 4);
      break;
    case 0xA1:
      Res(_state.BC.low, 4);
      break;
    case 0xA2:
      Res(_state.DE.high, 4);
      break;
    case 0xA3:
      Res(_state.DE.low, 4);
      break;
    case 0xA4:
      Res(_state.HL.high, 4);
      break;
    case 0xA5:
      Res(_state.HL.low, 4);
      break;
    case 0xA6:
      Res(_mmu.Address(_state.HL.reg), 4);
      break;
    case 0xA7:
      Res(_state.AF.high, 4);
      break;
    case 0xA8:
      Res(_state.BC.high, 5);
      break;
    case 0xA9:
      Res(_state.BC.low, 5);
      break;
    case 0xAA:
      Res(_state.DE.high, 5);
      break;
    case 0xAB:
      Res(_state.DE.low, 5);
      break;
    case 0xAC:
      Res(_state.HL.high, 5);
      break;
    case 0xAD:
      Res(_state.HL.low, 5);
      break;
    case 0xAE:
      Res(_mmu.Address(_state.HL.reg), 5);
      break;
    case 0xAF:
      Res(_state.AF.high, 5);
      break;
    case 0xB0:
      Res(_state.BC.high, 6);
      break;
    case 0xB1:
      Res(_state.BC.low, 6);
      break;
    case 0xB2:
      Res(_state.DE.high, 6);
      break;
    case 0xB3:
      Res(_state.DE.low, 6);
      break;
    case 0xB4:
      Res(_state.HL.high, 6);
      break;
    case 0xB5:
      Res(_state.HL.low, 6);
      break;
    case 0xB6:
      Res(_mmu.Address(_state.HL.reg), 6);
      break;
    case 0xB7:
      Res(_state.AF.high, 6);
      break;
    case 0xB8:
      Res(_state.BC.high, 7);
      break;
    case 0xB9:
      Res(_state.BC.low, 7);
      break;
    case 0xBA:
      Res(_state.DE.high, 7);
      break;
    case 0xBB:
      Res(_state.DE.low, 7);
      break;
    case 0xBC:
      Res(_state.HL.high, 7);
      break;
    case 0xBD:
      Res(_state.HL.low, 7);
      break;
    case 0xBE:
      Res(_mmu.Address(_state.HL.reg), 7);
      break;
    case 0xBF:
      Res(_state.AF.high, 7);
      break;
    case 0xC0:
      Set(_state.BC.high, 0);
      break;
    case 0xC1:
      Set(_state.BC.low, 0);
      break;
    case 0xC2:
      Set(_state.DE.high, 0);
      break;
    case 0xC3:
      Set(_state.DE.low, 0);
      break;
    case 0xC4:
      Set(_state.HL.high, 0);
      break;
    case 0xC5:
      Set(_state.HL.low, 0);
      break;
    case 0xC6:
      Set(_mmu.Address(_state.HL.reg), 0);
      break;
    case 0xC7:
      Set(_state.AF.high, 0);
      break;
    case 0xC8:
      Set(_state.BC.high, 1);
      break;
    case 0xC9:
      Set(_state.BC.low, 1);
      break;
    case 0xCA:
      Set(_state.DE.high, 1);
      break;
    case 0xCB:
      Set(_state.DE.low, 1);
      break;
    case 0xCC:
      Set(_state.HL.high, 1);
      break;
    case 0xCD:
      Set(_state.HL.low, 1);
      break;
    case 0xCE:
      Set(_mmu.Address(_state.HL.reg), 1);
      break;
    case 0xCF:
      Set(_state.AF.high, 1);
      break;
    case 0xD0:
      Set(_state.BC.high, 2);
      break;
    case 0xD1:
      Set(_state.BC.low, 2);
      break;
    case 0xD2:
      Set(_state.DE.high, 2);
      break;
    case 0xD3:
      Set(_state.DE.low, 2);
      break;
    case 0xD4:
      Set(_state.HL.high, 2);
      break;
    case 0xD5:
      Set(_state.HL.low, 2);
      break;
    case 0xD6:
      Set(_mmu.Address(_state.HL.reg), 2);
      break;
    case 0xD7:
      Set(_state.AF.high, 2);
      break;
    case 0xD8:
      Set(_state.BC.high, 3);
      break;
    case 0xD9:
      Set(_state.BC.low, 3);
      break;
    case 0xDA:
      Set(_state.DE.high, 3);
      break;
    case 0xDB:
      Set(_state.DE.low, 3);
      break;
    case 0xDC:
      Set(_state.HL.high, 3);
      break;
    case 0xDD:
      Set(_state.HL.low, 3);
      break;
    case 0xDE:
      Set(_mmu.Address(_state.HL.reg), 3);
      break;
    case 0xDF:
      Set(_state.AF.high, 3);
      break;
    case 0xE0:
      Set(_state.BC.high, 4);
      break;
    case 0xE1:
      Set(_state.BC.low, 4);
      break;
    case 0xE2:
      Set(_state.DE.high, 4);
      break;
    case 0xE3:
      Set(_state.DE.low, 4);
      break;
    case 0xE4:
      Set(_state.HL.high, 4);
      break;
    case 0xE5:
      Set(_state.HL.low, 4);
      break;
    case 0xE6:
      Set(_mmu.Address(_state.HL.reg), 4);
      break;
    case 0xE7:
      Set(_state.AF.high, 4);
      break;
    case 0xE8:
      Set(_state.BC.high, 5);
      break;
    case 0xE9:
      Set(_state.BC.low, 5);
      break;
    case 0xEA:
      Set(_state.DE.high, 5);
      break;
    case 0xEB:
      Set(_state.DE.low, 5);
      break;
    case 0xEC:
      Set(_state.HL.high, 5);
      break;
    case 0xED:
      Set(_state.HL.low, 5);
      break;
    case 0xEE:
      Set(_mmu.Address(_state.HL.reg), 5);
      break;
    case 0xEF:
      Set(_state.AF.high, 5);
      break;
    case 0xF0:
      Set(_state.BC.high, 6);
      break;
    case 0xF1:
      Set(_state.BC.low, 6);
      break;
    case 0xF2:
      Set(_state.DE.high, 6);
      break;
    case 0xF3:
      Set(_state.DE.low, 6);
      break;
    case 0xF4:
      Set(_state.HL.high, 6);
      break;
    case 0xF5:
      Set(_state.HL.low, 6);
      break;
    case 0xF6:
      Set(_mmu.Address(_state.HL.reg), 6);
      break;
    case 0xF7:
      Set(_state.AF.high, 6);
      break;
    case 0xF8:
      Set(_state.BC.high, 7);
      break;
    case 0xF9:
      Set(_state.BC.low, 7);
      break;
    case 0xFA:
      Set(_state.DE.high, 7);
      break;
    case 0xFB:
      Set(_state.DE.low, 7);
      break;
    case 0xFC:
      Set(_state.HL.high, 7);
      break;
    case 0xFD:
      Set(_state.HL.low, 7);
      break;
    case 0xFE:
      Set(_mmu.Address(_state.HL.reg), 7);
      break;
    case 0xFF:
      Set(_state.AF.high, 7);
      break;
    default:
      throw std::runtime_error(std::format(
          "[CB] PC: {:#06X}, failed to execute instruction \033[31m{:#04X}\033[0m",
          _state.PC.reg - 1, opcode));
  }
}

void Cpu::HandleInterruptsIfAny()
{
  // Check if interrupts are enabled
  if (_state._interrupt->_ime)
  {
    // Check if any interrupts are enabled and requested
    if ((_state._interrupt->_ie & _state._interrupt->_if) != 0)
    {
      // Check if VBlank interrupt is enabled and requested
      if (BitUtils::Test<InterruptType::VBLANK>(_state._interrupt->_ie)
          && BitUtils::Test<InterruptType::VBLANK>(_state._interrupt->_if))
      {
        LOG_DEBUG(_logger, "VBlank interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::VBLANK);
      }
      // Check if LCD interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::LCD>(_state._interrupt->_ie)
               && BitUtils::Test<InterruptType::LCD>(_state._interrupt->_if))
      {
        LOG_DEBUG(_logger, "LCD interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::LCD);
      }
      // Check if Timer interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::TIMER>(_state._interrupt->_ie)
               && BitUtils::Test<InterruptType::TIMER>(_state._interrupt->_if))
      {
        LOG_DEBUG(_logger, "Timer interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::TIMER);
      }
      // Check if Serial interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::SERIAL>(_state._interrupt->_ie)
               && BitUtils::Test<InterruptType::SERIAL>(_state._interrupt->_if))
      {
        LOG_DEBUG(_logger, "Serial interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::SERIAL);
      }
      // Check if Joypad interrupt is enabled and requested
      else if (BitUtils::Test<InterruptType::JOYPAD>(_state._interrupt->_ie)
               && BitUtils::Test<InterruptType::JOYPAD>(_state._interrupt->_if))
      {
        LOG_DEBUG(_logger, "Joypad interrupt is enabled and requested");
        DisableInterruptAndJumpToInterruptHandler(InterruptType::JOYPAD);
      }
    }
  }
}

// TODO: Check if I can Refactor this method
void Cpu::DisableInterruptAndJumpToInterruptHandler(InterruptType interruptType)
{
  LOG_DEBUG(_logger, "Disabling interrupt before jumping to interrupt handler");
  Di();
  LOG_DEBUG(_logger, "Saving PC on stack before jumping to interrupt handler");
  _state.SP.reg--;
  _mmu.Write(_state.SP.reg, _state.PC.high);
  _state.SP.reg--;
  _mmu.Write(_state.SP.reg, _state.PC.low);

  switch (interruptType)
  {
    case InterruptType::VBLANK:
      LOG_DEBUG(_logger,
          "Unset bit VBLANK (0) in Interrupt Flag register (IF: 0xFF0F)");
      BitUtils::Unset<InterruptType::VBLANK>(_state._interrupt->_if);
      LOG_DEBUG(_logger, "Jumping to VBLANK interrupt handler");
      _state.PC.reg = VBLANK_INTERRUPT_HANDLER_ADDRESS;
      break;
    case InterruptType::LCD:
      LOG_DEBUG(
          _logger, "Unset bit LCD (1) in Interrupt Flag register (IF: 0xFF0F)");
      BitUtils::Unset<InterruptType::LCD>(_state._interrupt->_if);
      LOG_DEBUG(_logger, "Jumping to LCD interrupt handler");
      _state.PC.reg = STAT_INTERRUPT_HANDLER_ADDRESS;
      break;
    case InterruptType::TIMER:
      LOG_DEBUG(_logger,
          "Unset bit TIMER (2) in Interrupt Flag register (IF: 0xFF0F)");
      BitUtils::Unset<InterruptType::TIMER>(_state._interrupt->_if);
      LOG_DEBUG(_logger, "Jumping to TIMER interrupt handler");
      _state.PC.reg = TIMER_INTERRUPT_HANDLER_ADDRESS;
      break;
    case InterruptType::SERIAL:
      LOG_DEBUG(_logger,
          "Unset bit SERIAL (3) in Interrupt Flag register (IF: 0xFF0F)");
      BitUtils::Unset<InterruptType::SERIAL>(_state._interrupt->_if);
      LOG_DEBUG(_logger, "Jumping to SERIAL interrupt handler");
      _state.PC.reg = SERIAL_INTERRUPT_HANDLER_ADDRESS;
      break;
    case InterruptType::JOYPAD:
      LOG_DEBUG(_logger,
          "Unset bit JOYPAD (4) in Interrupt Flag register (IF: 0xFF0F)");
      BitUtils::Unset<InterruptType::JOYPAD>(_state._interrupt->_if);
      LOG_DEBUG(_logger, "Jumping to JOYPAD interrupt handler");
      _state.PC.reg = JOYPAD_INTERRUPT_HANDLER_ADDRESS;
      break;
  }
}

// opcodes

void Cpu::DecHl()
{
  uint8_t data = _mmu.Read(_state.HL.reg);
  uint16_t res = data - 1;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((data & 0x0FU) - 1U) > 0x0F);

  _mmu.Write(_state.HL.reg, (res & 0xFFU));
}

void Cpu::LdAU16()
{
  uint8_t lsb = _mmu.Read(_state.PC.reg++);
  uint8_t msb = _mmu.Read(_state.PC.reg++);
  _state.AF.high = _mmu.Read(ToU16(lsb, msb));
}

void Cpu::LdSpHl()
{
  _state.SP.reg = _state.HL.reg;
}

void Cpu::LdHlS8()
{
  auto i8 = static_cast<int8_t>(_mmu.Read(_state.PC.reg++));
  uint16_t res = _state.SP.reg + static_cast<std::uint16_t>(i8);
  SetZ(false);
  SetN(false);
  SetH(((_state.SP.reg ^ i8 ^ res) & 0x10) != 0);
  SetCY(((_state.SP.reg ^ i8 ^ res) & 0x100) != 0);

  _state.HL.reg = res;
}

void Cpu::OrU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) | static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::LdAC()
{
  _state.AF.high = _mmu.Read(0xFF00 + _state.BC.low);
}

void Cpu::AndU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint8_t res = _state.AF.high & u8;
  SetZ(res == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  _state.AF.high = res;
}

void Cpu::AddSpS8()
{
  auto i8 = static_cast<int8_t>(_mmu.Read(_state.PC.reg++));
  uint16_t res = _state.SP.reg + static_cast<std::uint16_t>(i8);

  SetZ(false);
  SetN(false);
  SetH(((_state.SP.reg ^ i8 ^ res) & 0x10) != 0);  // Half-carry detection
  SetCY(((_state.SP.reg ^ i8 ^ res) & 0x100) != 0);

  _state.SP.reg = res;
}

void Cpu::JpHl()
{
  _state.PC.reg = _state.HL.reg;
}

void Cpu::XorU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint8_t res = _state.AF.high ^ u8;
  SetZ(res == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = res;
}

void Cpu::RetI()
{
  uint8_t lsb = _mmu.Read(_state.SP.reg++);
  uint8_t msb = _mmu.Read(_state.SP.reg++);
  _state.PC.reg = ToU16(lsb, msb);
  _state._interrupt->_ime = true;
}

void Cpu::SubU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::SbcU8()
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AddU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  AddR(u8);
}

void Cpu::AdcU8()
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(u8) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high & 0xFU) + (u8 & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AndR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) & static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AndIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) & static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::XorR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) ^ static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::XorIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) ^ static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::OrR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) | static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::OrIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) | static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::CpR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::CpIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::SubIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::SbcIHl()
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::SbcR(std::uint8_t reg)
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(reg) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (reg & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AdcIHl()
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(u8) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high & 0xFU) + (u8 & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AddR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high ^ reg ^ res) & 0x10) != 0);
  SetCY(((_state.AF.high ^ reg ^ res) & 0x100) != 0);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AdcR(std::uint8_t reg)
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(reg) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high & 0xFU) + (reg & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::Halt()
{
  throw std::runtime_error(
      std::format("Unimplemented instruction: 0x76 (HALT)"));
}

void Cpu::Ccf()
{
  SetN(false);
  SetH(false);

  _state.AF.low = _state.AF.low ^ (1U << 4U);
}

void Cpu::LdAHlN()
{
  _state.AF.high = _mmu.Read(_state.HL.reg);
  _state.HL.reg = _state.HL.reg - 1;
}

void Cpu::Scf()
{
  SetN(false);
  SetH(false);
  SetCY(true);
}

void Cpu::IncIHl()
{
  uint8_t data = _mmu.Read(_state.HL.reg);
  uint16_t res = data + 1;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((data & 0x0FU) + 1U) > 0x0F);

  _mmu.Write(_state.HL.reg, (res & 0xFFU));
}

void Cpu::Cpl()
{
  SetN(true);
  SetH(true);

  _state.AF.high = ~_state.AF.high;
}

// DAA
// (https://forums.nesdev.org/viewtopic.php?p=196282&sid=a1cdd6adc0b01ea3d77f61aee9527449#p196282)
void Cpu::Daa()
{
  if (!(_state.AF.low & (1U << 6U)))
  {
    if ((_state.AF.low & (1U << 4U)) || _state.AF.high > 0x99)
    {
      _state.AF.high += 0x60;
      _state.AF.low = ((_state.AF.low & ~(1U << 4U)) | (1U << 4U));
    }
    if ((_state.AF.low & (1U << 5U)) || (_state.AF.high & 0x0FU) > 0x09)
    {
      _state.AF.high += 0x06;
    }
  }
  else
  {
    if (_state.AF.low & (1U << 4U))
    {
      _state.AF.high -= 0x60;
    }
    if (_state.AF.low & (1U << 5U))
    {
      _state.AF.high -= 0x06;
    }
  }

  SetZ(_state.AF.high == 0);
  SetH(false);
}

void Cpu::Rra()
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY((_state.AF.high & (1U << 0U)));

  _state.AF.high = _state.AF.high >> 1U;
  _state.AF.high =
      (_state.AF.high & ~(1U << 7U)) | static_cast<uint8_t>(oldCY << 7U);
}

void Cpu::Stop()
{
  // TODO: check what needs to be done here
  LOG_WARN(_logger, std::format("Unimplemented instruction: 0x10 (STOP)"));
}

void Cpu::Rrca()
{
  uint8_t b0 = (_state.AF.high & (1U << 0U));

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY(b0);

  _state.AF.high = _state.AF.high >> 1U;
  _state.AF.high =
      (_state.AF.high & ~(1U << 7U)) | static_cast<uint8_t>(b0 << 7U);
}

void Cpu::LdDU16Sp()
{
  uint8_t lsb = _mmu.Read(_state.PC.reg++);
  uint8_t msb = _mmu.Read(_state.PC.reg++);
  uint16_t nn = ToU16(lsb, msb);
  _mmu.Write(nn, _state.SP.low);
  ++nn;
  _mmu.Write(nn, _state.SP.high);
}

void Cpu::Rlca()
{
  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY((_state.AF.high & (1U << 7U)) >> 7U);

  _state.AF.high = _state.AF.high << 1U;
  _state.AF.high =
      ((_state.AF.high & ~(1U << 0U)) | ((_state.AF.low & (1U << 4U)) >> 4U));
}

void Cpu::LdIRrA(Register& reg)
{
  _mmu.Write(reg.reg, _state.AF.high);
}

void Cpu::AddHlRr(Register& reg)
{
  uint32_t res =
      static_cast<uint32_t>(_state.HL.reg) + static_cast<uint32_t>(reg.reg);

  SetN(false);
  SetH(((_state.HL.reg & 0xFFFU) + (reg.reg & 0xFFFU)) > 0xFFFU);
  SetCY(res > 0xFFFFU);

  _state.HL.reg = res & 0xFFFFU;
}

void Cpu::RstU8(std::uint8_t addr)
{
  _state.SP.reg--;
  _mmu.Write(_state.SP.reg, _state.PC.high);
  _state.SP.reg--;
  _mmu.Write(_state.SP.reg, _state.PC.low);
  _state.PC.reg = ToU16(addr, 0x00);
}

void Cpu::LdRIHl(std::uint8_t& reg)
{
  reg = _mmu.Read(_state.HL.reg);
}

void Cpu::Ei()
{
  _state._interrupt->_enableRequested = true;
  LOG_DEBUG(_logger, "Enable interrupt requested");
}

void Cpu::DecRr(Register& reg)
{
  reg.reg--;
}

void Cpu::LdAHlP()
{
  _state.AF.high = _mmu.Read(_state.HL.reg);
  _state.HL.reg++;
}

void Cpu::LdHlU8()
{
  auto data = _mmu.Read(_state.PC.reg++);
  _mmu.Write(_state.HL.reg, data);
}

void Cpu::Di()
{
  _state._interrupt->_enableRequested = false;
  _state._interrupt->_ime = false;
  LOG_DEBUG(_logger, "Interrupt disabled");
}

void Cpu::JpCcU16(bool cc)
{
  auto lsb = _mmu.Read(_state.PC.reg++);
  auto msb = _mmu.Read(_state.PC.reg++);
  if (cc)
  {
    _state.PC.reg = ToU16(lsb, msb);
  }
}

void Cpu::Nop()
{
}

void Cpu::SubR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::AddAHl()
{
  auto data = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(data);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high ^ data ^ res) & 0x10) != 0);
  SetCY(((_state.AF.high ^ data ^ res) & 0x100) != 0);

  _state.AF.high = (res & 0xFFU);
}

void Cpu::LdU16A()
{
  auto low = _mmu.Read(_state.PC.reg++);
  auto high = _mmu.Read(_state.PC.reg++);
  _mmu.Write(ToU16(low, high), _state.AF.high);
}
void Cpu::CpU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::CpAHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
}

void Cpu::LdHlPA()
{
  _mmu.Write(_state.HL.reg, _state.AF.high);
  ++_state.HL.reg;
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
  reg.low = _mmu.Read(_state.SP.reg);
  ++_state.SP.reg;
  reg.high = _mmu.Read(_state.SP.reg);
  ++_state.SP.reg;
}

void Cpu::RlA()
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY(((_state.AF.high & (1U << 7U)) >> 7U) == 1U);

  _state.AF.high = static_cast<uint8_t>(_state.AF.high << 1U);
  _state.AF.high = static_cast<uint8_t>((_state.AF.high & ~(1U << 0U)) | oldCY);
}

void Cpu::PushRr(Register& reg)
{
  --_state.SP.reg;
  _mmu.Write(_state.SP.reg, reg.high);
  --_state.SP.reg;
  _mmu.Write(_state.SP.reg, reg.low);
}

void Cpu::LdRR(std::uint8_t& reg1, std::uint8_t& reg2)
{
  reg1 = reg2;
}

void Cpu::CAllCcU16(bool cc)
{
  auto low = _mmu.Read(_state.PC.reg++);
  auto high = _mmu.Read(_state.PC.reg++);

  if (cc)
  {
    --_state.SP.reg;
    _mmu.Write(_state.SP.reg, _state.PC.high);
    --_state.SP.reg;
    _mmu.Write(_state.SP.reg, _state.PC.low);

    _state.PC.low = low;
    _state.PC.high = high;
  }
}

void Cpu::RetCc(bool cc)
{
  if (cc)
  {
    auto low = _mmu.Read(_state.SP.reg);
    ++_state.SP.reg;
    auto high = _mmu.Read(_state.SP.reg);
    ++_state.SP.reg;
    _state.PC.reg = ToU16(low, high);
  }
}

void Cpu::LdRrU16(Register& reg)
{
  reg.low = _mmu.Read(_state.PC.reg++);
  reg.high = _mmu.Read(_state.PC.reg++);
}

void Cpu::LdRU8(std::uint8_t& reg)
{
  reg = _mmu.Read(_state.PC.reg++);
}

void Cpu::LdAIRr(Register& reg)
{
  _state.AF.high = _mmu.Read(reg.reg);
}

void Cpu::LdHlMA()
{
  _mmu.Write(_state.HL.reg, _state.AF.high);
  --_state.HL.reg;
}

void Cpu::LdIHlR(std::uint8_t& reg)
{
  _mmu.Write(_state.HL.reg, reg);
}

void Cpu::JrCCI8(bool cc)
{
  auto i8 = static_cast<std::int8_t>(_mmu.Read(_state.PC.reg++));
  if (cc)
  {
    _state.PC.reg = static_cast<std::uint16_t>(_state.PC.reg + i8);
  }
}

void Cpu::LdhAU8()
{
  auto low = _mmu.Read(_state.PC.reg++);
  auto addr = ToU16(low, 0xFF);
  _state.AF.high = _mmu.Read(addr);
}

void Cpu::LdhCA()
{
  auto addr = ToU16(_state.BC.low, 0xFF);
  _mmu.Write(addr, _state.AF.high);
}

void Cpu::LdhU8A()
{
  auto addr = ToU16(_mmu.Read(_state.PC.reg++), 0xFF);
  _mmu.Write(addr, _state.AF.high);
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
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;
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
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;
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
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;
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
  _state.AF.low = (_state.AF.low & ~(1U << 7U)) | ((!bitX) << 7U);
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

// utility

// Set zero flag
void Cpu::SetZ(bool value)
{
  _state.AF.low = static_cast<std::uint8_t>(
      (_state.AF.low & ~(1U << 7U)) | static_cast<uint8_t>(value << 7U));
}

// Get value of zero flag
[[nodiscard]]
bool Cpu::GetZ() const
{
  return ((_state.AF.low & (1U << 7U)) >> 7U) == 1;
}

// Set negative flag
void Cpu::SetN(bool value)
{
  _state.AF.low = static_cast<std::uint8_t>(
      (_state.AF.low & ~(1U << 6U)) | static_cast<uint8_t>(value << 6U));
}

// Get value of negative flag
[[nodiscard]]
bool Cpu::GetN() const
{
  return ((_state.AF.low & (1U << 6U)) >> 6U) == 1;
}

// Set half carry flag
void Cpu::SetH(bool value)
{
  _state.AF.low = static_cast<std::uint8_t>(
      (_state.AF.low & ~(1U << 5U)) | static_cast<uint8_t>(value << 5U));
}

// Get value of half carry flag
[[nodiscard]]
bool Cpu::GetH() const
{
  return ((_state.AF.low & (1U << 5U)) >> 5U) == 1;
}

// Set carry flag
void Cpu::SetCY(bool value)
{
  _state.AF.low = static_cast<std::uint8_t>(
      (_state.AF.low & ~(1U << 4U)) | static_cast<uint8_t>(value << 4U));
}

// Get value of carry flag
[[nodiscard]]
bool Cpu::GetCY() const
{
  return ((_state.AF.low & (1U << 4U)) >> 4U) == 1;
}

std::uint16_t Cpu::ToU16(std::uint8_t lsb, std::uint8_t msb)
{
  return static_cast<std::uint16_t>(msb << 8U) | lsb;
}
// NOLINTEND(readability-suspicious-call-argument, hicpp-signed-bitwise,
// readability-convert-member-functions-to-static)
