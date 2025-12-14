#include <exception>
#include <format>
#include <memory>

#include "SDL3/SDL_events.h"
#include "bootrom.hpp"
#include "common.hpp"
#include "concretememoryrange.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"
#include "sdldisplay.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char **argv)
{
  spdlog::set_level(spdlog::level::debug);
  // early exit if no rom present
  if (argc != 2)
  {
    SPDLOG_ERROR("No rom path provided\n");
    return 1;
  }
  char *romPath = argv[1];  // rom filepath

  MemoryManagementUnit mmu;

  // load the bootrom and add it to romPath
  mmu.AddMemoryRange(
      std::make_shared<BootRom>(std::move(BootRom::Create(romPath))));
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
    SPDLOG_ERROR("{}\n", ex.what());
  }
  return 0;
}
