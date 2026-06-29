#pragma once

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace mr {

class Messenger {
  std::thread m_workerThread;
  std::condition_variable m_cv;
  std::mutex m_mutex;
  std::queue<std::string> m_messageQueue;
  std::atomic_bool m_running = false;

public:
  ~Messenger() { stop(); }

  void init() { start(); };
  void start() {
    m_running = true;
    m_workerThread = std::thread([this]() { work(); });
  }
  void stop();
  void sendMessage(const std::string& message);
  void sendMessage(const std::string& messageType, const std::string& location,
                   const std::string& message);

private:
  void work();
};
}  // namespace mr
