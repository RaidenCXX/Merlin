#include "Merlin.h"

#include "Component.h"
#include "EntityRegistry.h"
#include "Platform.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ServiceLocator.h"
#include "resource/Resource.h"
#include "resource/ResourceHandle.h"

#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <memory>

Merlin::Merlin() {
  init();
}

void Merlin::init() {
  m_platform = std::make_unique<GLFWPlatform>();
  m_renderer = std::make_unique<VkRenderer>();
  m_resourceManager = std::make_unique<AsyncResourceManager>();
  m_sceneManager = std::make_unique<SceneManager>();
  m_entityRegistry = std::make_unique<EntityRegistry>();

  ServiceLocator::provideRenderer(m_renderer.get());
  ServiceLocator::provideResourceManager(m_resourceManager.get());
  ServiceLocator::provideEntityRegistry(m_entityRegistry.get());

  m_resourceManager->setAPI(GraphicsAPI::Vulkan);

  uint32_t id = m_entityRegistry->createEntity("player");
  Entity* e = m_entityRegistry->getEntity(id);

  m_resourceManager->LoadResource<Texture>("stone", "textures/stone.png",
                                           [id, this](ResourceHandle<Texture> texHandle) {
                                             Entity* e = this->m_entityRegistry->getEntity(id);
                                             e->addComponent<CTexture>();
                                             e->getComponent<CTexture>().m_texture = texHandle;
                                             std::cout << "Done " << texHandle->getId()
                                                       << std::endl;
                                           });

  if (!m_platform->init("Merlin", 1000, 1000))
    std::cout << "Window creation failure!" << std::endl;
}

void Merlin::run() {
  while (!m_platform->windowShouldClose()) {}
}
