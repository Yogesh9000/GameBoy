#pragma once

class PpuPhase
{
public:
  virtual ~PpuPhase() = default;
  PpuPhase() = default;
  PpuPhase(const PpuPhase &) = default;
  PpuPhase(PpuPhase &&) = default;
  PpuPhase &operator=(const PpuPhase &) = default;
  PpuPhase &operator=(PpuPhase &&) = default;

  virtual void Start() = 0;

  [[nodiscard]]
  virtual bool Tick() = 0;
};
