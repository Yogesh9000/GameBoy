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

Cpu::Cpu(MemoryManagementUnit &mmu) : _state{}, _mmu(mmu)
{
  _state._interrupt = std::make_shared<Interrupt>();
  _mmu.AddMemoryRange(_state._interrupt);
  _logger = LogManager::GetLogger("Cpu");
}

Cpu::Cpu(CpuState state, MemoryManagementUnit &mmu)
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
int Cpu::Tick()
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
      return Nop();
    case 0x01:
      return LdRrU16(_state.BC);
    case 0x02:
      return LdIRrA(_state.BC);
    case 0x03:
      return IncRr(_state.BC);
    case 0x04:
      return IncR(_state.BC.high);
    case 0x05:
      return DecR(_state.BC.high);
    case 0x06:
      return LdRU8(_state.BC.high);
    case 0x07:
      return Rlca();
    case 0x08:
      return LdDU16Sp();
    case 0x09:
      return AddHlRr(_state.BC);
    case 0x0A:
      return LdAIRr(_state.BC);
    case 0x0B:
      return DecRr(_state.BC);
    case 0x0C:
      return IncR(_state.BC.low);
    case 0x0D:
      return DecR(_state.BC.low);
    case 0x0E:
      return LdRU8(_state.BC.low);
    case 0x0F:
      return Rrca();
    case 0x10:
      return Stop();
    case 0x11:
      return LdRrU16(_state.DE);
    case 0x12:
      return LdIRrA(_state.DE);
    case 0x13:
      return IncRr(_state.DE);
    case 0x14:
      return IncR(_state.DE.high);
    case 0x15:
      return DecR(_state.DE.high);
    case 0x16:
      return LdRU8(_state.DE.high);
    case 0x17:
      return RlA();
    case 0x18:
      return JrCCI8(true);
    case 0x19:
      return AddHlRr(_state.DE);
    case 0x1A:
      return LdAIRr(_state.DE);
    case 0x1B:
      return DecRr(_state.DE);
    case 0x1C:
      return IncR(_state.DE.low);
    case 0x1D:
      return DecR(_state.DE.low);
    case 0x1E:
      return LdRU8(_state.DE.low);
    case 0x1F:
      return Rra();
    case 0x20:
      return JrCCI8(!GetZ());
    case 0x21:
      return LdRrU16(_state.HL);
    case 0x22:
      return LdHlPA();
    case 0x23:
      return IncRr(_state.HL);
    case 0x24:
      return IncR(_state.HL.high);
    case 0x25:
      return DecR(_state.HL.high);
    case 0x26:
      return LdRU8(_state.HL.high);
    case 0x27:
      return Daa();
    case 0x28:
      return JrCCI8(GetZ());
    case 0x29:
      return AddHlRr(_state.HL);
    case 0x2A:
      return LdAHlP();
    case 0x2B:
      return DecRr(_state.HL);
    case 0x2C:
      return IncR(_state.HL.low);
    case 0x2D:
      return DecR(_state.HL.low);
    case 0x2E:
      return LdRU8(_state.HL.low);
    case 0x2F:
      return Cpl();
    case 0x30:
      return JrCCI8(!GetCY());
    case 0x31:
      return LdRrU16(_state.SP);
    case 0x32:
      return LdHlMA();
    case 0x33:
      return IncRr(_state.SP);
    case 0x34:
      return IncIHl();
    case 0x35:
      return DecHl();
    case 0x36:
      return LdHlU8();
    case 0x37:
      return Scf();
    case 0x38:
      return JrCCI8(GetCY());
    case 0x39:
      return AddHlRr(_state.SP);
    case 0x3A:
      return LdAHlN();
    case 0x3B:
      return DecRr(_state.SP);
    case 0x3C:
      return IncR(_state.AF.high);
    case 0x3D:
      return DecR(_state.AF.high);
    case 0x3E:
      return LdRU8(_state.AF.high);
    case 0x3F:
      return Ccf();
    case 0x40:
      return LdRR(_state.BC.high, _state.BC.high);
    case 0x41:
      return LdRR(_state.BC.high, _state.BC.low);
    case 0x42:
      return LdRR(_state.BC.high, _state.DE.high);
    case 0x43:
      return LdRR(_state.BC.high, _state.DE.low);
    case 0x44:
      return LdRR(_state.BC.high, _state.HL.high);
    case 0x45:
      return LdRR(_state.BC.high, _state.HL.low);
    case 0x46:
      return LdRIHl(_state.BC.high);
    case 0x47:
      return LdRR(_state.BC.high, _state.AF.high);
    case 0x48:
      return LdRR(_state.BC.low, _state.BC.high);
    case 0x49:
      return LdRR(_state.BC.low, _state.BC.low);
    case 0x4A:
      return LdRR(_state.BC.low, _state.DE.high);
    case 0x4B:
      return LdRR(_state.BC.low, _state.DE.low);
    case 0x4C:
      return LdRR(_state.BC.low, _state.HL.high);
    case 0x4D:
      return LdRR(_state.BC.low, _state.HL.low);
    case 0x4E:
      return LdRIHl(_state.BC.low);
    case 0x4F:
      return LdRR(_state.BC.low, _state.AF.high);
    case 0x50:
      return LdRR(_state.DE.high, _state.BC.high);
    case 0x51:
      return LdRR(_state.DE.high, _state.BC.low);
    case 0x52:
      return LdRR(_state.DE.high, _state.DE.high);
    case 0x53:
      return LdRR(_state.DE.high, _state.DE.low);
    case 0x54:
      return LdRR(_state.DE.high, _state.HL.high);
    case 0x55:
      return LdRR(_state.DE.high, _state.HL.low);
    case 0x56:
      return LdRIHl(_state.DE.high);
    case 0x57:
      return LdRR(_state.DE.high, _state.AF.high);
    case 0x58:
      return LdRR(_state.DE.low, _state.BC.high);
    case 0x59:
      return LdRR(_state.DE.low, _state.BC.low);
    case 0x5A:
      return LdRR(_state.DE.low, _state.DE.high);
    case 0x5B:
      return LdRR(_state.DE.low, _state.DE.low);
    case 0x5C:
      return LdRR(_state.DE.low, _state.HL.high);
    case 0x5D:
      return LdRR(_state.DE.low, _state.HL.low);
    case 0x5E:
      return LdRIHl(_state.DE.low);
    case 0x5F:
      return LdRR(_state.DE.low, _state.AF.high);
    case 0x60:
      return LdRR(_state.HL.high, _state.BC.high);
    case 0x61:
      return LdRR(_state.HL.high, _state.BC.low);
    case 0x62:
      return LdRR(_state.HL.high, _state.DE.high);
    case 0x63:
      return LdRR(_state.HL.high, _state.DE.low);
    case 0x64:
      return LdRR(_state.HL.high, _state.HL.high);
    case 0x65:
      return LdRR(_state.HL.high, _state.HL.low);
    case 0x66:
      return LdRIHl(_state.HL.high);
    case 0x67:
      return LdRR(_state.HL.high, _state.AF.high);
    case 0x68:
      return LdRR(_state.HL.low, _state.BC.high);
    case 0x69:
      return LdRR(_state.HL.low, _state.BC.low);
    case 0x6A:
      return LdRR(_state.HL.low, _state.DE.high);
    case 0x6B:
      return LdRR(_state.HL.low, _state.DE.low);
    case 0x6C:
      return LdRR(_state.HL.low, _state.HL.high);
    case 0x6D:
      return LdRR(_state.HL.low, _state.HL.low);
    case 0x6E:
      return LdRIHl(_state.HL.low);
    case 0x6F:
      return LdRR(_state.HL.low, _state.AF.high);
    case 0x70:
      return LdIHlR(_state.BC.high);
    case 0x71:
      return LdIHlR(_state.BC.low);
    case 0x72:
      return LdIHlR(_state.DE.high);
    case 0x73:
      return LdIHlR(_state.DE.low);
    case 0x74:
      return LdIHlR(_state.HL.high);
    case 0x75:
      return LdIHlR(_state.HL.low);
    case 0x76:
      return Halt();
    case 0x77:
      return LdIHlR(_state.AF.high);
    case 0x78:
      return LdRR(_state.AF.high, _state.BC.high);
    case 0x79:
      return LdRR(_state.AF.high, _state.BC.low);
    case 0x7A:
      return LdRR(_state.AF.high, _state.DE.high);
    case 0x7B:
      return LdRR(_state.AF.high, _state.DE.low);
    case 0x7C:
      return LdRR(_state.AF.high, _state.HL.high);
    case 0x7D:
      return LdRR(_state.AF.high, _state.HL.low);
    case 0x7E:
      return LdRIHl(_state.AF.high);
    case 0x7F:
      return LdRR(_state.AF.high, _state.AF.high);
    case 0x80:
      return AddR(_state.BC.high);
    case 0x81:
      return AddR(_state.BC.low);
    case 0x82:
      return AddR(_state.DE.high);
    case 0x83:
      return AddR(_state.DE.low);
    case 0x84:
      return AddR(_state.HL.high);
    case 0x85:
      return AddR(_state.HL.low);
    case 0x86:
      return AddAHl();
    case 0x87:
      return AddR(_state.AF.high);
    case 0x88:
      return AdcR(_state.BC.high);
    case 0x89:
      return AdcR(_state.BC.low);
    case 0x8A:
      return AdcR(_state.DE.high);
    case 0x8B:
      return AdcR(_state.DE.low);
    case 0x8C:
      return AdcR(_state.HL.high);
    case 0x8D:
      return AdcR(_state.HL.low);
    case 0x8E:
      return AdcIHl();
    case 0x8F:
      return AdcR(_state.AF.high);
    case 0x90:
      return SubR(_state.BC.high);
    case 0x91:
      return SubR(_state.BC.low);
    case 0x92:
      return SubR(_state.DE.high);
    case 0x93:
      return SubR(_state.DE.low);
    case 0x94:
      return SubR(_state.HL.high);
    case 0x95:
      return SubR(_state.HL.low);
    case 0x96:
      return SubIHl();
    case 0x97:
      return SubR(_state.AF.high);
    case 0x98:
      return SbcR(_state.BC.high);
    case 0x99:
      return SbcR(_state.BC.low);
    case 0x9A:
      return SbcR(_state.DE.high);
    case 0x9B:
      return SbcR(_state.DE.low);
    case 0x9C:
      return SbcR(_state.HL.high);
    case 0x9D:
      return SbcR(_state.HL.low);
    case 0x9E:
      return SbcIHl();
    case 0x9F:
      return SbcR(_state.AF.high);
    case 0xA0:
      return AndR(_state.BC.high);
    case 0xA1:
      return AndR(_state.BC.low);
    case 0xA2:
      return AndR(_state.DE.high);
    case 0xA3:
      return AndR(_state.DE.low);
    case 0xA4:
      return AndR(_state.HL.high);
    case 0xA5:
      return AndR(_state.HL.low);
    case 0xA6:
      return AndIHl();
    case 0xA7:
      return AndR(_state.AF.high);
    case 0xA8:
      return XorR(_state.BC.high);
    case 0xA9:
      return XorR(_state.BC.low);
    case 0xAA:
      return XorR(_state.DE.high);
    case 0xAB:
      return XorR(_state.DE.low);
    case 0xAC:
      return XorR(_state.HL.high);
    case 0xAD:
      return XorR(_state.HL.low);
    case 0xAE:
      return XorIHl();
    case 0xAF:
      return XorR(_state.AF.high);
    case 0xB0:
      return OrR(_state.BC.high);
    case 0xB1:
      return OrR(_state.BC.low);
    case 0xB2:
      return OrR(_state.DE.high);
    case 0xB3:
      return OrR(_state.DE.low);
    case 0xB4:
      return OrR(_state.HL.high);
    case 0xB5:
      return OrR(_state.HL.low);
    case 0xB6:
      return OrIHl();
    case 0xB7:
      return OrR(_state.AF.high);
    case 0xB8:
      return CpR(_state.BC.high);
    case 0xB9:
      return CpR(_state.BC.low);
    case 0xBA:
      return CpR(_state.DE.high);
    case 0xBB:
      return CpR(_state.DE.low);
    case 0xBC:
      return CpR(_state.HL.high);
    case 0xBD:
      return CpR(_state.HL.low);
    case 0xBE:
      return CpIHl();
    case 0xBF:
      return CpR(_state.AF.high);
    case 0xC0:
      return RetCc(!GetZ());
    case 0xC1:
      return PopRr(_state.BC);
    case 0xC2:
      return JpCcU16(!GetZ());
    case 0xC3:
      return JpCcU16(true);  // unconditional jump
    case 0xC4:
      return CAllCcU16(!GetZ());
    case 0xC5:
      return PushRr(_state.BC);
    case 0xC6:
      return AddU8();
    case 0xC7:
      return RstU8(0x00);
    case 0xC8:
      return RetCc(GetZ());
    case 0xC9:
    {
      RetCc(true);  // unconditional return
      return 16;
    }
    case 0xCA:
      return JpCcU16(GetZ());
    case 0xCB:
      return TickExtended();
    case 0xCC:
      return CAllCcU16(GetZ());
    case 0xCD:
      return CAllCcU16(true);  // unconditional call
    case 0xCE:
      return AdcU8();
    case 0xCF:
      return RstU8(0x08);
    case 0xD0:
      return RetCc(!GetCY());
    case 0xD1:
      return PopRr(_state.DE);
    case 0xD2:
      return JpCcU16(!GetCY());
    case 0xD4:
      return CAllCcU16(!GetCY());
    case 0xD5:
      return PushRr(_state.DE);
    case 0xD6:
      return SubU8();
    case 0xD7:
      return RstU8(0x10);
    case 0xD8:
      return RetCc(GetCY());
    case 0xD9:
      return RetI();
    case 0xDA:
      return JpCcU16(GetCY());
    case 0xDC:
      return CAllCcU16(GetCY());
    case 0xDE:
      return SbcU8();
    case 0xDF:
      return RstU8(0x18);
    case 0xE0:
      return LdhU8A();
    case 0xE1:
      return PopRr(_state.HL);
    case 0xE2:
      return LdhCA();
    case 0xE5:
      return PushRr(_state.HL);
    case 0xE6:
      return AndU8();
    case 0xE7:
      return RstU8(0x20);
    case 0xE8:
      return AddSpS8();
    case 0xE9:
      return JpHl();
    case 0xEA:
      return LdU16A();
    case 0xEE:
      return XorU8();
    case 0xEF:
      return RstU8(0x28);
    case 0xF0:
      return LdhAU8();
    case 0xF1:
    {
      int cycles = PopRr(_state.AF);
      _state.AF.low &= 0xF0U;  // clear unused lower nibble
      return cycles;
    }
    case 0xF2:
      return LdAC();
    case 0xF3:
      return Di();
    case 0xF5:
      return PushRr(_state.AF);
    case 0xF6:
      return OrU8();
    case 0xF7:
      return RstU8(0x30);
    case 0xF8:
      return LdHlS8();
    case 0xF9:
      return LdSpHl();
    case 0xFA:
      return LdAU16();
    case 0xFB:
      return Ei();
    case 0xFE:
      return CpU8();
    case 0xFF:
      return RstU8(0x38U);
    default:
      throw std::runtime_error(std::format(
          "PC: {:#06X}, failed to execute instruction \033[31m{:#04X}\033[0m",
          _state.PC.reg - 1, opcode));
  }
  return 0x00;
}

