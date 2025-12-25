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
  void Di();
  void JpU16();
  void Nop();
  void SubR(std::uint8_t reg);
  void AddAHl();
  void LdU16A();
  void CpN();
  void CpAHl() ;
  void LdHlPA() ;
  void DecR(std::uint8_t& reg) ;
  void PopRR(Register& reg) ;
  void RlA() ;
  void PushRR(Register& reg) ;
  void LdRR(std::uint8_t& reg1, std::uint8_t& reg2) ;
  void CAllU16() ;
  void Ret() ;
  void LdRrU16(Register& reg) ;
  void LdRU8(std::uint8_t& reg) ;
  void XorAA() ;
  void LdADe() ;
  void LdHlMA() ;
  void LdHlR(std::uint8_t& reg) ;
  void JrCCI8(bool cc) ;
  void LdhAU8() ;
  void LdhCA() ;
  void LdhU8A() ;
  void IncR(std::uint8_t& reg) ;
  void IncRR(Register& reg) ;

  // extended opcodes
  void RlR(std::uint8_t& reg) ;
  void BitBR(unsigned int bit, std::uint8_t reg) ;

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
