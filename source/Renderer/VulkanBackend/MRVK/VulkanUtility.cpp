#include "VulkanUtility.h"

#include "MRVKClasses.h"
#include "VkMacro.h"
#include "Volk/volk.h"
#include "VulkanStructs.h"
#include "glsl/glslang_c_interface.h"
#include "glsl/glslang_c_shader_types.h"
#include "minilog.h"
#include "scope_guard/scope_guard.hpp"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

mrvk::VulkanResult mrvk::getResultFromVkResult(VkResult result) {
  switch (result) {
    case VK_SUCCESS:
      return VulkanResult::Ok;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return VulkanResult::OutOfHostMemory;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return VulkanResult::OutOfDeviceMemory;
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      return VulkanResult::OutOfPoolMemory;
    case VK_ERROR_DEVICE_LOST:
      return VulkanResult::DeviceLost;
    case VK_ERROR_INITIALIZATION_FAILED:
      return VulkanResult::InitializationFailed;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return VulkanResult::ExtensionNotPresent;
    case VK_ERROR_LAYER_NOT_PRESENT:
      return VulkanResult::ExtensionNotPresent;
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return VulkanResult::IncompatibleDriver;
    default:
      return VulkanResult::Unknown;
  }
}

mrvk::HWDeviceType mrvk::convertVulkanDeviceTypeToMRVK(VkPhysicalDeviceType vkDeviceType) {
  switch (vkDeviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return HWDeviceType_Discrete;

    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return HWDeviceType_Integrated;

    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return HWDeviceType_External;

    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return HWDeviceType_Software;

    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return HWDeviceType_External;

    default:
      return HWDeviceType_Software;
  }
}

const char* mrvk::convertValidationFeatureToString(VkValidationFeatureEnableEXT feature) {
  switch (feature) {
    case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
      return "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT";
    case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
      return "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT";
    case VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT:
      return "VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT";
    case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
      return "VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT";
    case VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT:
      return "VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT";
    case VK_VALIDATION_FEATURE_ENABLE_MAX_ENUM_EXT:
      return "VK_VALIDATION_FEATURE_ENABLE_MAX_ENUM_EXT";
  }
  return "VK_VALIDATION_FEATURE_ENABLE_MAX_ENUM_EXT";
}

