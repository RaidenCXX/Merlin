#pragma once

#include "EntityRegistry.h"
#include "Platform.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "SceneManager.h"

#include <memory>

class Merlin {
  std::unique_ptr<Platform> m_platform;
  std::unique_ptr<ResourceManager> m_resourceManager;
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<SceneManager> m_sceneManager;
  std::unique_ptr<EntityRegistry> m_entityRegistry;

  void init();

public:
  Merlin();

  void run();
};
