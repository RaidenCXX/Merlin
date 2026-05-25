#include "EntityRegistry.h"

#include "Component.h"
#include "Entity.h"

#include <cstdint>
#include <vector>

uint64_t EntityRegistry::createEntity(const std::string& name, EntityTag tag) {
  Entity e{this};
  std::vector<CTag>& tags = std::get<std::vector<CTag>>(m_entityComponents);
  std::vector<CName>& names = std::get<std::vector<CName>>(m_entityComponents);
  if (m_freeEntitiesID.empty()) {
    e.m_id = m_entities.size();

    tags.emplace_back(tag);
    names.emplace_back(name);

    e.setBit((uint64_t)ComponentBit::CName | (uint64_t)ComponentBit::CTag);

    m_entities.emplace_back(e);
  } else {
    e.m_id = m_freeEntitiesID.front();
    m_freeEntitiesID.pop();

    names[e.m_id] = CName{name};
    tags[e.m_id] = CTag{};

    e.setBit((uint64_t)ComponentBit::CName | (uint64_t)ComponentBit::CTag);
  }

  m_nameToID[name] = e.m_id;

  return m_entities[e.m_id].m_id;
}

void EntityRegistry::deleteEntity(const ::std::string& name) {
  auto it = m_nameToID.find(name);
  if (it == m_nameToID.end())
    return;

  std::vector<CTag>& tagVector = std::get<std::vector<CTag>>(m_entityComponents);
  std::vector<CName>& namesVector = std::get<std::vector<CName>>(m_entityComponents);

  uint32_t id = it->second;

  m_entities[id].m_mask = 0;
  m_freeEntitiesID.push(id);
  m_nameToID.erase(it);
}

Entity* EntityRegistry::getEntity(const std::string& name) {
  auto it = m_nameToID.find(name);
  if (it == m_nameToID.end())
    return nullptr;

  return &m_entities[it->second];
}

Entity* EntityRegistry::getEntity(uint64_t id) {
  return &m_entities[id];
}
