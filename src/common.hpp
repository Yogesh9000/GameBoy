#pragma once
#include <format>
#include <iostream>
#include <string>

// addresses
constexpr std::uint16_t LY_REGISTER_ADDRESS{0xFF44};
constexpr std::uint16_t LCDC_REGISTER_ADDRESS{0xFF40};
constexpr std::uint16_t OAM_SIZE{0xA0};
constexpr std::uint16_t OAM_START_ADDRESS{0xFE00};
constexpr std::uint16_t SCY_REGISTER_ADDRESS{0xFF42};
constexpr std::uint16_t SCX_REGISTER_ADDRESS{0xFF43};

constexpr std::uint16_t BG_WIN_TILEMAP_ADDRESS0{0x9800};
constexpr std::uint16_t BG_WIN_TILEMAP_ADDRESS1{0x9C00};
constexpr std::uint16_t BG_WIN_TILEDATA_ADDRESS0{0x9000}; // This address mode uses 0x9000 as the base address and offset are signed
constexpr std::uint16_t BG_WIN_TILEDATA_ADDRESS1{0x8000}; // This addresses mode uses 0x8000 as the base address and offset are unsigned
constexpr unsigned int BG_WIN_TILEMAP_ROW_SIZE{32};

constexpr unsigned int TILE_DATA_SIZE{16}; // each tile takes 16 bytes
constexpr unsigned int MAX_DOTS_PER_SCANLINE{456};

constexpr unsigned int BOOTROM_ENABLE_ADDRESS{ 0xFF50 }; // writing to this address disable's bootrom

constexpr unsigned int INTERRUPT_ENABLE{ 0xFFFF };
constexpr unsigned int INTERRUPT_FLAG{ 0xFF0F };

constexpr unsigned int VBLANK_INTERRUPT_HANDLER_ADDRESS{ 0x40 };
constexpr unsigned int STAT_INTERRUPT_HANDLER_ADDRESS{ 0x48 };
constexpr unsigned int TIMER_INTERRUPT_HANDLER_ADDRESS{ 0x50 };
constexpr unsigned int SERIAL_INTERRUPT_HANDLER_ADDRESS{ 0x58 };
constexpr unsigned int JOYPAD_INTERRUPT_HANDLER_ADDRESS{ 0x60 };
