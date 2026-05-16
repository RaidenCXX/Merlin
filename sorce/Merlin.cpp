#include "Merlin.h"

#include "ServiceLocator.h"

#include <memory>

Merlin::Merlin() {
  init();
}

void Merlin::init() {
  m_platform = std::make_unique<Platform>();
  m_renderer = std::make_unique<Renderer>();
  m_resourceManager = std::make_unique<ResourceManager>();
  m_sceneManager = std::make_unique<SceneManager>();
  m_entityRegistry = std::make_unique<EntityRegistry>();

  ServiceLocator::provideRenderer(m_renderer.get());
  ServiceLocator::provideResourceManager(m_resourceManager.get());
  ServiceLocator::provideEntityRegistry(m_entityRegistry.get());
}
