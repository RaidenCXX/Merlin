#pragma once
//
#include "VulkanStructs.h"
#include "glsl/glslang_c_interface.h"
#include "vulkan/vulkan_core.h"

#include <string>
#include <vector>

namespace mrvk {
class VulkanContext;

VulkanResult getResultFromVkResult(VkResult result);
HWDeviceType convertVulkanDeviceTypeToMRVK(VkPhysicalDeviceType vkDeviceType);
const char* convertValidationFeatureToString(VkValidationFeatureEnableEXT feature);
const char* deviceFeatureToString(DeviceFeature feature);
VkDynamicState convertToVkDynamicState(RenderPipelineDesc::DynamicState state);
VkPipelineColorBlendAttachmentState convertToVkBlendAttachmentState(
  RenderPipelineDesc::BlendAttachment& blendState);
VkSemaphore createSemaphore(VkDevice device, const char* semaphoreName);
VkFence createFence(VkDevice device, const char* fenceName);
VkResult setDebugObjectName(VkDevice device, VkObjectType objectType, uint64_t handle,
                            const char* objectName);
Result compileShader(VkShaderStageFlagBits stage, const char* code, std::vector<uint8_t>* outSPIRV,
                     const glslang_resource_t* glslLangResource = nullptr,
                     bool generateDebugInfo = false);

const std::string readShaderFile(const char* fileName);
ShaderStage shaderStageFromFileName(const char* fileName);
VkShaderStageFlagBits shaderStageToVk(ShaderStage stage);
glslang_resource_t getGlslangResource(
  const VkPhysicalDeviceLimits& limits,
  const VkPhysicalDeviceMeshShaderPropertiesEXT* meshShader = nullptr);

void destroy(VulkanContext* context, ShaderModuleHandle handle);
void destroy(VulkanContext* context, RenderPipelineHandle handle);

}  // namespace mrvk
