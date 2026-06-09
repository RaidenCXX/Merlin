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
  bool doLoad() override;
  bool doUnload() override;
};

class Mesh : public Resource {
public:
  Mesh(const std::string& id, const std::string& path);
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
  explicit VkMesh(const std::string& id, const std::string& path) : Mesh(id, path) {}

  ~VkMesh() override { unload(); }
};

class Shader : public Resource {
  vk::ShaderModule m_shaderModule;
  vk::ShaderStageFlagBits m_stage;

public:
  Shader(const std::string& id, const std::string& path, vk::ShaderStageFlagBits shaderStage);

  ~Shader() override { unload(); }

  bool doLoad() override {
    // Determine file extension based on shader stage
    std::string extension;
    switch (m_stage) {
      case vk::ShaderStageFlagBits::eVertex:
        extension = ".vert";
        break;
      case vk::ShaderStageFlagBits::eFragment:
        extension = ".frag";
        break;
      case vk::ShaderStageFlagBits::eCompute:
        extension = ".comp";
        break;
      default:
        return false;
    }

    // Load shader from file
    std::string filePath = "shaders/" + getName() + "." + getExt();

    // Read shader code
    std::vector<char> shaderCode;
    if (!ReadFile(filePath, shaderCode)) {
      return false;
    }

    // Create shader module
    CreateShaderModule(shaderCode);

    return Resource::load();
  }

  bool doUnload() override {
    // Destroy Vulkan resources
    if (isLoaded()) {
      // Get device from somewhere (e.g., singleton or parameter)
      vk::Device device = GetDevice();

      device.destroyShaderModule(m_shaderModule);

      Resource::unload();
      return true;
    }
    return false;
  }

  // Getters for Vulkan resources
  vk::ShaderModule GetShaderModule() const { return m_shaderModule; }
  vk::ShaderStageFlagBits GetStage() const { return m_stage; }

private:
  bool ReadFile(const std::string& filePath, std::vector<char>& buffer) {
    // Implementation to read binary file
    // ...
    return true;  // Placeholder
  }

  void CreateShaderModule(const std::vector<char>& code) {
    // Implementation to create Vulkan shader module
    // ...
  }

  vk::Device GetDevice() {
    // Get device from somewhere (e.g., singleton or parameter)
    // ...
    return vk::Device();  // Placeholder
  }
};
