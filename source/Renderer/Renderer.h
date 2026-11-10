#pragma once
#include "VulkanBackend/MRVK/MRVKClasses.h"

#include <cstdint>
#include <unordered_map>

namespace mr {

enum GraphicsAPI { GraphicsAPI_None = 0, GraphicsAPI_Vulkan = 1, GraphicsAPI_Metal = 2 };

class IRenderer {
protected:
  uint64_t renderFrame = 0;

public:
  virtual ~IRenderer() = default;

  virtual void init() = 0;
  virtual void shutdown() = 0;
  virtual void drawFrame() = 0;
};

// Vulkan
class VkRenderer : public IRenderer {
  mrvk::VulkanContext ctx_;

  std::unordered_map<const char*, mrvk::Holder<mrvk::ShaderModuleHandle>> shadersModules_;
  std::unordered_map<const char*, mrvk::Holder<mrvk::RenderPipelineHandle>> renderPipelines_;

public:
  VkRenderer() = default;
  virtual ~VkRenderer() { shutdown(); }

  virtual void init() override;
  virtual void drawFrame() override;
  virtual void shutdown() override;
};

}  // namespace mr
