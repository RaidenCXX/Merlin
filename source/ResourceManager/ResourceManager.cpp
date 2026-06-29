#include "ResourceManager.h"

#include "../ServiceLocator.h"
#include "RMServiceLocator.h"
#include "SlangCompiler.h"

#include <memory>

Settings* mr::ResourceManager::getSettings() {
  return mr::ServiceLocator::getSettings();
}

void mr::AsyncResourceManager::init() {
  GraphicsAPI api = ServiceLocator::getSettings()->m_graphicsApi;
  if (api == GraphicsAPI::Vulkan) {
    m_slangCompiler = std::make_unique<SlangCompiler>();
    mr::RMServiceLocator::provideSlangCompiler(m_slangCompiler.get());

    m_slangCompiler->init();
  }

  start();
}

void mr::AsyncResourceManager::start() {
  m_running = true;
  m_workerThread = std::thread([this]() { workerThread(); });
}

void mr::AsyncResourceManager::stop() {
  GraphicsAPI api = ServiceLocator::getSettings()->m_graphicsApi;
  if (api == GraphicsAPI::Vulkan)
    m_slangCompiler->shutdown();

  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_running = false;
  }
  m_condition.notify_one();
  if (m_workerThread.joinable()) {
    m_workerThread.join();
  }
}

void mr::AsyncResourceManager::workerThread() {
  while (m_running) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(m_queueMutex);
      m_condition.wait(lock, [this]() { return !m_taskQueue.empty() || !m_running; });

      if (!m_running && m_taskQueue.empty()) {
        return;
      }

      task = std::move(m_taskQueue.front());
      m_taskQueue.pop();
    }

    task();
  }
}
