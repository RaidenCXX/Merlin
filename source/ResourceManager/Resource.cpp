#include "Resource.h"

#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string>

// Resource
mr::Resource::Resource(const std::string& id, const std::string& path)
    : m_resourceId(id), m_path(path) {}

const std::string mr::Resource::getExt() {
  size_t posDot = m_path.rfind('.');
  if (posDot == std::string::npos)
    return {};

  return std::string(m_path.begin() + posDot + 1, m_path.end());
}

const std::string mr::Resource::getName() {
  size_t dotPos = m_path.rfind('.');
  size_t slashPos = m_path.rfind('/');

  if (dotPos == std::string::npos) {
    return std::string{};
  }

  size_t start = (slashPos == std::string::npos) ? 0 : slashPos + 1;
  return std::string{m_path.begin() + slashPos, m_path.begin() + dotPos};
}

const std::string mr::Resource::getPath() {
  size_t pos = m_path.rfind('/');
  if (pos == std::string::npos)
    return std::string{};

  return std::string{m_path.begin(), m_path.begin() + (pos + 1)};
}

// Texture
unsigned char* mr::Texture::LoadImageData(const std::string& filePath, uint32_t& width,
                                          uint32_t& height, uint8_t& channels) {
  unsigned char* data =
    stbi_load(filePath.c_str(), (int*)&width, (int*)&height, (int*)&channels, 0);
  return data;
}

void mr::Texture::FreeImageData(unsigned char* data) {
  stbi_image_free(data);
}

// Mesh
mr::Mesh::Mesh(const std::string& id, const std::string& path) : Resource(id, path) {}

// Shader
mr::Shader::Shader(const std::string& id, const std::string& path) : Resource(id, path) {}
