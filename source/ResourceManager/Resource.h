#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <string>
#include <vector>

namespace mr {

/*
Resource
*/
class Resource {
  std::string m_path;
  std::string m_resourceId;
  bool m_loaded = false;

public:
  explicit Resource(const std::string& id, const std::string& path);
  virtual ~Resource() = default;

  const std::string& getId() const { return m_resourceId; }
  const std::string getExt();
  const std::string getName();
  const std::string getPath();

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

/*
Textures
*/
class Texture : public Resource {
  // Texture metadata for validation and debugging
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  uint8_t m_channels = 0;

public:
  Texture(const std::string& id, const std::string& path) : Resource(id, path) {}
  unsigned char* LoadImageData(const std::string& filePath, uint32_t& width, uint32_t& height,
                               uint8_t& channels);
  void FreeImageData(unsigned char* data);

  void setWidth(uint32_t width) { m_width = width; }
  void setHeight(uint32_t height) { m_width = height; }
  void setChannels(uint8_t channels) { m_width = channels; }

  uint32_t getWidth() const { return m_width; }
  uint32_t getHeight() const { return m_height; }
  uint8_t getChannels() const { return m_channels; }

  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;
};

class VkTexture : public Texture {
  vk::Image m_image;          // GPU image object containing pixel data
  vk::DeviceMemory m_memory;  // GPU memory allocation backing the image
  vk::DeviceSize m_offset;    // Offset within the memory allocation for this texture
  vk::ImageView m_imageView;  // Shader-accessible view into the image
  vk::Sampler m_sampler;      // Sampling configuration (filtering, wrapping, etc.)

public:
  explicit VkTexture(const std::string& id, const std::string& path) : Texture(id, path) {}
  ~VkTexture() override { unload(); }
  bool createVulkanImage(unsigned char* data, uint32_t width, uint32_t height, uint8_t channels);

protected:
  virtual bool doLoad() override;
  virtual bool doUnload() override;
};

/*
Meshs
*/
class Mesh : public Resource {
public:
  Mesh(const std::string& id, const std::string& path);

  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;
};

class VkMesh : public Mesh {
  // Vertex data management - stores per-vertex attributes like position, normal, UV coordinates
  vk::Buffer vertexBuffer;              // GPU buffer containing vertex attribute data
  vk::DeviceMemory vertexBufferMemory;  // GPU memory backing the vertex buffer
  vk::DeviceSize vertexBufferOffset;    // Offset within the memory allocation for vertex buffer
  uint32_t vertexCount = 0;             // Number of vertices in this mesh

  // Index data management - defines triangle connectivity using vertex indices
  vk::Buffer indexBuffer;              // GPU buffer containing triangle index data
  vk::DeviceMemory indexBufferMemory;  // GPU memory backing the index buffer
  vk::DeviceSize indexBufferOffset;    // Offset within the memory allocation for index buffer
  uint32_t indexCount = 0;             // Number of indices in this mesh (typically 3 per triangle)

public:
  explicit VkMesh(const std::string& id, const std::string& path);

  ~VkMesh() override { unload(); }

  virtual bool doLoad() override;
  virtual bool doUnload() override;
};

/*
Shaders
*/
class Shader : public Resource {
public:
  Shader(const std::string& id, const std::string& path);

  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;
};

class VkShader : public Shader {
  VkShaderModule m_shaderModule;
  VkShaderStageFlagBits m_stage;

public:
  VkShader(const std::string& id, const std::string& path, VkShaderStageFlagBits shaderStage);

  ~VkShader() override { unload(); }

  virtual bool doLoad() override;
  virtual bool doUnload() override;

  // Getters for Vulkan resources
  VkShaderStageFlagBits GetStage() const { return m_stage; }
  VkShaderModule getModule() const { return m_shaderModule; }

private:
  vk::Device GetDevice();
  VkShaderModule createShaderModule(std::vector<char>& spirvCode);
};
}  // namespace mr