int Cpu::TickExtended()
{
  auto opcode = _mmu.Read(_state.PC.reg++);

  switch (opcode)
  {
    case 0x00:
      return Rlc(_state.BC.high);
    case 0x01:
      return Rlc(_state.BC.low);
    case 0x02:
      return Rlc(_state.DE.high);
    case 0x03:
      return Rlc(_state.DE.low);
    case 0x04:
      return Rlc(_state.HL.high);
    case 0x05:
      return Rlc(_state.HL.low);
    case 0x06:
      return 8 + Rlc(_mmu.Address(_state.HL.reg));
    case 0x07:
      return Rlc(_state.AF.high);
    case 0x08:
      return Rrc(_state.BC.high);
    case 0x09:
      return Rrc(_state.BC.low);
    case 0x0A:
      return Rrc(_state.DE.high);
    case 0x0B:
      return Rrc(_state.DE.low);
    case 0x0C:
      return Rrc(_state.HL.high);
    case 0x0D:
      return Rrc(_state.HL.low);
    case 0x0E:
      return 8 + Rrc(_mmu.Address(_state.HL.reg));
    case 0x0F:
      return Rrc(_state.AF.high);
    case 0x10:
      return Rl(_state.BC.high);
    case 0x11:
      return Rl(_state.BC.low);
    case 0x12:
      return Rl(_state.DE.high);
    case 0x13:
      return Rl(_state.DE.low);
    case 0x14:
      return Rl(_state.HL.high);
    case 0x15:
      return Rl(_state.HL.low);
    case 0x16:
      return 8 + Rl(_mmu.Address(_state.HL.reg));
    case 0x17:
      return Rl(_state.AF.high);
    case 0x18:
      return Rr(_state.BC.high);
    case 0x19:
      return Rr(_state.BC.low);
    case 0x1A:
      return Rr(_state.DE.high);
    case 0x1B:
      return Rr(_state.DE.low);
    case 0x1C:
      return Rr(_state.HL.high);
    case 0x1D:
      return Rr(_state.HL.low);
    case 0x1E:
      return 8 + Rr(_mmu.Address(_state.HL.reg));
    case 0x1F:
      return Rr(_state.AF.high);
    case 0x20:
      return Sla(_state.BC.high);
    case 0x21:
      return Sla(_state.BC.low);
    case 0x22:
      return Sla(_state.DE.high);
    case 0x23:
      return Sla(_state.DE.low);
    case 0x24:
      return Sla(_state.HL.high);
    case 0x25:
      return Sla(_state.HL.low);
    case 0x26:
      return 8 + Sla(_mmu.Address(_state.HL.reg));
    case 0x27:
      return Sla(_state.AF.high);
    case 0x28:
      return Sra(_state.BC.high);
    case 0x29:
      return Sra(_state.BC.low);
    case 0x2A:
      return Sra(_state.DE.high);
    case 0x2B:
      return Sra(_state.DE.low);
    case 0x2C:
      return Sra(_state.HL.high);
    case 0x2D:
      return Sra(_state.HL.low);
    case 0x2E:
      return 8 + Sra(_mmu.Address(_state.HL.reg));
    case 0x2F:
      return Sra(_state.AF.high);
    case 0x30:
      return Swap(_state.BC.high);
    case 0x31:
      return Swap(_state.BC.low);
    case 0x32:
      return Swap(_state.DE.high);
    case 0x33:
      return Swap(_state.DE.low);
    case 0x34:
      return Swap(_state.HL.high);
    case 0x35:
      return Swap(_state.HL.low);
    case 0x36:
      return 8 + Swap(_mmu.Address(_state.HL.reg));
    case 0x37:
      return Swap(_state.AF.high);
    case 0x38:
      return Srl(_state.BC.high);
    case 0x39:
      return Srl(_state.BC.low);
    case 0x3A:
      return Srl(_state.DE.high);
    case 0x3B:
      return Srl(_state.DE.low);
    case 0x3C:
      return Srl(_state.HL.high);
    case 0x3D:
      return Srl(_state.HL.low);
    case 0x3E:
      return 8 + Srl(_mmu.Address(_state.HL.reg));
    case 0x3F:
      return Srl(_state.AF.high);
    case 0x40:
      return Bit(_state.BC.high, 0);
    case 0x41:
      return Bit(_state.BC.low, 0);
    case 0x42:
      return Bit(_state.DE.high, 0);
    case 0x43:
      return Bit(_state.DE.low, 0);
    case 0x44:
      return Bit(_state.HL.high, 0);
    case 0x45:
      return Bit(_state.HL.low, 0);
    case 0x46:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 0);
    case 0x47:
      return Bit(_state.AF.high, 0);
    case 0x48:
      return Bit(_state.BC.high, 1);
    case 0x49:
      return Bit(_state.BC.low, 1);
    case 0x4A:
      return Bit(_state.DE.high, 1);
    case 0x4B:
      return Bit(_state.DE.low, 1);
    case 0x4C:
      return Bit(_state.HL.high, 1);
    case 0x4D:
      return Bit(_state.HL.low, 1);
    case 0x4E:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 1);
    case 0x4F:
      return Bit(_state.AF.high, 1);
    case 0x50:
      return Bit(_state.BC.high, 2);
    case 0x51:
      return Bit(_state.BC.low, 2);
    case 0x52:
      return Bit(_state.DE.high, 2);
    case 0x53:
      return Bit(_state.DE.low, 2);
    case 0x54:
      return Bit(_state.HL.high, 2);
    case 0x55:
      return Bit(_state.HL.low, 2);
    case 0x56:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 2);
    case 0x57:
      return Bit(_state.AF.high, 2);
    case 0x58:
      return Bit(_state.BC.high, 3);
    case 0x59:
      return Bit(_state.BC.low, 3);
    case 0x5A:
      return Bit(_state.DE.high, 3);
    case 0x5B:
      return Bit(_state.DE.low, 3);
    case 0x5C:
      return Bit(_state.HL.high, 3);
    case 0x5D:
      return Bit(_state.HL.low, 3);
    case 0x5E:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 3);
    case 0x5F:
      return Bit(_state.AF.high, 3);
    case 0x60:
      return Bit(_state.BC.high, 4);
    case 0x61:
      return Bit(_state.BC.low, 4);
    case 0x62:
      return Bit(_state.DE.high, 4);
    case 0x63:
      return Bit(_state.DE.low, 4);
    case 0x64:
      return Bit(_state.HL.high, 4);
    case 0x65:
      return Bit(_state.HL.low, 4);
    case 0x66:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 4);
    case 0x67:
      return Bit(_state.AF.high, 4);
    case 0x68:
      return Bit(_state.BC.high, 5);
    case 0x69:
      return Bit(_state.BC.low, 5);
    case 0x6A:
      return Bit(_state.DE.high, 5);
    case 0x6B:
      return Bit(_state.DE.low, 5);
    case 0x6C:
      return Bit(_state.HL.high, 5);
    case 0x6D:
      return Bit(_state.HL.low, 5);
    case 0x6E:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 5);
    case 0x6F:
      return Bit(_state.AF.high, 5);
    case 0x70:
      return Bit(_state.BC.high, 6);
    case 0x71:
      return Bit(_state.BC.low, 6);
    case 0x72:
      return Bit(_state.DE.high, 6);
    case 0x73:
      return Bit(_state.DE.low, 6);
    case 0x74:
      return Bit(_state.HL.high, 6);
    case 0x75:
      return Bit(_state.HL.low, 6);
    case 0x76:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 6);
    case 0x77:
      return Bit(_state.AF.high, 6);
    case 0x78:
      return Bit(_state.BC.high, 7);
    case 0x79:
      return Bit(_state.BC.low, 7);
    case 0x7A:
      return Bit(_state.DE.high, 7);
    case 0x7B:
      return Bit(_state.DE.low, 7);
    case 0x7C:
      return Bit(_state.HL.high, 7);
    case 0x7D:
      return Bit(_state.HL.low, 7);
    case 0x7E:
      return 4 + Bit(_mmu.Read(_state.HL.reg), 7);
    case 0x7F:
      return Bit(_state.AF.high, 7);
    case 0x80:
      return Res(_state.BC.high, 0);
    case 0x81:
      return Res(_state.BC.low, 0);
    case 0x82:
      return Res(_state.DE.high, 0);
    case 0x83:
      return Res(_state.DE.low, 0);
    case 0x84:
      return Res(_state.HL.high, 0);
    case 0x85:
      return Res(_state.HL.low, 0);
    case 0x86:
      return 8 + Res(_mmu.Address(_state.HL.reg), 0);
    case 0x87:
      return Res(_state.AF.high, 0);
    case 0x88:
      return Res(_state.BC.high, 1);
    case 0x89:
      return Res(_state.BC.low, 1);
    case 0x8A:
      return Res(_state.DE.high, 1);
    case 0x8B:
      return Res(_state.DE.low, 1);
    case 0x8C:
      return Res(_state.HL.high, 1);
    case 0x8D:
      return Res(_state.HL.low, 1);
    case 0x8E:
      return 8 + Res(_mmu.Address(_state.HL.reg), 1);
    case 0x8F:
      return Res(_state.AF.high, 1);
    case 0x90:
      return Res(_state.BC.high, 2);
    case 0x91:
      return Res(_state.BC.low, 2);
    case 0x92:
      return Res(_state.DE.high, 2);
    case 0x93:
      return Res(_state.DE.low, 2);
    case 0x94:
      return Res(_state.HL.high, 2);
    case 0x95:
      return Res(_state.HL.low, 2);
    case 0x96:
      return 8 + Res(_mmu.Address(_state.HL.reg), 2);
    case 0x97:
      return Res(_state.AF.high, 2);
    case 0x98:
      return Res(_state.BC.high, 3);
    case 0x99:
      return Res(_state.BC.low, 3);
    case 0x9A:
      return Res(_state.DE.high, 3);
    case 0x9B:
      return Res(_state.DE.low, 3);
    case 0x9C:
      return Res(_state.HL.high, 3);
    case 0x9D:
      return Res(_state.HL.low, 3);
    case 0x9E:
      return 8 + Res(_mmu.Address(_state.HL.reg), 3);
    case 0x9F:
      return Res(_state.AF.high, 3);
    case 0xA0:
      return Res(_state.BC.high, 4);
    case 0xA1:
      return Res(_state.BC.low, 4);
    case 0xA2:
      return Res(_state.DE.high, 4);
    case 0xA3:
      return Res(_state.DE.low, 4);
    case 0xA4:
      return Res(_state.HL.high, 4);
    case 0xA5:
      return Res(_state.HL.low, 4);
    case 0xA6:
      return 8 + Res(_mmu.Address(_state.HL.reg), 4);
    case 0xA7:
      return Res(_state.AF.high, 4);
    case 0xA8:
      return Res(_state.BC.high, 5);
    case 0xA9:
      return Res(_state.BC.low, 5);
    case 0xAA:
      return Res(_state.DE.high, 5);
    case 0xAB:
      return Res(_state.DE.low, 5);
    case 0xAC:
      return Res(_state.HL.high, 5);
    case 0xAD:
      return Res(_state.HL.low, 5);
    case 0xAE:
      return 8 + Res(_mmu.Address(_state.HL.reg), 5);
    case 0xAF:
      return Res(_state.AF.high, 5);
    case 0xB0:
      return Res(_state.BC.high, 6);
    case 0xB1:
      return Res(_state.BC.low, 6);
    case 0xB2:
      return Res(_state.DE.high, 6);
    case 0xB3:
      return Res(_state.DE.low, 6);
    case 0xB4:
      return Res(_state.HL.high, 6);
    case 0xB5:
      return Res(_state.HL.low, 6);
    case 0xB6:
      return 8 + Res(_mmu.Address(_state.HL.reg), 6);
    case 0xB7:
      return Res(_state.AF.high, 6);
    case 0xB8:
      return Res(_state.BC.high, 7);
    case 0xB9:
      return Res(_state.BC.low, 7);
    case 0xBA:
      return Res(_state.DE.high, 7);
    case 0xBB:
      return Res(_state.DE.low, 7);
    case 0xBC:
      return Res(_state.HL.high, 7);
    case 0xBD:
      return Res(_state.HL.low, 7);
    case 0xBE:
      return 8 + Res(_mmu.Address(_state.HL.reg), 7);
    case 0xBF:
      return Res(_state.AF.high, 7);
    case 0xC0:
      return Set(_state.BC.high, 0);
    case 0xC1:
      return Set(_state.BC.low, 0);
    case 0xC2:
      return Set(_state.DE.high, 0);
    case 0xC3:
      return Set(_state.DE.low, 0);
    case 0xC4:
      return Set(_state.HL.high, 0);
    case 0xC5:
      return Set(_state.HL.low, 0);
    case 0xC6:
      return 8 + Set(_mmu.Address(_state.HL.reg), 0);
    case 0xC7:
      return Set(_state.AF.high, 0);
    case 0xC8:
      return Set(_state.BC.high, 1);
    case 0xC9:
      return Set(_state.BC.low, 1);
    case 0xCA:
      return Set(_state.DE.high, 1);
    case 0xCB:
      return Set(_state.DE.low, 1);
    case 0xCC:
      return Set(_state.HL.high, 1);
    case 0xCD:
      return Set(_state.HL.low, 1);
    case 0xCE:
      return 8 + Set(_mmu.Address(_state.HL.reg), 1);
    case 0xCF:
      return Set(_state.AF.high, 1);
    case 0xD0:
      return Set(_state.BC.high, 2);
    case 0xD1:
      return Set(_state.BC.low, 2);
    case 0xD2:
      return Set(_state.DE.high, 2);
    case 0xD3:
      return Set(_state.DE.low, 2);
    case 0xD4:
      return Set(_state.HL.high, 2);
    case 0xD5:
      return Set(_state.HL.low, 2);
    case 0xD6:
      return 8 + Set(_mmu.Address(_state.HL.reg), 2);
    case 0xD7:
      return Set(_state.AF.high, 2);
    case 0xD8:
      return Set(_state.BC.high, 3);
    case 0xD9:
      return Set(_state.BC.low, 3);
    case 0xDA:
      return Set(_state.DE.high, 3);
    case 0xDB:
      return Set(_state.DE.low, 3);
    case 0xDC:
      return Set(_state.HL.high, 3);
    case 0xDD:
      return Set(_state.HL.low, 3);
    case 0xDE:
      return 8 + Set(_mmu.Address(_state.HL.reg), 3);
    case 0xDF:
      return Set(_state.AF.high, 3);
    case 0xE0:
      return Set(_state.BC.high, 4);
    case 0xE1:
      return Set(_state.BC.low, 4);
    case 0xE2:
      return Set(_state.DE.high, 4);
    case 0xE3:
      return Set(_state.DE.low, 4);
    case 0xE4:
      return Set(_state.HL.high, 4);
    case 0xE5:
      return Set(_state.HL.low, 4);
    case 0xE6:
      return 8 + Set(_mmu.Address(_state.HL.reg), 4);
    case 0xE7:
      return Set(_state.AF.high, 4);
    case 0xE8:
      return Set(_state.BC.high, 5);
    case 0xE9:
      return Set(_state.BC.low, 5);
    case 0xEA:
      return Set(_state.DE.high, 5);
    case 0xEB:
      return Set(_state.DE.low, 5);
    case 0xEC:
      return Set(_state.HL.high, 5);
    case 0xED:
      return Set(_state.HL.low, 5);
    case 0xEE:
      return 8 + Set(_mmu.Address(_state.HL.reg), 5);
    case 0xEF:
      return Set(_state.AF.high, 5);
    case 0xF0:
      return Set(_state.BC.high, 6);
    case 0xF1:
      return Set(_state.BC.low, 6);
    case 0xF2:
      return Set(_state.DE.high, 6);
    case 0xF3:
      return Set(_state.DE.low, 6);
    case 0xF4:
      return Set(_state.HL.high, 6);
    case 0xF5:
      return Set(_state.HL.low, 6);
    case 0xF6:
      return 8 + Set(_mmu.Address(_state.HL.reg), 6);
    case 0xF7:
      return Set(_state.AF.high, 6);
    case 0xF8:
      return Set(_state.BC.high, 7);
    case 0xF9:
      return Set(_state.BC.low, 7);
    case 0xFA:
      return Set(_state.DE.high, 7);
    case 0xFB:
      return Set(_state.DE.low, 7);
    case 0xFC:
      return Set(_state.HL.high, 7);
    case 0xFD:
      return Set(_state.HL.low, 7);
    case 0xFE:
      return 8 + Set(_mmu.Address(_state.HL.reg), 7);
    case 0xFF:
      return Set(_state.AF.high, 7);
    default:
      throw std::runtime_error(std::format(
          "[CB] PC: {:#06X}, failed to execute instruction \033[31m{:#04X}\033[0m",
          _state.PC.reg - 1, opcode));
  }
  return 0x00;
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

