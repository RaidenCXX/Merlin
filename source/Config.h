#pragma once

#include "Renderer/Renderer.h"
#include "WindowSystem/WindowSystem.h"

#include <cstdint>

namespace mr {

struct EngineConfig {
  GraphicsAPI graphicsApi = GraphicsAPI_None;
  WindowSystemType windowSystem = WindowSystem_None;
  uint32_t windowWidth = 1920;
  uint32_t windowHeight = 1080;
  uint64_t gFrame = 0;
};

}  // namespace mr
