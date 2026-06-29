#pragma once

#include <string>

namespace mr {

class ResourceManager;

template <typename T>
class ResourceHandle {
  std::string m_id;
  ResourceManager* m_manager;

public:
  ResourceHandle() : m_manager(nullptr) {}
  ResourceHandle(const std::string& id, ResourceManager* manager) : m_id(id), m_manager(manager) {}

  T* get() const;
  bool isValid() const;
  T* operator->() const;
  T& operator*() const;
  operator bool() const;
};

}  // namespace mr
