#include "ServiceLocator.h"

#include "Renderer.h"
#include "ResourceManager.h"

Renderer* ServiceLocator::m_renderer = nullptr;
ResourceManager* ServiceLocator::m_resourceManager = nullptr;

void ServiceLocator::provideRenderer(Renderer* renderer) {
  if (renderer != nullptr)
    m_renderer = renderer;
}

void ServiceLocator::provideResourceManager(ResourceManager* resourceManager) {
  if (resourceManager != nullptr)
    m_resourceManager = resourceManager;
}

void ServiceLocator::provideEntityRegistry(EntityRegistry* entityRegistry) {
  if (entityRegistry != nullptr)
    m_entityRegistry = entityRegistry;
}

Renderer& ServiceLocator::getRenderer() {
  return *m_renderer;
}

ResourceManager& ServiceLocator::getResourceManager() {
  return *m_resourceManager;
}

EntityRegistry& ServiceLocator::getEntityRegistry() {
  return *m_entityRegistry;
}
