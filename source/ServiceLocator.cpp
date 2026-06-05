#include "ServiceLocator.h"

#include "EntityRegistry.h"
#include "Platform.h"
#include "Renderer.h"
#include "ResourceManager.h"

Renderer* ServiceLocator::m_renderer = nullptr;
AsyncResourceManager* ServiceLocator::m_resourceManager = nullptr;
EntityRegistry* ServiceLocator::m_entityRegistry = nullptr;
Platform* ServiceLocator::m_platform = nullptr;

void ServiceLocator::provideRenderer(Renderer* renderer) {
  if (renderer != nullptr)
    m_renderer = renderer;
}

void ServiceLocator::provideResourceManager(AsyncResourceManager* resourceManager) {
  if (resourceManager != nullptr)
    m_resourceManager = resourceManager;
}

void ServiceLocator::provideEntityRegistry(EntityRegistry* entityRegistry) {
  if (entityRegistry != nullptr)
    m_entityRegistry = entityRegistry;
}

void ServiceLocator::providePlatform(Platform* platform) {
  if (platform != nullptr)
    m_platform = platform;
}

Renderer& ServiceLocator::getRenderer() {
  return *m_renderer;
}

AsyncResourceManager& ServiceLocator::getResourceManager() {
  return *m_resourceManager;
}

EntityRegistry& ServiceLocator::getEntityRegistry() {
  return *m_entityRegistry;
}
