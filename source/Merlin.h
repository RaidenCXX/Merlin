#pragma once

#include "Config.h"
#include "Renderer/Renderer.h"
#include "WindowSystem/WindowSystem.h"

#include <memory>

namespace mr {

class Merlin {
  std::unique_ptr<mr::IWindowSystem> windowSystem_;
  std::unique_ptr<IRenderer> renderer_;
  EngineConfig config_;

  void init();

public:
  Merlin();

  void run();
};

}  // namespace mr