const char* mrvk::deviceFeatureToString(DeviceFeature feature) {
#define MRVK_FEATURE_TO_STRING(name)                                                               \
  case name:                                                                                       \
    return #name

  switch (feature) {
    // Vulkan 1.0
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_RobustBufferAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_FullDrawIndexUint32);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ImageCubeArray);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_IndependentBlend);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_GeometryShader);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_TessellationShader);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SampleRateShading);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_DualSrcBlend);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_LogicOp);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_MultiDrawIndirect);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_DrawIndirectFirstInstance);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_DepthClamp);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_DepthBiasClamp);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_FillModeNonSolid);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_DepthBounds);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_WideLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_LargePoints);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_AlphaToOne);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_MultiViewport);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SamplerAnisotropy);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_TextureCompressionETC2);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_TextureCompressionASTC_LDR);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_TextureCompressionBC);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_OcclusionQueryPrecise);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_PipelineStatisticsQuery);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_VertexPipelineStoresAndAtomics);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_FragmentStoresAndAtomics);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderTessellationAndGeometryPointSize);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderImageGatherExtended);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderStorageImageExtendedFormats);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderStorageImageMultisample);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderStorageImageReadWithoutFormat);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderStorageImageWriteWithoutFormat);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderUniformBufferArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderSampledImageArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderStorageBufferArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderStorageImageArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderClipDistance);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderCullDistance);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderFloat64);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderInt64);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderInt16);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderResourceResidency);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_ShaderResourceMinLod);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseBinding);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidencyBuffer);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidencyImage2D);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidencyImage3D);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidency2Samples);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidency4Samples);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidency8Samples);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidency16Samples);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_SparseResidencyAliased);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_VariableMultisampleRate);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_10_InheritedQueries);

    // Vulkan 1.1
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_StorageBuffer16BitAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_UniformAndStorageBuffer16BitAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_StoragePushConstant16);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_StorageInputOutput16);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_Multiview);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_MultiviewGeometryShader);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_MultiviewTessellationShader);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_VariablePointersStorageBuffer);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_VariablePointers);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_ProtectedMemory);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_SamplerYcbcrConversion);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_11_ShaderDrawParameters);

    // Vulkan 1.2
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_SamplerMirrorClampToEdge);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DrawIndirectCount);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_StorageBuffer8BitAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_UniformAndStorageBuffer8BitAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_StoragePushConstant8);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderBufferInt64Atomics);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderSharedInt64Atomics);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderFloat16);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderInt8);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderInputAttachmentArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderUniformTexelBufferArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderStorageTexelBufferArrayDynamicIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderUniformBufferArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderSampledImageArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderStorageBufferArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderStorageImageArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderInputAttachmentArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderUniformTexelBufferArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderStorageTexelBufferArrayNonUniformIndexing);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingUniformBufferUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingSampledImageUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingStorageImageUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingStorageBufferUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingUniformTexelBufferUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingStorageTexelBufferUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingUpdateUnusedWhilePending);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingPartiallyBound);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_DescriptorBindingVariableDescriptorCount);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_RuntimeDescriptorArray);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_SamplerFilterMinmax);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ScalarBlockLayout);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ImagelessFramebuffer);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_UniformBufferStandardLayout);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderSubgroupExtendedTypes);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_SeparateDepthStencilLayouts);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_HostQueryReset);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_TimelineSemaphore);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_BufferDeviceAddress);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_BufferDeviceAddressCaptureReplay);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_BufferDeviceAddressMultiDevice);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_VulkanMemoryModel);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_VulkanMemoryModelDeviceScope);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_VulkanMemoryModelAvailabilityVisibilityChains);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderOutputViewportIndex);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_ShaderOutputLayer);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_12_SubgroupBroadcastDynamicId);

    // Vulkan 1.3
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_RobustImageAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_InlineUniformBlock);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_DescriptorBindingInlineUniformBlockUpdateAfterBind);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_PipelineCreationCacheControl);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_PrivateData);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_ShaderDemoteToHelperInvocation);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_ShaderTerminateInvocation);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_SubgroupSizeControl);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_ComputeFullSubgroups);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_Synchronization2);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_TextureCompressionASTC_HDR);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_ShaderZeroInitializeWorkgroupMemory);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_DynamicRendering);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_ShaderIntegerDotProduct);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_13_Maintenance4);

    // Vulkan 1.4
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_GlobalPriorityQuery);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_ShaderSubgroupRotate);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_ShaderSubgroupRotateClustered);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_ShaderFloatControls2);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_ShaderExpectAssume);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_RectangularLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_BresenhamLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_SmoothLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_StippledRectangularLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_StippledBresenhamLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_StippledSmoothLines);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_VertexAttributeInstanceRateDivisor);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_VertexAttributeInstanceRateZeroDivisor);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_IndexTypeUint8);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_DynamicRenderingLocalRead);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_Maintenance5);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_Maintenance6);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_PipelineProtectedAccess);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_PipelineRobustness);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_HostImageCopy);
    MRVK_FEATURE_TO_STRING(DeviceFeatures_14_PushDescriptor);

    // Extensions
    MRVK_FEATURE_TO_STRING(DeviceFeatures_KHR_SwapchainMaintenance1);

    default:
      return "UnknownDeviceFeature";
  }

#undef MRVK_FEATURE_TO_STRING
}

VkDynamicState mrvk::convertToVkDynamicState(RenderPipelineDesc::DynamicState state) {
  switch (state) {
    case RenderPipelineDesc::VIEWPORT_STATE:
      return VK_DYNAMIC_STATE_VIEWPORT;
    case RenderPipelineDesc::SCISSOR_STATE:
      return VK_DYNAMIC_STATE_SCISSOR;
    case RenderPipelineDesc::NO_STATE:
      return VK_DYNAMIC_STATE_MAX_ENUM;
    default:
      return VK_DYNAMIC_STATE_MAX_ENUM;
  }
}

VkPipelineColorBlendAttachmentState mrvk::convertToVkBlendAttachmentState(
  RenderPipelineDesc::BlendAttachment& blendState) {
  return {.blendEnable = blendState.blendEnable,
          .srcColorBlendFactor = blendState.srcColorBlendFactor,
          .dstColorBlendFactor = blendState.dstColorBlendFactor,
          .colorBlendOp = blendState.colorBlendOp,
          .srcAlphaBlendFactor = blendState.srcAlphaBlendFactor,
          .dstAlphaBlendFactor = blendState.dstAlphaBlendFactor,
          .alphaBlendOp = blendState.alphaBlendOp,
          .colorWriteMask = blendState.colorWriteMask};
}

