#pragma once

#include "EntityRegistry.h"
#include "Platform.h"
#include "Renderer.h"
#include "ResourceManager.h"

#include <memory>
class ServiceLocator {
  static Renderer* m_renderer;
  static AsyncResourceManager* m_resourceManager;
  static EntityRegistry* m_entityRegistry;
  static Platform* m_platform;

public:
  static void provideRenderer(Renderer* renderer);
  static void provideResourceManager(AsyncResourceManager* resourceManager);
  static void provideEntityRegistry(EntityRegistry* entityRegistry);
  static void providePlatform(Platform* platform);

  static Renderer& getRenderer();
  static AsyncResourceManager& getResourceManager();
  static EntityRegistry& getEntityRegistry();

  static Platform* getPlatform();
};
