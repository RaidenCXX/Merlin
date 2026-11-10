#include "Merlin.h"

#include "Config.h"
#include "Renderer/Renderer.h"
#include "ServiceLocator.h"
#include "WindowSystem/WindowSystem.h"
#include "minilog/minilog.h"

#include <vulkan/vulkan_core.h>

#include <iostream>
#include <memory>
#include <ostream>
#include <string>

static void setConfig(mr::EngineConfig& config);

mr::Merlin::Merlin() {
  init();
}

void mr::Merlin::init() {
  minilog::initialize(nullptr, {.threadNames = false});

  setConfig(config_);

  switch (config_.windowSystem) {
    case mr::WindowSystem_GLFW:
      windowSystem_ = std::make_unique<mr::GLFWWindowSystem>();
      break;
    default:
      break;
  }
  mr::ServiceLocator::provideWindowSystem(windowSystem_.get());

  switch (config_.graphicsApi) {
    case GraphicsAPI_Vulkan:
      renderer_ = std::make_unique<VkRenderer>();
      break;
    default:
      break;
  }

  mr::ServiceLocator::provideRenderer(renderer_.get());

  windowSystem_->init("Merlin", config_.windowWidth, config_.windowHeight);
  LLOGL("Platform success init");
  renderer_->init();
  LLOGL("Renderer success init");
}

void mr::Merlin::run() {
  while (!windowSystem_->windowShouldClose()) {
    windowSystem_->pollEvents();
    renderer_->drawFrame();
  }

  renderer_->shutdown();
  windowSystem_->shutdown();
}

static void setConfig(mr::EngineConfig& config) {
  LLOGL("Config");
  config.windowWidth = 1920;
  config.windowHeight = 1080;
  LLOGL("    Window size w %i : h %i ", config.windowWidth, config.windowHeight);

#if defined(LINUX) || defined(WINDOWS)
  config.graphicsApi = mr::GraphicsAPI_Vulkan;
#endif

  switch (config.graphicsApi) {
    case mr::GraphicsAPI_Vulkan:
      LLOGL("    GraphicsAPI: Vulkan");
      break;
    case mr::GraphicsAPI_Metal:
      LLOGL("    GraphicsAPI: Metal");
      break;
    case mr::GraphicsAPI_None:
      break;
  }

#if defined(LINUX) | defined(WINDOWS)
  config.windowSystem = mr::WindowSystem_GLFW;
#endif

  switch (config.windowSystem) {
    case mr::WindowSystem_GLFW:
      LLOGL("    WindowSystem: GLFW");
      break;
    case mr::WindowSystem_SDL3:
      LLOGL("    WindowSystem: SDL3");
      break;
    default:
      LLOGL("    WindowSystem: None");
  }
};
