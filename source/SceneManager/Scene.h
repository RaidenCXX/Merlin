#pragma once
#include "EntityRegistry.h"

#include <memory>

namespace mr {

class Scene {
  std::unique_ptr<mr::EntityRegistry> m_entityRegistry;

  void init();
};

}  // namespace mr
