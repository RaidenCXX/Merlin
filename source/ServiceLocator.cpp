#include "ServiceLocator.h"

#include "Messenger.h"
#include "Platform/Platform.h"
#include "Renderer/Renderer.h"
#include "ResourceManager/ResourceManager.h"

mr::Renderer* mr::ServiceLocator::m_renderer = nullptr;
mr::AsyncResourceManager* mr::ServiceLocator::m_resourceManager = nullptr;
mr::Platform* mr::ServiceLocator::m_platform = nullptr;
mr::Messenger* mr::ServiceLocator::m_messenger = nullptr;
Settings* mr::ServiceLocator::m_settings = nullptr;

void mr::ServiceLocator::provideRenderer(mr::Renderer* renderer) {
  if (renderer != nullptr)
    m_renderer = renderer;
}

void mr::ServiceLocator::provideResourceManager(mr::AsyncResourceManager* resourceManager) {
  if (resourceManager != nullptr)
    m_resourceManager = resourceManager;
}

void mr::ServiceLocator::providePlatform(mr::Platform* platform) {
  if (platform != nullptr)
    m_platform = platform;
}

void mr::ServiceLocator::provideMessenger(Messenger* messenger) {
  if (messenger != nullptr)
    m_messenger = messenger;
}

void mr::ServiceLocator::provideSettings(Settings* settings) {
  if (settings != nullptr)
    m_settings = settings;
}

mr::Renderer& mr::ServiceLocator::getRenderer() {
  return *m_renderer;
}

mr::AsyncResourceManager& mr::ServiceLocator::getResourceManager() {
  return *m_resourceManager;
}

mr::Platform* mr::ServiceLocator::getPlatform() {
  return m_platform;
}

Settings* mr::ServiceLocator::getSettings() {
  return m_settings;
}
