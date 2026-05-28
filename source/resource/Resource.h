#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <string>

class Resource {
  std::string m_path;
  std::string m_resourceId;
  bool m_loaded = false;

public:
  explicit Resource(const std::string& id, const std::string& path);
  virtual ~Resource() = default;

  const std::string& getId() const { return m_resourceId; }
  bool isLoaded() const { return m_loaded; }

  bool load() {
    m_loaded = doLoad();
    return m_loaded;
  }

  void unload() {
    doUnload();
    m_loaded = false;
  }

protected:
  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;
};

class TextureVk : public Resource {
  vk::Image m_image;          // GPU image object containing pixel data
  vk::DeviceMemory m_memory;  // GPU memory allocation backing the image
  vk::DeviceSize m_offset;    // Offset within the memory allocation for this texture
  vk::ImageView m_imageView;  // Shader-accessible view into the image
  vk::Sampler m_sampler;      // Sampling configuration (filtering, wrapping, etc.)

  // Texture metadata for validation and debugging
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  uint8_t m_channels = 0;

public:
  explicit TextureVk(const std::string& id, const std::string& path) : Resource(id, path) {}
  ~TextureVk() override { unload(); }
};

class MeshVk : public Resource {};
