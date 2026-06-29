#pragma once

#include "Default.h"
#include "Messenger.h"
#include "Platform/Platform.h"
#include "Renderer/Renderer.h"
#include "ResourceManager/ResourceManager.h"

namespace mr {

class ServiceLocator {
  static mr::Renderer* m_renderer;
  static mr::AsyncResourceManager* m_resourceManager;
  static mr::Platform* m_platform;
  static mr::Messenger* m_messenger;
  static Settings* m_settings;

public:
  static void provideRenderer(mr::Renderer* renderer);
  static void provideResourceManager(mr::AsyncResourceManager* resourceManager);
  static void providePlatform(mr::Platform* platform);
  static void provideSettings(Settings* settings);
  static void provideMessenger(Messenger* messenger);

  static mr::Renderer& getRenderer();
  static mr::AsyncResourceManager& getResourceManager();
  static mr::Platform* getPlatform();
  static mr::Messenger* getMessenger();
  static Settings* getSettings();
};

}  // namespace mr
