#pragma once
#include "Component.h"
#include "Entity.h"

#include <cstdint>
#include <iostream>
#include <queue>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using EntityComponentVectorTuple =
  std::tuple<std::vector<CTag>, std::vector<CTransform>, std::vector<CName>>;

using CameraComponentVectorTuple = std::tuple<std::vector<CCamera>, std::vector<CTransform>>;

class EntityRegistry {
  std::vector<Entity> m_entities;
  std::unordered_map<std::string, uint32_t> m_nameToID;
  std::queue<uint32_t> m_freeEntitiesID;
  EntityComponentVectorTuple m_entityComponents;
  std::vector<Camera> m_cameras;
  std::queue<uint32_t> m_freeCamerasID;
  CameraComponentVectorTuple m_cameraComponents;

public:
  uint64_t createEntity(const std::string& name, EntityTag tag = EntityTag::None);
  void deleteEntity(const ::std::string& name);
  Entity* getEntity(const std::string& name);
  Entity* getEntity(uint64_t id);

  template <typename E, typename T, typename... Args>
  T* addComponent(uint32_t id, Args&&... args) {
    std::vector<T>* pool = nullptr;

    if constexpr (std::is_same_v<E, Entity>) {
      pool = &std::get<std::vector<T>>(m_entityComponents);
      m_entities[id].addBit(T::bit);
    } else if constexpr (std::is_same_v<E, Camera>) {
      pool = &std::get<std::vector<T>>(m_cameraComponents);
      m_cameras[id].addBit(T::bit);
    }

    if (id >= pool->size()) {
      pool->resize(id + 1);
    }

    (*pool)[id] = T{std::forward<Args>(args)...};

    return &(*pool)[id];
  }

  template <typename E, typename T>
  void removeComponent(uint32_t id) {
    std::vector<T>* pool = nullptr;

    if constexpr (std::is_same_v<E, Entity>) {
      pool = &std::get<std::vector<T>>(m_entityComponents);
      m_entities[id].removeBit(T::bit);
    } else if constexpr (std::is_same_v<E, Camera>) {
      pool = &std::get<std::vector<T>>(m_cameraComponents);
      m_cameras[id].removeBit(T::bit);
    }

    if (id >= pool->size())
      return;
    (*pool)[id] = T{};
  }

  template <typename E, typename T>
  bool hasComponent(uint32_t id) {
    bool status = false;
    if constexpr (std::is_same_v<E, Entity>) {
      status = m_entities[id].checkBit(T::bit);
    } else if constexpr (std::is_same_v<E, Camera>) {
      status = m_cameras[id].checkBit(T::bit);
    }

    return status;
  }

  template <typename E, typename T>
  T* getComponent(uint64_t id) {
    if (!hasComponent<E, T>(id))
      return nullptr;

    std::vector<T>* pool = nullptr;

    if constexpr (std::is_same_v<E, Entity>) {
      pool = &std::get<std::vector<T>>(m_entityComponents);
    } else if constexpr (std::is_same_v<E, Camera>) {
      pool = &std::get<std::vector<T>>(m_cameraComponents);
    }

    return &(*pool)[id];
  }
};

// Entity template metods definition
template <typename T, typename... Args>
T* Entity::addComponent(Args&&... args) {
  return m_registry->addComponent<Entity, T>(m_id, std::forward<Args>(args)...);
}

template <typename T>
void Entity::removeComponent() {
  m_registry->removeComponent<Entity, T>(m_id);
}

template <typename T>
bool Entity::hasComponent() {
  return m_registry->hasComponent<Entity, T>(m_id);
}

template <typename T>
T& Entity::getComponent() {
  return *(m_registry->getComponent<Entity, T>(m_id));
}

// Camera template medods definition
template <typename T, typename... Args>
T* Camera::addComponent(Args&&... args) {
  return m_registry->addComponent<Camera, T>(m_id, std::forward<Args>(args)...);
}

template <typename T>
void Camera::removeComponent() {
  m_registry->removeComponent<Camera, T>(m_id);
}

template <typename T>
bool Camera::hasComponent() {
  return m_registry->hasComponent<Camera, T>(m_id);
}