int Cpu::DecHl()
{
  uint8_t data = _mmu.Read(_state.HL.reg);
  uint16_t res = data - 1;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((data & 0x0FU) - 1U) > 0x0F);

  _mmu.Write(_state.HL.reg, (res & 0xFFU));
  return 12;
}

int Cpu::LdAU16()
{
  uint8_t lsb = _mmu.Read(_state.PC.reg++);
  uint8_t msb = _mmu.Read(_state.PC.reg++);
  _state.AF.high = _mmu.Read(ToU16(lsb, msb));
  return 16;
}

int Cpu::LdSpHl()
{
  _state.SP.reg = _state.HL.reg;
  return 8;
}

int Cpu::LdHlS8()
{
  auto i8 = static_cast<int8_t>(_mmu.Read(_state.PC.reg++));
  uint16_t res = _state.SP.reg + static_cast<std::uint16_t>(i8);
  SetZ(false);
  SetN(false);
  SetH(((_state.SP.reg ^ i8 ^ res) & 0x10) != 0);
  SetCY(((_state.SP.reg ^ i8 ^ res) & 0x100) != 0);

  _state.HL.reg = res;
  return 12;
}

int Cpu::OrU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) | static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::LdAC()
{
  _state.AF.high = _mmu.Read(0xFF00 + _state.BC.low);
  return 8;
}

