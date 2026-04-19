#include <gtest/gtest.h>

#include <format>
#include <fstream>
#include <string>

#include "blarggstestmemoryrange.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "timer.hpp"

static void TestInstructionBlarggs(std::string romPath)
{
  std::string filePath =
      std::format("blarggs/cpu_instrs/individual/{}.gb", romPath);
  std::ifstream file{filePath, std::ios::binary};
  ASSERT_TRUE(file.is_open());

  MemoryManagementUnit mmu;
  auto timer = std::make_shared<Timer>(mmu);
  mmu.AddMemoryRange(timer);

  auto mr = std::make_shared<BlarggsTestMemoryRange>(0x10000, 0x00);
  mmu.AddMemoryRange(mr);

  uint16_t address{0};
  uint8_t byte{};
  while (file.read(reinterpret_cast<char *>(&byte), 1))
  {
    mmu.Write(address, byte);
    ++address;
  }
  file.close();

  CpuState state;
  state.AF.reg = 0x01B0;
  state.BC.reg = 0x0013;
  state.DE.reg = 0x00D8;
  state.HL.reg = 0x014D;
  state.SP.reg = 0xFFFE;
  state.PC.reg = 0x0100;

  Cpu cpu(state, mmu);
  while (!mr->IsTestCompleted())
  {
    int cycles = cpu.Tick();
    timer->Tick(cycles);
  }

  EXPECT_EQ(true, mr->IsTestPassed()) << mr->GetMessage();
}

#define TEST_DEF(n, x, r)      \
  TEST(n, x)                   \
  {                            \
    TestInstructionBlarggs(r); \
  }

TEST_DEF(BLARGG_CPU_INDIVIDUAL, 01_SPECIAL, "01-special");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, DISABLED_02_INTERRUPTS, "02-interrupts");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 03_SP_HL, "03-op sp,hl");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 04_R_IMM, "04-op r,imm");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 05_RP, "05-op rp");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 06_LD_R_R, "06-ld r,r");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 07_JR_JP_CALL_RET_RST, "07-jr,jp,call,ret,rst");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 08_MIST, "08-misc instrs");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 09_R_R, "09-op r,r");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 10_BIT, "10-bit ops");
TEST_DEF(BLARGG_CPU_INDIVIDUAL, 11_A_HL, "11-op a,(hl)");
