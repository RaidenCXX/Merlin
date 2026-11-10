#include "ServiceLocator.h"

#include "Config.h"

mr::IRenderer* mr::ServiceLocator::m_renderer = nullptr;
mr::IWindowSystem* mr::ServiceLocator::m_windowSystem = nullptr;
mr::EngineConfig* mr::ServiceLocator::m_engineConfig = nullptr;

void mr::ServiceLocator::provideRenderer(mr::IRenderer* renderer) {
  if (renderer != nullptr)
    m_renderer = renderer;
}

void mr::ServiceLocator::provideWindowSystem(mr::IWindowSystem* windowSystem) {
  if (windowSystem != nullptr)
    m_windowSystem = windowSystem;
}

void mr::ServiceLocator::provideEngineConfig(mr::EngineConfig* engineConfig) {
  if (engineConfig != nullptr)
    m_engineConfig = engineConfig;
}

mr::IRenderer* mr::ServiceLocator::getRenderer() {
  return m_renderer;
}

mr::IWindowSystem* mr::ServiceLocator::getWindowSystem() {
  return m_windowSystem;
}

mr::EngineConfig* mr::ServiceLocator::getEngineConfig() {
  return m_engineConfig;
}
