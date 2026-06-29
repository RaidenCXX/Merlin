#include "RMServiceLocator.h"
#include "Resource.h"
#include "SlangCompiler.h"
#include "vulkan/vulkan_core.h"

/*
Vulkan texures
*/
bool mr::VkTexture::createVulkanImage(unsigned char* data, uint32_t width, uint32_t height,
                                      uint8_t channels) {
  return true;
}

bool mr::VkTexture::doLoad() {
  std::string filePath = "texture/" + getName() + "." + getExt();

  uint32_t width, height;
  uint8_t channels;

  unsigned char* imgData = LoadImageData(filePath, width, height, channels);

  setWidth(width);
  setHeight(height);
  setChannels(channels);

  createVulkanImage(imgData, getWidth(), getHeight(), getChannels());

  FreeImageData(imgData);

  return Resource::load();
}

bool mr::VkTexture::doUnload() {
  // vk::Device device = getDevice();
  return true;
}

/*
Vulkan Mesh
*/
mr::VkMesh::VkMesh(const std::string& id, const std::string& path) : Mesh(id, path) {}

bool mr::VkMesh::doLoad() {
  return true;
}

bool mr::VkMesh::doUnload() {
  return true;
}

/*
Vulkan Shader
*/
mr::VkShader::VkShader(const std::string& id, const std::string& path,
                       VkShaderStageFlagBits shaderStage)
    : Shader(id, path), m_stage(shaderStage) {}

bool mr::VkShader::doLoad() {
  std::string entryPoint;
  switch (m_stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
      entryPoint = "vertexMain";
      break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      entryPoint = "fragmentMain";
      break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
      entryPoint = "computeMain";
      break;
    default:
      return false;
  }

  SlangCompiler* compailer = mr::RMServiceLocator::getSlangCompiler();
  if (compailer == nullptr)
    return false;

  std::vector<char> spirvCode;
  if (!compailer->compile(getPath(), entryPoint, spirvCode))
    return false;

  return createShaderModule(spirvCode);
}

bool mr::VkShader::doUnload() {
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

VkShaderModule mr::VkShader::createShaderModule(std::vector<char>& spirvCode) {}
