#include <exception>
#include <format>
#include <memory>

#include "SDL3/SDL_events.h"
#include "bootrom.hpp"
#include "common.hpp"
#include "concretememoryrange.hpp"
#include "filememoryrange.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"
#include "sdldisplay.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char **argv)
{
  spdlog::set_level(spdlog::level::debug);
  // early exit if no rom present
  if (argc != 3)
  {
    SPDLOG_CRITICAL("No rom paths provided\n");
    return 1;
  }

  MemoryManagementUnit mmu;

  // load the bootrom and add it to romPath
  char *bootRomPath = argv[1];  // rom filepath
  auto bootRom = std::make_shared<BootRom>();
  bootRom->Load(bootRomPath);
  mmu.AddMemoryRange(bootRom);

  // load the game rom
  char *romPath = argv[2];  // rom filepath
  auto rom = std::make_shared<FileMemoryRange>();
  rom->Load(romPath, 0x0);
  mmu.AddMemoryRange(rom);

  // add work ram 1: 0xC000 - 0xCFFF
  mmu.AddMemoryRange(std::make_shared<ConcreteMemoryRange>(0x1000, 0xC000));
  // add work ram 1: 0xD000 - 0xDFFF
  mmu.AddMemoryRange(std::make_shared<ConcreteMemoryRange>(0x1000, 0xD000));

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
    SPDLOG_CRITICAL("{}\n", ex.what());
  }
  return 0;
}
