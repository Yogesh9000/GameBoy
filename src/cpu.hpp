#pragma once

#include "mmu.hpp"
#include "register.hpp"
#include "interrupt.hpp"
#include <memory>

class Cpu
{
public:
  explicit Cpu(MemoryManagementUnit& mmu);

  void Tick();

private:
  void TickExtended();

  void HandleInterruptsIfAny();
  void DisableInterruptAndJumpToInterruptHandler(InterruptType interruptType);

  // opcodes
  void DecHl();
  void LdAU16();
  void LdSpHl();
  void LdHlS8();
  void OrU8();
  void LdAC();
  void AndU8();
  void AddSpS8();
  void JpHl();
  void XorU8();
  void RetI();
  void SubU8();
  void SbcU8();
  void AddU8();
  void AdcU8();
  void AndR(std::uint8_t reg);
  void AndIHl();
  void XorR(std::uint8_t reg);
  void XorIHl();
  void OrR(std::uint8_t reg);
  void OrIHl();
  void CpR(std::uint8_t reg);
  void CpIHl();
  void SubIHl();
  void SbcIHl();
  void SbcR(std::uint8_t reg);
  void AdcIHl();
  void AddR(std::uint8_t reg);
  void AdcR(std::uint8_t reg);
  void Halt();
  void Ccf();
  void LdAHlN();
  void Scf();
  void IncIHl();
  void Cpl();
  void Daa();
  void Rra();
  void Stop();
  void Rrca();
  void LdDU16Sp();
  void Rlca();
  void LdIRrA(Register& reg);
  void AddHlRr(Register& reg);
  void RstU8(std::uint8_t addr);
  void LdRIHl(std::uint8_t &reg);
  void Ei();
  void DecRr(Register& reg);
  void LdAHlP();
  void LdHlU8();
  void Di();
  void JpCcU16(bool c);
  void Nop();
  void SubR(std::uint8_t reg);
  void AddAHl();
  void LdU16A();
  void CpU8();
  void CpAHl() ;
  void LdHlPA() ;
  void DecR(std::uint8_t& reg) ;
  void PopRr(Register& reg) ;
  void RlA() ;
  void PushRr(Register& reg) ;
  void LdRR(std::uint8_t& reg1, std::uint8_t& reg2) ;
  void CAllCcU16(bool c) ;
  void RetCc(bool c) ;
  void LdRrU16(Register& reg) ;
  void LdRU8(std::uint8_t& reg) ;
  void LdAIRr(Register& reg) ;
  void LdHlMA() ;
  void LdIHlR(std::uint8_t& reg) ;
  void JrCCI8(bool cc) ;
  void LdhAU8() ;
  void LdhCA() ;
  void LdhU8A() ;
  void IncR(std::uint8_t& reg) ;
  void IncRr(Register& reg) ;

  // extended opcodes

  void Rlc(uint8_t& reg);
  void RlR(std::uint8_t& reg) ;
  void BitBR(unsigned int bit, std::uint8_t reg) ;
  void Rrc(uint8_t& reg);
  void Rl(uint8_t& reg);
  void Rr(uint8_t& reg);
  void Sla(uint8_t& reg);
  void Sra(uint8_t& reg);
  void Srl(uint8_t& reg);
  void Swap(uint8_t& reg);
  void Bit(uint8_t reg, uint8_t bit);
  void Res(uint8_t& reg, uint8_t bit);
  void Set(uint8_t& reg, uint8_t bit);

  // utility

  // Set zero flag
  void SetZ(bool value) ;

  // Get value of zero flag
  [[nodiscard]]
  bool GetZ() const ;

  // Set negative flag
  void SetN(bool value) ;

  // Get value of negative flag
  [[nodiscard]]
  bool GetN() const ;

  // Set half carry flag
  void SetH(bool value) ;

  // Get value of half carry flag
  [[nodiscard]]
  bool GetH() const ;

  // Set carry flag
  void SetCY(bool value) ;

  // Get value of carry flag
  [[nodiscard]]
  bool GetCY() const;

  static std::uint16_t ToU16(std::uint8_t lsb, std::uint8_t msb) ;

  // state
  Register AF{};
  Register BC{};
  Register DE{};
  Register HL{};
  Register SP{};
  Register PC{};

private:
  MemoryManagementUnit _mmu;
  std::shared_ptr<Interrupt> _interrupt;
};
