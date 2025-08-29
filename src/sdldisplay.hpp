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
  SdlDisplay(const char *title)
      : _pixels(
            static_cast<unsigned long>(PIXEL_SIZE * LCD_WIDTH * LCD_HEIGHT), 0)
  {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(title, LCD_WIDTH * SCALE, LCD_HEIGHT * SCALE,
        SDL_WINDOW_RESIZABLE, &_window, &_renderer);
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);
  }

  ~SdlDisplay() override
  {
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
  }

  void Draw(const Pixel &pixel) override
  {
    auto idx = (LCD_WIDTH * PIXEL_SIZE * pixel.y) + (PIXEL_SIZE * pixel.x);
    if (idx + 2 <= _pixels.size())
    {
      _pixels[idx] = pixel.color.red;
      _pixels[idx + 1] = pixel.color.green;
      _pixels[idx + 2] = pixel.color.blue;
    }
  }

  void UpdateFrame() override
  {
    SDL_UpdateTexture(
        _texture, nullptr, _pixels.data(), LCD_WIDTH * PIXEL_SIZE);
    SDL_RenderClear(_renderer);
    SDL_RenderTexture(_renderer, _texture, nullptr, nullptr);
    SDL_RenderPresent(_renderer);
  }

private:
  SDL_Window *_window;
  SDL_Renderer *_renderer;
  SDL_Texture *_texture;
  std::vector<std::uint8_t> _pixels;
};
