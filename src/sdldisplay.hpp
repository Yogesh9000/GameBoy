#pragma once

#include <SDL3/SDL.h>

#include <vector>

#include "display.hpp"

class SdlDisplay : public Display
{
public:
  constexpr static int LCD_WIDTH{160};
  constexpr static int LCD_HEIGHT{144};
  constexpr static int SCALE{4};
  constexpr static int PIXEL_SIZE{3};

public:
  explicit SdlDisplay(const char *title);

  ~SdlDisplay() override;

  void Draw(const Pixel &pixel) override;

  void UpdateFrame() override;

private:
  SDL_Window *_window;
  SDL_Renderer *_renderer;
  SDL_Texture *_texture;
  std::vector<std::uint8_t> _pixels;
};
