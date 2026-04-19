#pragma once

#include <spdlog/logger.h>

#include <memory>

#include "interrupt.hpp"
#include "mmu.hpp"
#include "register.hpp"

struct CpuState
{
  Register AF{};
  Register BC{};
  Register DE{};
  Register HL{};
  Register SP{};
  Register PC{};
  std::shared_ptr<Interrupt> _interrupt;
};

class Cpu
{
public:
  explicit Cpu(MemoryManagementUnit &mmu);
  Cpu(CpuState state, MemoryManagementUnit &mmu);

  [[nodiscard]]
  CpuState GetCpuState() const;

  int Tick();

private:
  int TickExtended();

  void HandleInterruptsIfAny();
  void DisableInterruptAndJumpToInterruptHandler(InterruptType interruptType);

  // opcodes
  int DecHl();
  int LdAU16();
  int LdSpHl();
  int LdHlS8();
  int OrU8();
  int LdAC();
  int AndU8();
  int AddSpS8();
  int JpHl();
  int XorU8();
  int RetI();
  int SubU8();
  int SbcU8();
  int AddU8();
  int AdcU8();
  int AndR(std::uint8_t reg);
  int AndIHl();
  int XorR(std::uint8_t reg);
  int XorIHl();
  int OrR(std::uint8_t reg);
  int OrIHl();
  int CpR(std::uint8_t reg);
  int CpIHl();
  int SubIHl();
  int SbcIHl();
  int SbcR(std::uint8_t reg);
  int AdcIHl();
  int AddR(std::uint8_t reg);
  int AdcR(std::uint8_t reg);
  int Halt();
  int Ccf();
  int LdAHlN();
  int Scf();
  int IncIHl();
  int Cpl();
  int Daa();
  int Rra();
  int Stop();
  int Rrca();
  int LdDU16Sp();
  int Rlca();
  int LdIRrA(Register &reg);
  int AddHlRr(Register &reg);
  int RstU8(std::uint8_t addr);
  int LdRIHl(std::uint8_t &reg);
  int Ei();
  int DecRr(Register &reg);
  int LdAHlP();
  int LdHlU8();
  int Di();
  int JpCcU16(bool c);
  int Nop();
  int SubR(std::uint8_t reg);
  int AddAHl();
  int LdU16A();
  int CpU8();
  int CpAHl();
  int LdHlPA();
  int DecR(std::uint8_t &reg);
  int PopRr(Register &reg);
  int RlA();
  int PushRr(Register &reg);
  int LdRR(std::uint8_t &reg1, std::uint8_t &reg2);
  int CAllCcU16(bool c);
  int RetCc(bool c);
  int LdRrU16(Register &reg);
  int LdRU8(std::uint8_t &reg);
  int LdAIRr(Register &reg);
  int LdHlMA();
  int LdIHlR(std::uint8_t &reg);
  int JrCCI8(bool cc);
  int LdhAU8();
  int LdhCA();
  int LdhU8A();
  int IncR(std::uint8_t &reg);
  int IncRr(Register &reg);

  // extended opcodes

  int Rlc(uint8_t &reg);
  int RlR(std::uint8_t &reg);
  int BitBR(unsigned int bit, std::uint8_t reg);
  int Rrc(uint8_t &reg);
  int Rl(uint8_t &reg);
  int Rr(uint8_t &reg);
  int Sla(uint8_t &reg);
  int Sra(uint8_t &reg);
  int Srl(uint8_t &reg);
  int Swap(uint8_t &reg);
  int Bit(uint8_t reg, uint8_t bit);
  int Res(uint8_t &reg, uint8_t bit);
  int Set(uint8_t &reg, uint8_t bit);

  // utility

  // Set zero flag
  void SetZ(bool value);

  // Get value of zero flag
  [[nodiscard]]
  bool GetZ() const;

  // Set negative flag
  void SetN(bool value);

  // Get value of negative flag
  [[nodiscard]]
  bool GetN() const;

  // Set half carry flag
  void SetH(bool value);

  // Get value of half carry flag
  [[nodiscard]]
  bool GetH() const;

  // Set carry flag
  void SetCY(bool value);

  // Get value of carry flag
  [[nodiscard]]
  bool GetCY() const;

  static std::uint16_t ToU16(std::uint8_t lsb, std::uint8_t msb);

private:
  CpuState _state;
  MemoryManagementUnit _mmu;
  std::shared_ptr<spdlog::logger> _logger{};
};
