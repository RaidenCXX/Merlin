#pragma once

#include "Default.h"
#include "Messenger.h"
#include "Platform/Platform.h"
#include "Renderer/Renderer.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/SlangCompiler.h"
#include "SceneManager/SceneManager.h"

#include <memory>

class Merlin {
  std::unique_ptr<mr::Platform> m_platform;
  std::unique_ptr<mr::AsyncResourceManager> m_resourceManager;
  std::unique_ptr<mr::Renderer> m_renderer;
  std::unique_ptr<mr::SceneManager> m_sceneManager;
  std::unique_ptr<mr::Messenger> m_messenger;
  std::unique_ptr<mr::SlangCompiler> m_slangCompiler;
  Settings m_settings;

  void init();

public:
  Merlin();

  void run();
};
