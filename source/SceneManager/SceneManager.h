#pragma once

#include "Scene.h"

#include <vector>

namespace mr {

class SceneManager {
  std::vector<mr::Scene> m_scenes;

public:
  void init();
};

}  // namespace mr