int Cpu::AndU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint8_t res = _state.AF.high & u8;
  SetZ(res == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  _state.AF.high = res;
  return 8;
}

int Cpu::AddSpS8()
{
  auto i8 = static_cast<int8_t>(_mmu.Read(_state.PC.reg++));
  uint16_t res = _state.SP.reg + static_cast<std::uint16_t>(i8);

  SetZ(false);
  SetN(false);
  SetH(((_state.SP.reg ^ i8 ^ res) & 0x10) != 0);  // Half-carry detection
  SetCY(((_state.SP.reg ^ i8 ^ res) & 0x100) != 0);

  _state.SP.reg = res;
  return 16;
}

int Cpu::JpHl()
{
  _state.PC.reg = _state.HL.reg;
  return 4;
}

int Cpu::XorU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint8_t res = _state.AF.high ^ u8;
  SetZ(res == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = res;
  return 8;
}

int Cpu::RetI()
{
  uint8_t lsb = _mmu.Read(_state.SP.reg++);
  uint8_t msb = _mmu.Read(_state.SP.reg++);
  _state.PC.reg = ToU16(lsb, msb);
  _state._interrupt->_ime = true;
  return 16;
}

int Cpu::SubU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::SbcU8()
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
  return 8;
}

int Cpu::AddU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  AddR(u8);
  return 8;
}

