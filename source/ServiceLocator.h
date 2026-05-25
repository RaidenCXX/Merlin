#pragma once

#include "EntityRegistry.h"
#include "Renderer.h"
#include "ResourceManager.h"

#include <memory>
class ServiceLocator {
  static Renderer* m_renderer;
  static ResourceManager* m_resourceManager;
  static EntityRegistry* m_entityRegistry;

public:
  static void provideRenderer(Renderer* renderer);
  static void provideResourceManager(ResourceManager* resourceManager);
  static void provideEntityRegistry(EntityRegistry* entityRegistry);

  static Renderer& getRenderer();
  static ResourceManager& getResourceManager();
  static EntityRegistry& getEntityRegistry();
};