VkSemaphore mrvk::createSemaphore(VkDevice device, const char* semaphoreName) {
  VkSemaphoreCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                              .pNext = nullptr,
                              .flags = 0};
  VkSemaphore semaphone;
  MRVK_VERIFY(vkCreateSemaphore(device, &ci, nullptr, &semaphone));
  MRVK_VERIFY(
    mrvk::setDebugObjectName(device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)semaphone, semaphoreName));
  return semaphone;
}

VkFence mrvk::createFence(VkDevice device, const char* fenceName) {
  VkFenceCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                          .pNext = nullptr,
                          .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  VkFence fence;

  MRVK_VERIFY(vkCreateFence(device, &ci, nullptr, &fence));
  MRVK_VERIFY(setDebugObjectName(device, VK_OBJECT_TYPE_FENCE, (uint64_t)fence, fenceName));
  return fence;
}

VkResult mrvk::setDebugObjectName(VkDevice device, VkObjectType objectType, uint64_t handle,
                                  const char* objectName) {
  if (!objectName || !*objectName || !vkSetDebugUtilsObjectNameEXT) {
    return VK_SUCCESS;
  }

  const VkDebugUtilsObjectNameInfoEXT ni = {.sType =
                                              VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                                            .objectType = objectType,
                                            .objectHandle = handle,
                                            .pObjectName = objectName};

  return vkSetDebugUtilsObjectNameEXT(device, &ni);
}

mrvk::Result mrvk::compileShader(VkShaderStageFlagBits stage, const char* code,
                                 std::vector<uint8_t>* outSPIRV,
                                 const glslang_resource_t* glslLangResource,
                                 bool generateDebugInfo) {
  glslang_stage_t glslStage;
  switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
      glslStage = GLSLANG_STAGE_VERTEX;
      break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      glslStage = GLSLANG_STAGE_FRAGMENT;
      break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
      glslStage = GLSLANG_STAGE_GEOMETRY;
      break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
      glslStage = GLSLANG_STAGE_TESSCONTROL;
      break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
      glslStage = GLSLANG_STAGE_TESSEVALUATION;
      break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
      glslStage = GLSLANG_STAGE_COMPUTE;
      break;
    default:
      return mrvk::Result{Result::Code_Error};
  }

  const glslang_input_t input = {
    .language = GLSLANG_SOURCE_GLSL,
    .stage = glslStage,
    .client = GLSLANG_CLIENT_VULKAN,
    .client_version = GLSLANG_TARGET_VULKAN_1_3,
    .target_language = GLSLANG_TARGET_SPV,
    .target_language_version = GLSLANG_TARGET_SPV_1_6,
    .code = code,
    .default_version = 100,
    .default_profile = GLSLANG_NO_PROFILE,
    .force_default_version_and_profile = false,
    .forward_compatible = false,
    .messages = GLSLANG_MSG_DEFAULT_BIT,
    .resource = glslLangResource,
  };

  glslang_shader_t* shader = glslang_shader_create(&input);
  SCOPE_EXIT {
    glslang_shader_delete(shader);
  };

  if (!glslang_shader_preprocess(shader, &input)) {
    LLOGW("Shader preprocessing failde: \n");
    LLOGW("   %s\n", glslang_shader_get_info_log(shader));
    LLOGW("   %s\n", glslang_shader_get_info_debug_log(shader));
    return mrvk::Result(mrvk::Result::Code_RuntimeError);
  }

  if (!glslang_shader_parse(shader, &input)) {
    LLOGW("Shader parsing failed:\n");
    LLOGW("   %s\n", glslang_shader_get_info_log(shader));
    LLOGW("   %s\n", glslang_shader_get_info_debug_log(shader));
    assert(false);
    return mrvk::Result(mrvk::Result::Code_RuntimeError, "glslang_shader_parse() failed");
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
    LLOGW("Shader linking failed:\n");
    LLOGW("  %s\n", glslang_program_get_info_log(program));
    LLOGW("  %s\n", glslang_program_get_info_debug_log(program));
    assert(false);
    return mrvk::Result(mrvk::Result::Code_RuntimeError, "glslang_program_link() failed");
  }

  glslang_spv_options_t options = {
    .generate_debug_info = generateDebugInfo,
    .strip_debug_info = !generateDebugInfo,
    .disable_optimizer = false,
    .optimize_size = true,
    .disassemble = false,
    .validate = true,
    .emit_nonsemantic_shader_debug_info = false,
    .emit_nonsemantic_shader_debug_source = false,
  };

  glslang_program_SPIRV_generate_with_options(program, input.stage, &options);

  if (glslang_program_SPIRV_get_messages(program)) {
    LLOGW("%s\n", glslang_program_SPIRV_get_messages(program));
  }

  const uint8_t* spirv = reinterpret_cast<const uint8_t*>(glslang_program_SPIRV_get_ptr(program));
  const size_t numBytes = glslang_program_SPIRV_get_size(program) * sizeof(uint32_t);

  *outSPIRV = std::vector(spirv, spirv + numBytes);

  return mrvk::Result{};
}

