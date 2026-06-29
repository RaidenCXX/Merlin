#include "Messenger.h"

#include <mutex>

void mr::Messenger::stop() {
  m_running = false;
  m_cv.notify_one();
  if (m_workerThread.joinable())
    m_workerThread.join();
};

void mr::Messenger::sendMessage(const std::string& message) {
#ifndef NDEBUG
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messageQueue.push(message);
  }
  m_cv.notify_one();
#endif
}

void mr::Messenger::sendMessage(const std::string& messageType, const std::string& location,
                                const std::string& message) {
#ifndef NDEBUG
  std::string ms = '[' + messageType + "](" + location + "){" + message + '}';

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messageQueue.push(ms);
  }
  m_cv.notify_one();
#endif
}

void mr::Messenger::work() {
  while (m_running) {
    std::string ms;
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this]() { return !m_messageQueue.empty() || !m_running; });

      if (!m_running && m_messageQueue.empty())
        return;

      ms = std::move(m_messageQueue.front());
      m_messageQueue.pop();
    }
    std::cout << ms << std::endl;
  }
}
