#pragma once
#include "vulkan/vulkan.h"

#include <vulkan/vulkan_core.h>

class Renderer {
public:
  virtual bool init() = 0;
};

class VkRenderer : public Renderer {
  VkInstance m_instance;

public:
  virtual bool init() override;
};