const std::string mrvk::readShaderFile(const char* fileName) {
  std::ifstream file(fileName, std::ios::in);

  if (!file.is_open()) {
    MRVK_LOG_WARNING("Couldn't open the file: %s", fileName);

    return {};
  }

  std::string code{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

  if (file.bad()) {
    MRVK_LOG_ERROR("Failed to read the file: %s", fileName);

    return {};
  }

  MRVK_LOG_INFO("Successfully read the file: %s", fileName);

  return code;
}

mrvk::ShaderStage mrvk::shaderStageFromFileName(const char* fileName) {
  std::string_view path(fileName);
  const size_t dotPos = path.find_last_of('.');
  if (dotPos == std::string_view::npos)
    return ShaderStage::Stage_Miss;

  const std::string_view extension = path.substr(dotPos + 1);
  if (extension == "vert")
    return ShaderStage::Stage_Vert;
  else if (extension == "frag")
    return ShaderStage::Stage_Frag;
  else if (extension == "comp")
    return ShaderStage::Stage_Comp;
  else if (extension == "geom")
    return ShaderStage::Stage_Geom;
  else if (extension == "tesc")
    return ShaderStage::Stage_Tesc;
  else if (extension == "tese")
    return ShaderStage::Stage_Tese;

  return ShaderStage::Stage_Miss;
}

VkShaderStageFlagBits mrvk::shaderStageToVk(ShaderStage stage) {
  switch (stage) {
    case Stage_Vert:
      return VK_SHADER_STAGE_VERTEX_BIT;

    case Stage_Frag:
      return VK_SHADER_STAGE_FRAGMENT_BIT;

    case Stage_Comp:
      return VK_SHADER_STAGE_COMPUTE_BIT;

    case Stage_Geom:
      return VK_SHADER_STAGE_GEOMETRY_BIT;

    case Stage_Tesc:
      return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    case Stage_Tese:
      return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    case Stage_RayGen:
      return VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    case Stage_AnyHit:
      return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

    case Stage_ClosestHit:
      return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    case Stage_Miss:
      return VK_SHADER_STAGE_MISS_BIT_KHR;

    case Stage_Intersection:
      return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

    case Stage_Callable:
      return VK_SHADER_STAGE_CALLABLE_BIT_KHR;

    case Stage_Task:
      return VK_SHADER_STAGE_TASK_BIT_EXT;

    case Stage_Mesh:
      return VK_SHADER_STAGE_MESH_BIT_EXT;

    default:
      return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  }
}

glslang_resource_t mrvk::getGlslangResource(
  const VkPhysicalDeviceLimits& limits, const VkPhysicalDeviceMeshShaderPropertiesEXT* meshShader) {
  // minimums guaranteed by the `VK_EXT_mesh_shader` spec - used when the extension is unavailable
  const VkPhysicalDeviceMeshShaderPropertiesEXT kMeshShaderMinimums = {
    .maxTaskWorkGroupSize = {128, 128, 128},
    .maxMeshWorkGroupSize = {128, 128, 128},
    .maxMeshOutputVertices = 256,
    .maxMeshOutputPrimitives = 256,
    .maxMeshMultiviewViewCount = 1,
  };
  const VkPhysicalDeviceMeshShaderPropertiesEXT& mesh =
    meshShader ? *meshShader : kMeshShaderMinimums;

  // maximum sample count usable for both color and depth attachments - the size of
  // `gl_SampleMask[]`
  const auto getMaxSampleCount = [&limits]() -> int {
    const VkSampleCountFlags counts =
      limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
    for (VkSampleCountFlags bit = VK_SAMPLE_COUNT_64_BIT; bit; bit >>= 1) {
      if (counts & bit)
        return (int)bit;
    }
    return 1;
  };

  const glslang_resource_t resource = {
    .max_lights = 32,  // legacy OpenGL compatibility profile, no Vulkan equivalent
    .max_clip_planes = (int)limits.maxClipDistances,
    .max_texture_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_texture_coords = (int)limits.maxPerStageDescriptorSampledImages,
    .max_vertex_attribs = (int)limits.maxVertexInputAttributes,
    .max_vertex_uniform_components = (int)limits.maxUniformBufferRange / 4,
    .max_varying_floats =
      (int)std::min(limits.maxVertexOutputComponents, limits.maxFragmentInputComponents),
    .max_vertex_texture_image_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_combined_texture_image_units = (int)limits.maxDescriptorSetSampledImages,
    .max_texture_image_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_fragment_uniform_components = (int)limits.maxUniformBufferRange / 4,
    .max_draw_buffers = (int)limits.maxColorAttachments,
    .max_vertex_uniform_vectors = (int)limits.maxUniformBufferRange / 16,
    .max_varying_vectors =
      (int)std::min(limits.maxVertexOutputComponents, limits.maxFragmentInputComponents) / 4,
    .max_fragment_uniform_vectors = (int)limits.maxUniformBufferRange / 16,
    .max_vertex_output_vectors = (int)limits.maxVertexOutputComponents / 4,
    .max_fragment_input_vectors = (int)limits.maxFragmentInputComponents / 4,
    .min_program_texel_offset = limits.minTexelOffset,
    .max_program_texel_offset = (int)limits.maxTexelOffset,
    .max_clip_distances = (int)limits.maxClipDistances,
    .max_compute_work_group_count_x = (int)limits.maxComputeWorkGroupCount[0],
    .max_compute_work_group_count_y = (int)limits.maxComputeWorkGroupCount[1],
    .max_compute_work_group_count_z = (int)limits.maxComputeWorkGroupCount[2],
    .max_compute_work_group_size_x = (int)limits.maxComputeWorkGroupSize[0],
    .max_compute_work_group_size_y = (int)limits.maxComputeWorkGroupSize[1],
    .max_compute_work_group_size_z = (int)limits.maxComputeWorkGroupSize[2],
    .max_compute_uniform_components = (int)limits.maxUniformBufferRange / 4,
    .max_compute_texture_image_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_compute_image_uniforms = (int)limits.maxPerStageDescriptorStorageImages,
    // atomic counters are an OpenGL-only feature, no Vulkan equivalent
    .max_compute_atomic_counters = 8,
    .max_compute_atomic_counter_buffers = 1,
    .max_varying_components =
      (int)std::min(limits.maxVertexOutputComponents, limits.maxFragmentInputComponents),
    .max_vertex_output_components = (int)limits.maxVertexOutputComponents,
    .max_geometry_input_components = (int)limits.maxGeometryInputComponents,
    .max_geometry_output_components = (int)limits.maxGeometryOutputComponents,
    .max_fragment_input_components = (int)limits.maxFragmentInputComponents,
    .max_image_units = (int)limits.maxPerStageDescriptorStorageImages,
    .max_combined_image_units_and_fragment_outputs = (int)limits.maxFragmentCombinedOutputResources,
    .max_combined_shader_output_resources = (int)limits.maxFragmentCombinedOutputResources,
    .max_image_samples = 0,  // multisampled storage images, no Vulkan equivalent
    .max_vertex_image_uniforms = (int)limits.maxPerStageDescriptorStorageImages,
    .max_tess_control_image_uniforms = (int)limits.maxPerStageDescriptorStorageImages,
    .max_tess_evaluation_image_uniforms = (int)limits.maxPerStageDescriptorStorageImages,
    .max_geometry_image_uniforms = (int)limits.maxPerStageDescriptorStorageImages,
    .max_fragment_image_uniforms = (int)limits.maxPerStageDescriptorStorageImages,
    .max_combined_image_uniforms = (int)limits.maxDescriptorSetStorageImages,
    .max_geometry_texture_image_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_geometry_output_vertices = (int)limits.maxGeometryOutputVertices,
    .max_geometry_total_output_components = (int)limits.maxGeometryTotalOutputComponents,
    .max_geometry_uniform_components = (int)limits.maxUniformBufferRange / 4,
    .max_geometry_varying_components = (int)limits.maxGeometryInputComponents,
    .max_tess_control_input_components = (int)limits.maxTessellationControlPerVertexInputComponents,
    .max_tess_control_output_components =
      (int)limits.maxTessellationControlPerVertexOutputComponents,
    .max_tess_control_texture_image_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_tess_control_uniform_components = (int)limits.maxUniformBufferRange / 4,
    .max_tess_control_total_output_components =
      (int)limits.maxTessellationControlTotalOutputComponents,
    .max_tess_evaluation_input_components = (int)limits.maxTessellationEvaluationInputComponents,
    .max_tess_evaluation_output_components = (int)limits.maxTessellationEvaluationOutputComponents,
    .max_tess_evaluation_texture_image_units = (int)limits.maxPerStageDescriptorSampledImages,
    .max_tess_evaluation_uniform_components = (int)limits.maxUniformBufferRange / 4,
    .max_tess_patch_components = (int)limits.maxTessellationControlPerPatchOutputComponents,
    .max_patch_vertices = (int)limits.maxTessellationPatchSize,
    .max_tess_gen_level = (int)limits.maxTessellationGenerationLevel,
    .max_viewports = (int)limits.maxViewports,
    .max_vertex_atomic_counters = 0,
    .max_tess_control_atomic_counters = 0,
    .max_tess_evaluation_atomic_counters = 0,
    .max_geometry_atomic_counters = 0,
    .max_fragment_atomic_counters = 8,
    .max_combined_atomic_counters = 8,
    .max_atomic_counter_bindings = 1,
    .max_vertex_atomic_counter_buffers = 0,
    .max_tess_control_atomic_counter_buffers = 0,
    .max_tess_evaluation_atomic_counter_buffers = 0,
    .max_geometry_atomic_counter_buffers = 0,
    .max_fragment_atomic_counter_buffers = 1,
    .max_combined_atomic_counter_buffers = 1,
    .max_atomic_counter_buffer_size = 16384,
    // `VK_EXT_transform_feedback` is not enabled by LVK, no properties to query
    .max_transform_feedback_buffers = 4,
    .max_transform_feedback_interleaved_components = 64,
    .max_cull_distances = (int)limits.maxCullDistances,
    .max_combined_clip_and_cull_distances = (int)limits.maxCombinedClipAndCullDistances,
    .max_samples = getMaxSampleCount(),
    // `VK_NV_mesh_shader` is not enabled by LVK, no properties to query
    .max_mesh_output_vertices_nv = 256,
    .max_mesh_output_primitives_nv = 512,
    .max_mesh_work_group_size_x_nv = 32,
    .max_mesh_work_group_size_y_nv = 32,
    .max_mesh_work_group_size_z_nv = 32,
    .max_task_work_group_size_x_nv = 32,
    .max_task_work_group_size_y_nv = 32,
    .max_task_work_group_size_z_nv = 32,
    .max_mesh_view_count_nv = 4,
    .max_mesh_output_vertices_ext = (int)mesh.maxMeshOutputVertices,
    .max_mesh_output_primitives_ext = (int)mesh.maxMeshOutputPrimitives,
    .max_mesh_work_group_size_x_ext = (int)mesh.maxMeshWorkGroupSize[0],
    .max_mesh_work_group_size_y_ext = (int)mesh.maxMeshWorkGroupSize[1],
    .max_mesh_work_group_size_z_ext = (int)mesh.maxMeshWorkGroupSize[2],
    .max_task_work_group_size_x_ext = (int)mesh.maxTaskWorkGroupSize[0],
    .max_task_work_group_size_y_ext = (int)mesh.maxTaskWorkGroupSize[1],
    .max_task_work_group_size_z_ext = (int)mesh.maxTaskWorkGroupSize[2],
    .max_mesh_view_count_ext = (int)mesh.maxMeshMultiviewViewCount,
    .maxDualSourceDrawBuffersEXT = (int)limits.maxFragmentDualSrcAttachments,
    .limits =
      {
        .non_inductive_for_loops = true,
        .while_loops = true,
        .do_while_loops = true,
        .general_uniform_indexing = true,
        .general_attribute_matrix_vector_indexing = true,
        .general_varying_indexing = true,
        .general_sampler_indexing = true,
        .general_variable_indexing = true,
        .general_constant_matrix_vector_indexing = true,
      },
  };

  return resource;
}

void mrvk::destroy(VulkanContext* context, ShaderModuleHandle handle) {
  if (context == nullptr && handle.empty())
    return;
  context->destroy(handle);
}

void mrvk::destroy(VulkanContext* context, RenderPipelineHandle handle) {
  if (context == nullptr && handle.empty())
    return;
  context->destroy(handle);
}