int Cpu::AdcU8()
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
  return 8;
}

int Cpu::AndR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) & static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::AndIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) & static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(true);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::XorR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) ^ static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::XorIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) ^ static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::OrR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) | static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::OrIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) | static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(false);
  SetCY(false);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::CpR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
  return 4;
}

int Cpu::CpIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
  return 8;
}

int Cpu::SubIHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::SbcIHl()
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
  return 8;
}

int Cpu::SbcR(std::uint8_t reg)
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(reg) - c;

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (reg & 0xFU) - c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::AdcIHl()
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
  return 8;
}

int Cpu::AddR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high ^ reg ^ res) & 0x10) != 0);
  SetCY(((_state.AF.high ^ reg ^ res) & 0x100) != 0);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::AdcR(std::uint8_t reg)
{
  auto c = static_cast<uint16_t>((_state.AF.low & (1U << 4U)) >> 4U);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(reg) + c;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high & 0xFU) + (reg & 0xFU) + c) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::Halt()
{
  throw std::runtime_error(
      std::format("Unimplemented instruction: 0x76 (HALT)"));
}

int Cpu::Ccf()
{
  SetN(false);
  SetH(false);

  _state.AF.low = _state.AF.low ^ (1U << 4U);
  return 4;
}

int Cpu::LdAHlN()
{
  _state.AF.high = _mmu.Read(_state.HL.reg);
  _state.HL.reg = _state.HL.reg - 1;
  return 8;
}

