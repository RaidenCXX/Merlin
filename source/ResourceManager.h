#pragma once

#include "resource/Resource.h"
#include "resource/ResourceHandle.h"

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

class ResourceManager {
  struct ResourceData {
    std::shared_ptr<Resource> resource;
    uint16_t refCount = 0;
  };

  std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>>
    m_resources;
  std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> m_refCounts;

public:
  template <typename T, typename... Args>
  ResourceHandle<T> load(const std::string& resourceId, Args&&... args) {
    // Check if resource derive from Resource
    static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

    // Add the right resource map and find resource
    std::unordered_map<std::string, std::shared_ptr<Resource>>& resourceMap =
      m_resources[std::type_index(typeid(T))];
    auto it = resourceMap.find(resourceId);

    // Add the right refCount map
    std::unordered_map<std::string, ResourceData>& refCountsMap =
      m_refCounts[std::type_index(typeid(T))];

    // If the resource already exists
    if (it != resourceMap.end()) {
      // Add ResourceData to map and increase counter
      refCountsMap[resourceId].refCount++;
      return ResourceHandle<T>(resourceId, this);
    }

    // Create resource
    std::shared_ptr<Resource> resource =
      std::make_shared<T>(resourceId, std::forward<Args>(args)...);

    // if the resource loading is failed
    if (!resource->load()) {
      return ResourceHandle<T>();
    }

    // If the resource was successfully loaded
    resourceMap[resourceId] = resource;
    refCountsMap[resourceId].refCount = 1;
    return ResourceHandle<T>(resourceId, this);
  }

  template <typename T>
  T* get(const std::string& resourceId) {
    // Find the right map!
    auto it = m_resources.find(std::type_index(typeid(T)));
    if (it == m_resources.end())
      return nullptr;

    // Find resource!
    auto resIt = it->second.find(resourceId);
    if (resIt == it->second.end())
      return nullptr;

    // Return a raw pointer to the resource
    return static_cast<T*>(resIt->second.get());
  }

  template <typename T>
  void release(const std::string& resourceId) {
    // Find the right map!
    auto it = m_resources.find(std::type_index(typeid(T)));
    if (it == m_resources.end())
      return;

    // Find resource!
    auto resIt = it->second.find(resourceId);
    if (resIt == it->second.end())
      return;

    // Find the right refCount map
    auto refCountMapIt = m_refCounts.find(std::type_index(typeid(T)));
    if (refCountMapIt == m_refCounts.end())
      return;

    // Find ResourceData!
    auto ResourceDataIt = refCountMapIt->second.find(resourceId);
    if (ResourceDataIt == refCountMapIt->second.end())
      return;

    // Decrease counter
    ResourceDataIt->second.refCount--;
    if (ResourceDataIt->second.refCount <= 0) {
      // Unload resource
      resIt->second->unload();
      it->second.erase(resIt);
      refCountMapIt->second.erase(ResourceDataIt);
    }
  }

  template <typename T>
  bool hasResource(const std::string& id) {
    // Find the right map!
    auto it = m_resources.find(std::type_index(typeid(T)));
    if (it == m_resources.end())
      return false;
    // Check if right resource map and resource exists
    return it->second.find(id) != it->second.end();
  }

  void unloadAll() {
    for (auto& [key, typeResources] : m_resources) {
      for (auto& [id, resource] : typeResources) {
        resource->unload();
      }
    }
    m_refCounts.clear();
  }
};

//****ResourceHandle definition****
template <typename T>
T* ResourceHandle<T>::get() const {
  if (!m_manager)
    return nullptr;
  return m_manager->get<T>(m_id);
}

template <typename T>
bool ResourceHandle<T>::isValid() const {
  return m_manager && m_manager->hasResource<T>(m_id);
}

template <typename T>
T* ResourceHandle<T>::operator->() const {
  return get();
}
template <typename T>
T& ResourceHandle<T>::operator*() const {
  return *get();
}
template <typename T>
ResourceHandle<T>::operator bool() const {
  return isValid();
}
