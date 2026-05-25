#include "Merlin.h"

#include "Component.h"
#include "EntityRegistry.h"
#include "ServiceLocator.h"
#include "glm/glm.hpp"

#include <glm/ext/vector_float3.hpp>
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

void Merlin::run() {
  m_entityRegistry->createEntity("Player", EntityTag::Player);
  Entity* player = m_entityRegistry->getEntity("Player");
  CTransform* cTransform = player->addComponent<CTransform>();
  bool hasTransform = player->hasComponent<CTransform>();
  std::cout << "hasTransform " << hasTransform << "\n";
  cTransform->setPosition(glm::vec3{1.0f, 1.0f, 1.0f});

  std::cout << "Name " << player->getComponent<CName>().m_name << std::endl;
  std::cout << "hasTag " << player->hasComponent<CTag>() << std::endl;

  std::cout << "pos" << player->getComponent<CTransform>().GetPosition().length() << "\n"
            << "HasTag " << static_cast<uint32_t>(player->getComponent<CTag>().m_tag);
}
