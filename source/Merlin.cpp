#include "Merlin.h"

#include "Messenger.h"
#include "Platform/Platform.h"
#include "Renderer/Renderer.h"
#include "ResourceManager/ResourceManager.h"
#include "ServiceLocator.h"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>

void setDefaultSettings(Settings& settings);

Merlin::Merlin() {
  init();
}

void Merlin::init() {
  setDefaultSettings(m_settings);

  m_platform = std::make_unique<mr::GLFWPlatform>();
  m_renderer = std::make_unique<mr::VkRenderer>();
  m_resourceManager = std::make_unique<mr::AsyncResourceManager>();
  m_sceneManager = std::make_unique<mr::SceneManager>();
  m_messenger = std::make_unique<mr::Messenger>();

  mr::ServiceLocator::provideRenderer(m_renderer.get());
  mr::ServiceLocator::provideResourceManager(m_resourceManager.get());
  mr::ServiceLocator::providePlatform(m_platform.get());
  mr::ServiceLocator::provideSettings(&m_settings);
  mr::ServiceLocator::provideMessenger(m_messenger.get());

  // Init
  m_messenger->init();
  m_platform->init("Merlin", 1000, 1000);
  m_resourceManager->init();
  m_sceneManager->init();
  m_renderer->init();

  m_resourceManager->loadResource<mr::VkShader>("default", "shaders/slang/default.slang",
                                                VK_SHADER_STAGE_VERTEX_BIT
                                                  | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Merlin::run() {
  while (!m_platform->windowShouldClose()) {}
}

void setDefaultSettings(Settings& settings) {
  settings.graphicsApi = mr::GraphicsAPI::Vulkan;
  settings.platform = mr::PlatformType::GLFWPlatform;

  settings.systemPaths.shadersFolderPath.push_back("shaders/slang");
}
