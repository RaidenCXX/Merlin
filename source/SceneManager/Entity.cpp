#include "Entity.h"

mr::Entity::Entity(EntityRegistry* registry) : m_registry(registry) {}

mr::Entity::Entity(uint32_t id, EntityRegistry* registry) : m_id(id), m_registry(registry) {}

mr::Entity::Entity(const Entity& e) : m_id(e.m_id), m_registry(e.m_registry), m_mask(e.m_mask) {}