int Cpu::Scf()
{
  SetN(false);
  SetH(false);
  SetCY(true);
  return 4;
}

int Cpu::IncIHl()
{
  uint8_t data = _mmu.Read(_state.HL.reg);
  uint16_t res = data + 1;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((data & 0x0FU) + 1U) > 0x0F);

  _mmu.Write(_state.HL.reg, (res & 0xFFU));
  return 12;
}

int Cpu::Cpl()
{
  SetN(true);
  SetH(true);

  _state.AF.high = static_cast<std::uint8_t>(~_state.AF.high);
  return 4;
}

// DAA
// (https://forums.nesdev.org/viewtopic.php?p=196282&sid=a1cdd6adc0b01ea3d77f61aee9527449#p196282)
int Cpu::Daa()
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

  return 4;
}

int Cpu::Rra()
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY((_state.AF.high & (1U << 0U)));

  _state.AF.high = _state.AF.high >> 1U;
  _state.AF.high =
      (_state.AF.high & ~(1U << 7U)) | static_cast<uint8_t>(oldCY << 7U);
  return 4;
}

int Cpu::Stop()
{
  // TODO: check what needs to be done here
  LOG_WARN(_logger, std::format("Unimplemented instruction: 0x10 (STOP)"));
  return 4;
}

int Cpu::Rrca()
{
  uint8_t b0 = (_state.AF.high & (1U << 0U));

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY(b0);

  _state.AF.high = _state.AF.high >> 1U;
  _state.AF.high =
      (_state.AF.high & ~(1U << 7U)) | static_cast<uint8_t>(b0 << 7U);
  return 4;
}

