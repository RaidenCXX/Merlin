#pragma once

#include "Config.h"
#include "Renderer/Renderer.h"
#include "WindowSystem/WindowSystem.h"

namespace mr {

class ServiceLocator {
  static mr::IRenderer* m_renderer;
  static mr::IWindowSystem* m_windowSystem;
  static mr::EngineConfig* m_engineConfig;

public:
  static void provideRenderer(mr::IRenderer* renderer);
  static void provideWindowSystem(mr::IWindowSystem* windowSystem);
  static void provideEngineConfig(mr::EngineConfig* engineConfig);

  static mr::IRenderer* getRenderer();
  static mr::IWindowSystem* getWindowSystem();
  static mr::EngineConfig* getEngineConfig();
};

}  // namespace mr
