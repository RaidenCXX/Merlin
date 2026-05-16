#pragma once
#include "Component.h"
#include "Entity.h"

#include <cstdint>
#include <vector>

class EntityRegistry {
  std::vector<Entity> entities;
  std::vector<CTransform> transforms;
  std::vector<CCamera> cameras;

public:
  void addEntity();
};