int Cpu::LdDU16Sp()
{
  uint8_t lsb = _mmu.Read(_state.PC.reg++);
  uint8_t msb = _mmu.Read(_state.PC.reg++);
  uint16_t nn = ToU16(lsb, msb);
  _mmu.Write(nn, _state.SP.low);
  ++nn;
  _mmu.Write(nn, _state.SP.high);
  return 20;
}

int Cpu::Rlca()
{
  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY((_state.AF.high & (1U << 7U)) >> 7U);

  _state.AF.high = static_cast<std::uint8_t>(_state.AF.high << 1U);
  _state.AF.high =
      ((_state.AF.high & ~(1U << 0U)) | ((_state.AF.low & (1U << 4U)) >> 4U));
  return 4;
}

int Cpu::LdIRrA(Register &reg)
{
  _mmu.Write(reg.reg, _state.AF.high);
  return 8;
}

int Cpu::AddHlRr(Register &reg)
{
  uint32_t res =
      static_cast<uint32_t>(_state.HL.reg) + static_cast<uint32_t>(reg.reg);

  SetN(false);
  SetH(((_state.HL.reg & 0xFFFU) + (reg.reg & 0xFFFU)) > 0xFFFU);
  SetCY(res > 0xFFFFU);

  _state.HL.reg = res & 0xFFFFU;
  return 8;
}

int Cpu::RstU8(std::uint8_t addr)
{
  _state.SP.reg--;
  _mmu.Write(_state.SP.reg, _state.PC.high);
  _state.SP.reg--;
  _mmu.Write(_state.SP.reg, _state.PC.low);
  _state.PC.reg = ToU16(addr, 0x00);
  return 16;
}

int Cpu::LdRIHl(std::uint8_t &reg)
{
  reg = _mmu.Read(_state.HL.reg);
  return 8;
}

int Cpu::Ei()
{
  _state._interrupt->_enableRequested = true;
  LOG_DEBUG(_logger, "Enable interrupt requested");
  return 4;
}

int Cpu::DecRr(Register &reg)
{
  reg.reg--;
  return 8;
}

int Cpu::LdAHlP()
{
  _state.AF.high = _mmu.Read(_state.HL.reg);
  _state.HL.reg++;
  return 8;
}

int Cpu::LdHlU8()
{
  auto data = _mmu.Read(_state.PC.reg++);
  _mmu.Write(_state.HL.reg, data);
  return 12;
}

int Cpu::Di()
{
  _state._interrupt->_enableRequested = false;
  _state._interrupt->_ime = false;
  LOG_DEBUG(_logger, "Interrupt disabled");
  return 4;
}

int Cpu::JpCcU16(bool cc)
{
  auto lsb = _mmu.Read(_state.PC.reg++);
  auto msb = _mmu.Read(_state.PC.reg++);
  if (cc)
  {
    _state.PC.reg = ToU16(lsb, msb);
    return 16;
  }
  else
  {
    return 12;
  }
}

int Cpu::Nop()
{
  return 4;
}

int Cpu::SubR(std::uint8_t reg)
{
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(reg);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (reg & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);

  _state.AF.high = (res & 0xFFU);
  return 4;
}

int Cpu::AddAHl()
{
  auto data = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) + static_cast<uint16_t>(data);

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((_state.AF.high ^ data ^ res) & 0x10) != 0);
  SetCY(((_state.AF.high ^ data ^ res) & 0x100) != 0);

  _state.AF.high = (res & 0xFFU);
  return 8;
}

int Cpu::LdU16A()
{
  auto low = _mmu.Read(_state.PC.reg++);
  auto high = _mmu.Read(_state.PC.reg++);
  _mmu.Write(ToU16(low, high), _state.AF.high);
  return 16;
}
int Cpu::CpU8()
{
  uint8_t u8 = _mmu.Read(_state.PC.reg++);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
  return 8;
}

int Cpu::CpAHl()
{
  uint8_t u8 = _mmu.Read(_state.HL.reg);
  uint16_t res =
      static_cast<uint16_t>(_state.AF.high) - static_cast<uint16_t>(u8);

  SetZ((res & 0xFFU) == 0);
  SetN(true);
  SetH(((_state.AF.high & 0xFU) - (u8 & 0xFU)) > 0xFU);
  SetCY(res > 0xFFU);
  return 0x00;
}

int Cpu::LdHlPA()
{
  _mmu.Write(_state.HL.reg, _state.AF.high);
  ++_state.HL.reg;
  return 8;
}

int Cpu::DecR(std::uint8_t &reg)
{
  std::uint8_t res = reg - 1;

  SetZ(res == 0);
  SetN(true);
  SetH(((reg & 0xFU) - 1) > 0xFU);

  reg = res;
  return 4;
}

int Cpu::PopRr(Register &reg)
{
  reg.low = _mmu.Read(_state.SP.reg);
  ++_state.SP.reg;
  reg.high = _mmu.Read(_state.SP.reg);
  ++_state.SP.reg;
  return 12;
}

int Cpu::RlA()
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;

  SetZ(false);
  SetN(false);
  SetH(false);
  SetCY(((_state.AF.high & (1U << 7U)) >> 7U) == 1U);

  _state.AF.high = static_cast<uint8_t>(_state.AF.high << 1U);
  _state.AF.high = (_state.AF.high & static_cast<uint8_t>(~(1U << 0U))) | oldCY;
  return 4;
}

int Cpu::PushRr(Register &reg)
{
  --_state.SP.reg;
  _mmu.Write(_state.SP.reg, reg.high);
  --_state.SP.reg;
  _mmu.Write(_state.SP.reg, reg.low);
  return 16;
}

int Cpu::LdRR(std::uint8_t &reg1, std::uint8_t &reg2)
{
  reg1 = reg2;
  return 4;
}

int Cpu::CAllCcU16(bool cc)
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
    return 24;
  }
  else
  {
    return 12;
  }
}

int Cpu::RetCc(bool cc)
{
  if (cc)
  {
    auto low = _mmu.Read(_state.SP.reg);
    ++_state.SP.reg;
    auto high = _mmu.Read(_state.SP.reg);
    ++_state.SP.reg;
    _state.PC.reg = ToU16(low, high);
    return 20;
  }
  else
  {
    return 8;
  }
}

