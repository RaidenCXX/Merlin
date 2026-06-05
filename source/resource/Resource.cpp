#include "Resource.h"

#include "vulkan/vulkan.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string>

Resource::Resource(const std::string& id, const std::string& path)
    : m_resourceId(id), m_path(path) {}

const std::string Resource::getExt() {
  size_t posDot = m_path.rfind('.');
  if (posDot == std::string::npos)
    return {};

  return std::string(m_path.begin() + posDot + 1, m_path.end());
}

const std::string Resource::getName() {
  size_t dotPos = m_path.rfind('.');
  size_t slashPos = m_path.rfind('/');

  if (dotPos == std::string::npos) {
    return std::string{};
  }

  size_t start = (slashPos == std::string::npos) ? 0 : slashPos + 1;
  return std::string{m_path.begin() + slashPos, m_path.begin() + dotPos};
}

const std::string Resource::getPath() {
  size_t pos = m_path.rfind('/');
  if (pos == std::string::npos)
    return std::string{};

  return std::string{m_path.begin(), m_path.begin() + (pos + 1)};
}

unsigned char* Texture::LoadImageData(const std::string& filePath, uint32_t& width,
                                      uint32_t& height, uint8_t& channels) {
  unsigned char* data =
    stbi_load(filePath.c_str(), (int*)&width, (int*)&height, (int*)&channels, 0);
  return data;
}

void Texture::FreeImageData(unsigned char* data) {
  stbi_image_free(data);
}

bool VkTexture::createVulkanImage(unsigned char* data, uint32_t width, uint32_t height,
                                  uint8_t channels) {
  return true;
}

bool VkTexture::doLoad() {
  std::string filePath = "texture/" + getName() + "." + getExt();

  unsigned char* imgData = LoadImageData(filePath, m_width, m_height, m_channels);

  createVulkanImage(imgData, m_width, m_height, m_channels);

  FreeImageData(imgData);

  return Resource::load();
}

bool VkTexture::doUnload() {
  // vk::Device device = getDevice();
  return true;
}

Shader::Shader(const std::string& id, const std::string& path, vk::ShaderStageFlagBits shaderStage)
    : Resource(id, path), m_stage(shaderStage) {}
