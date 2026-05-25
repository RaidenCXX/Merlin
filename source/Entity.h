#pragma once

#include <cstdint>
#include <vector>

class EntityRegistry;

class Entity {
public:
  uint64_t m_id;
  uint64_t m_mask = 0;

  Entity() = delete;
  Entity(EntityRegistry* registry);
  Entity(uint32_t id, EntityRegistry* registry);
  Entity(const Entity& e);

  template <typename T, typename... Args>
  T* addComponent(Args&&... args);

  template <typename T>
  void removeComponent();

  template <typename T>
  bool hasComponent();

  template <typename T>
  T& getComponent();

  void setBit(uint64_t bit) { m_mask = bit; }

  void addBit(uint64_t bit) { m_mask |= bit; }

  void removeBit(uint64_t bit) { m_mask &= ~bit; }

  bool checkBit(uint64_t bit) const { return m_mask & bit; }

private:
  EntityRegistry* m_registry = nullptr;
};

class Camera {
  EntityRegistry* m_registry = nullptr;

public:
  uint64_t m_id;
  uint64_t m_mask = 0;

  Camera() = delete;
  Camera(EntityRegistry* registry);
  Camera(uint32_t id, EntityRegistry* registry);
  Camera(Camera& e);

  template <typename T, typename... Args>
  T* addComponent(Args&&... args);

  template <typename T>
  void removeComponent();

  template <typename T>
  bool hasComponent();

  void addBit(uint64_t bit) { m_mask |= bit; }

  void removeBit(uint64_t bit) { m_mask &= ~bit; }

  bool checkBit(uint64_t bit) const { return m_mask & bit; }
};