int Cpu::LdRrU16(Register &reg)
{
  reg.low = _mmu.Read(_state.PC.reg++);
  reg.high = _mmu.Read(_state.PC.reg++);
  return 12;
}

int Cpu::LdRU8(std::uint8_t &reg)
{
  reg = _mmu.Read(_state.PC.reg++);
  return 8;
}

int Cpu::LdAIRr(Register &reg)
{
  _state.AF.high = _mmu.Read(reg.reg);
  return 8;
}

int Cpu::LdHlMA()
{
  _mmu.Write(_state.HL.reg, _state.AF.high);
  --_state.HL.reg;
  return 8;
}

int Cpu::LdIHlR(std::uint8_t &reg)
{
  _mmu.Write(_state.HL.reg, reg);
  return 8;
}

int Cpu::JrCCI8(bool cc)
{
  auto i8 = static_cast<std::int8_t>(_mmu.Read(_state.PC.reg++));
  if (cc)
  {
    _state.PC.reg = static_cast<std::uint16_t>(_state.PC.reg + i8);
    return 12;
  }
  else
  {
    return 8;
  }
}

int Cpu::LdhAU8()
{
  auto low = _mmu.Read(_state.PC.reg++);
  auto addr = ToU16(low, 0xFF);
  _state.AF.high = _mmu.Read(addr);
  return 12;
}

int Cpu::LdhCA()
{
  auto addr = ToU16(_state.BC.low, 0xFF);
  _mmu.Write(addr, _state.AF.high);
  return 8;
}

int Cpu::LdhU8A()
{
  auto addr = ToU16(_mmu.Read(_state.PC.reg++), 0xFF);
  _mmu.Write(addr, _state.AF.high);
  return 12;
}

int Cpu::IncR(std::uint8_t &reg)
{
  uint16_t res = reg + 1;

  SetZ((res & 0xFFU) == 0);
  SetN(false);
  SetH(((reg & 0x0FU) + 1) > 0x0FU);

  reg = (res & 0xFFU);
  return 4;
}

int Cpu::IncRr(Register &reg)
{
  ++reg.reg;
  return 8;
}

// extended opcodes
int Cpu::RlR(std::uint8_t &reg)
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;
  uint8_t bit7 = (reg & (1U << 7U)) >> 7U;
  reg = static_cast<uint8_t>(reg << 1U);
  reg = (reg & static_cast<std::uint8_t>(~(1U << 0U))) | oldCY;

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
  return 0x00;
}

int Cpu::BitBR(unsigned int bit, std::uint8_t reg)
{
  auto bitValue = (reg & static_cast<std::uint8_t>(1U << bit)) >> bit;
  SetZ(bitValue == 0);
  SetN(false);
  SetH(true);
  return 0x00;
}

// extended opcodes
int Cpu::Rlc(uint8_t &reg)
{
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  reg = static_cast<std::uint8_t>(reg << 1U);
  reg = (reg & ~(1U << 0U)) | (bit7);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
  return 8;
}

int Cpu::Rrc(uint8_t &reg)
{
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;
  reg = (reg & static_cast<std::uint8_t>(~(1U << 7U)))
        | static_cast<std::uint8_t>(bit0 << 7U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
  return 8;
}

int Cpu::Rl(uint8_t &reg)
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  reg = static_cast<std::uint8_t>(reg << 1U);
  reg = (reg & ~(1U << 0U)) | (oldCY);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
  return 8;
}

int Cpu::Rr(uint8_t &reg)
{
  uint8_t oldCY = (_state.AF.low & (1U << 4U)) >> 4U;
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;
  reg = (reg & static_cast<std::uint8_t>(~(1U << 7U)))
        | static_cast<std::uint8_t>(oldCY << 7U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
  return 8;
}

int Cpu::Sla(uint8_t &reg)
{
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  reg = static_cast<std::uint8_t>(reg << 1U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit7 == 1U);
  return 8;
}

int Cpu::Sra(uint8_t &reg)
{
  uint8_t bit7 = (reg & 0x80U) >> 7U;
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;
  reg = (reg & static_cast<std::uint8_t>(~(1U << 7U)))
        | static_cast<std::uint8_t>(bit7 << 7U);

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
  return 8;
}

int Cpu::Srl(uint8_t &reg)
{
  uint8_t bit0 = (reg & 0x01U);
  reg = reg >> 1U;

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(bit0 == 1U);
  return 8;
}

int Cpu::Swap(uint8_t &reg)
{
  uint8_t lowerNibble = (reg & 0x0FU);
  reg = reg >> 4U;
  reg = (reg & (0x0FU)) | static_cast<std::uint8_t>((lowerNibble << 4U));

  SetZ(reg == 0);
  SetN(false);
  SetH(false);
  SetCY(false);
  return 8;
}

int Cpu::Bit(uint8_t reg, uint8_t bit)
{
  uint8_t bitX = (reg & (1U << bit)) >> bit;
  _state.AF.low = (_state.AF.low & static_cast<std::uint8_t>(~(1U << 7U)))
                  | static_cast<std::uint8_t>(((!bitX) << 7U));
  SetN(false);
  SetH(true);
  return 8;
}

int Cpu::Res(uint8_t &reg, uint8_t bit)
{
  reg = (reg & ~(1U << bit));
  return 8;
}

int Cpu::Set(uint8_t &reg, uint8_t bit)
{
  reg = (reg & static_cast<std::uint8_t>(~(1U << bit)))
        | static_cast<std::uint8_t>((1U << bit));
  return 8;
}

// utility

// Set zero flag
void Cpu::SetZ(bool value)
{
  _state.AF.low = (_state.AF.low & static_cast<std::uint8_t>(~(1U << 7U)))
                  | static_cast<uint8_t>(value << 7U);
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
  _state.AF.low = (_state.AF.low & static_cast<std::uint8_t>(~(1U << 6U)))
                  | static_cast<uint8_t>(value << 6U);
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
  _state.AF.low = (_state.AF.low & static_cast<std::uint8_t>(~(1U << 5U)))
                  | static_cast<uint8_t>(value << 5U);
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
  _state.AF.low = (_state.AF.low & static_cast<std::uint8_t>(~(1U << 4U)))
                  | static_cast<uint8_t>(value << 4U);
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
