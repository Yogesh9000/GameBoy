#pragma once

#include "pixel.hpp"

struct Display
{
  Display(const Display &) = default;
  Display &operator=(const Display &) = default;
  Display(Display &&) = default;
  Display &operator=(Display &&) = default;
  virtual ~Display() = default;
  Display() = default;

  virtual void Draw(const Pixel &) = 0;
  virtual void UpdateFrame() = 0;
};
