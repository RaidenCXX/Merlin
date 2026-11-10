#pragma once

#include "Volk/volk.h"
#include "vulkan/vulkan_core.h"

#include <array>
#include <cstdint>
#include <vector>

namespace mrvk {

enum ShaderStage : uint8_t {
  Stage_Vert,
  Stage_Tesc,
  Stage_Tese,
  Stage_Geom,
  Stage_Frag,
  Stage_Comp,
  Stage_Task,
  Stage_Mesh,
  // ray tracing
  Stage_RayGen,
  Stage_AnyHit,
  Stage_ClosestHit,
  Stage_Miss,
  Stage_Intersection,
  Stage_Callable,
};

enum class VulkanResult {
  Ok = 0,
  OutOfMemory,
  OutOfHostMemory,
  OutOfDeviceMemory,
  OutOfPoolMemory,
  DeviceLost,
  InitializationFailed,
  ExtensionNotPresent,
  IncompatibleDriver,

  Unknown,
};

enum BlendOp : uint8_t {
  BlendOp_Add = 0,
  BlendOp_Subtract,
  BlendOp_ReverseSubtract,
  BlendOp_Min,
  BlendOp_Max,
};

enum Format : uint8_t {
  Format_Invalid = 0,

  Format_R_UN8,
  Format_R_UI8,
  Format_R_I8,
  Format_R_UI16,
  Format_R_I16,
  Format_R_UI32,
  Format_R_I32,
  Format_R_UN16,
  Format_R_F16,
  Format_R_F32,

  Format_A_UN8,

  Format_RG_UN8,
  Format_RG_UI8,
  Format_RG_I8,
  Format_RG_UI16,
  Format_RG_I16,
  Format_RG_UI32,
  Format_RG_I32,
  Format_RG_UN16,
  Format_RG_F16,
  Format_RG_F32,

  Format_RGBA_UN8,
  Format_RGBA_UI8,
  Format_RGBA_I8,
  Format_RGBA_UI16,
  Format_RGBA_I16,
  Format_RGBA_UI32,
  Format_RGBA_I32,
  Format_RGBA_UN16,
  Format_RGBA_F16,
  Format_RGBA_F32,
  Format_RGBA_SRGB8,

  Format_BGRA_UN8,
  Format_BGRA_UI8,
  Format_BGRA_SRGB8,

  Format_A2B10G10R10_UN,
  Format_A2R10G10B10_UN,
  Format_A1B5G5R5_UN,
  Format_R4G4B4A4_UN,
  Format_R5G5B5A1_UN,
  Format_R5G6B5_UN,

  // packed floating-point formats, as used for HDR render targets and irradiance data
  Format_B10G11R11_UF,
  Format_E5B9G9R9_UF,

  Format_ETC2_RGB8,
  Format_ETC2_SRGB8,
  Format_EAC_RG11,
  Format_BC1_RGBA,
  Format_BC1_SRGBA,
  Format_BC3_RGBA,
  Format_BC3_SRGBA,
  Format_BC4_R,
  Format_BC5_RG,     // 2-channel block compression (tangent-space normal maps etc)
  Format_BC5_RG_SN,  // signed variant of `Format_BC5_RG`
  Format_BC7_RGBA,
  Format_BC7_SRGBA,
  Format_ASTC_4x4_RGBA,   // requires `textureCompressionASTC_LDR`
  Format_ASTC_4x4_SRGBA,  // requires `textureCompressionASTC_LDR`
  Format_ASTC_5x5_RGBA,   // requires `textureCompressionASTC_LDR`
  Format_ASTC_5x5_SRGBA,  // requires `textureCompressionASTC_LDR`
  Format_ASTC_6x6_RGBA,   // requires `textureCompressionASTC_LDR`
  Format_ASTC_6x6_SRGBA,  // requires `textureCompressionASTC_LDR`

  Format_Z_UN16,
  Format_Z_UN24,
  Format_Z_F32,
  Format_Z_UN24_S_UI8,
  Format_Z_F32_S_UI8,
  Format_S_UI8,  // stencil-only

  Format_YUV_NV12,
  Format_YUV_420p,
};

enum HWDeviceType {
  HWDeviceType_Integrated = 1,
  HWDeviceType_Discrete = 2,
  HWDeviceType_External = 3,
  HWDeviceType_Software = 4,
};

enum ColorSpace : uint8_t {
  ColorSpace_SRGB_NONLINEAR,
  ColorSpace_SRGB_EXTENDED_LINEAR,
  ColorSpace_HDR10,
  ColorSpace_BT709_LINEAR,
};

enum Topology : uint8_t {
  Topology_Point,
  Topology_Line,
  Topology_LineStrip,
  Topology_Triangle,
  Topology_TriangleStrip,
  Topology_Patch,
};

enum ColorComponentBits : uint8_t {
  ColorComponentBit_R = 1 << 0,
  ColorComponentBit_G = 1 << 1,
  ColorComponentBit_B = 1 << 2,
  ColorComponentBit_A = 1 << 3,
  ColorComponentBit_All =
    ColorComponentBit_R | ColorComponentBit_G | ColorComponentBit_B | ColorComponentBit_A,
};

enum PolygonMode {
  PolygonMode_Fill,  //
  PolygonMode_Line,  //
  PolygonMode_Point  //
};

enum CallMode {
  CallMode_Front,  //
  CallMode_Back,   //
  CallMode_None    //
};

enum FrontFace {
  FrontFace_Clockwise,        //
  FrontFace_Conter_Clockwise  //
};

enum PipelineType {
  GRAPHICS_PIPELINE_TYPE = 1,
  COMPUTE_PIPELINE_TYPE = 2,
};

enum DeviceFeature : uint32_t {
  DeviceFeatures_10_RobustBufferAccess = 0,
  DeviceFeatures_10_FullDrawIndexUint32,
  DeviceFeatures_10_ImageCubeArray,
  DeviceFeatures_10_IndependentBlend,
  DeviceFeatures_10_GeometryShader,  // Enable geometry shader
  DeviceFeatures_10_TessellationShader,
  DeviceFeatures_10_SampleRateShading,  // Execute the Fragment Shader for multiple sample positions
  DeviceFeatures_10_DualSrcBlend,
  DeviceFeatures_10_LogicOp,
  DeviceFeatures_10_MultiDrawIndirect,  // Multiple indirect draw commands in a single call
  DeviceFeatures_10_DrawIndirectFirstInstance,
  DeviceFeatures_10_DepthClamp,
  DeviceFeatures_10_DepthBiasClamp,
  DeviceFeatures_10_FillModeNonSolid,
  DeviceFeatures_10_DepthBounds,
  DeviceFeatures_10_WideLines,
  DeviceFeatures_10_LargePoints,
  DeviceFeatures_10_AlphaToOne,
  DeviceFeatures_10_MultiViewport,
  DeviceFeatures_10_SamplerAnisotropy,
  DeviceFeatures_10_TextureCompressionETC2,
  DeviceFeatures_10_TextureCompressionASTC_LDR,
  DeviceFeatures_10_TextureCompressionBC,
  DeviceFeatures_10_OcclusionQueryPrecise,
  DeviceFeatures_10_PipelineStatisticsQuery,
  DeviceFeatures_10_VertexPipelineStoresAndAtomics,  // Allows Vertex, Tessellation, and Geometry
                                                     // Shaders to write data to a Storage Buffer
                                                     // and perform atomic operations.
  DeviceFeatures_10_FragmentStoresAndAtomics,        // Allows a fragment shader to write data to a
                                               // storage buffer and perform atomic operations.
  DeviceFeatures_10_ShaderTessellationAndGeometryPointSize,
  DeviceFeatures_10_ShaderImageGatherExtended,
  DeviceFeatures_10_ShaderStorageImageExtendedFormats,
  DeviceFeatures_10_ShaderStorageImageMultisample,
  DeviceFeatures_10_ShaderStorageImageReadWithoutFormat,
  DeviceFeatures_10_ShaderStorageImageWriteWithoutFormat,
  DeviceFeatures_10_ShaderUniformBufferArrayDynamicIndexing,
  DeviceFeatures_10_ShaderSampledImageArrayDynamicIndexing,
  DeviceFeatures_10_ShaderStorageBufferArrayDynamicIndexing,
  DeviceFeatures_10_ShaderStorageImageArrayDynamicIndexing,
  DeviceFeatures_10_ShaderClipDistance,
  DeviceFeatures_10_ShaderCullDistance,
  DeviceFeatures_10_ShaderFloat64,
  DeviceFeatures_10_ShaderInt64,  // Allows the use of 64-bit integers (int64_t and uint64_t) in
                                  // shaders for arithmetic operations.
  DeviceFeatures_10_ShaderInt16,
  DeviceFeatures_10_ShaderResourceResidency,
  DeviceFeatures_10_ShaderResourceMinLod,
  DeviceFeatures_10_SparseBinding,
  DeviceFeatures_10_SparseResidencyBuffer,
  DeviceFeatures_10_SparseResidencyImage2D,
  DeviceFeatures_10_SparseResidencyImage3D,
  DeviceFeatures_10_SparseResidency2Samples,
  DeviceFeatures_10_SparseResidency4Samples,
  DeviceFeatures_10_SparseResidency8Samples,
  DeviceFeatures_10_SparseResidency16Samples,
  DeviceFeatures_10_SparseResidencyAliased,
  DeviceFeatures_10_VariableMultisampleRate,
  DeviceFeatures_10_InheritedQueries,
  DeviceFeatures_11_StorageBuffer16BitAccess = 100,  // Allows a shader to use 16-bit data types for
                                                     // reading from and writing to a storage buffer
                                                     // (uint16_t, int16_t, float16_t — depending on
                                                     // support for the respective types and
                                                     // capabilities).
  DeviceFeatures_11_UniformAndStorageBuffer16BitAccess,
  DeviceFeatures_11_StoragePushConstant16,
  DeviceFeatures_11_StorageInputOutput16,
  DeviceFeatures_11_Multiview,
  DeviceFeatures_11_MultiviewGeometryShader,
  DeviceFeatures_11_MultiviewTessellationShader,
  DeviceFeatures_11_VariablePointersStorageBuffer,
  DeviceFeatures_11_VariablePointers,
  DeviceFeatures_11_ProtectedMemory,
  DeviceFeatures_11_SamplerYcbcrConversion,  // Allows for the conversion of images between RGB and
                                             // YCbCr formats when working with samplers. It is
                                             // primarily used for video decoding and processing
                                             // YCbCr textures.
  DeviceFeatures_11_ShaderDrawParameters,    // Allows shaders to access parameters of the current
                                             // draw call: `gl_DrawID`, `gl_BaseVertex`, and
  // `gl_BaseInstance`. It is primarily used for GPU-driven
  // rendering and MultiDrawIndirect.
  DeviceFeatures_12_SamplerMirrorClampToEdge = 200,
  DeviceFeatures_12_DrawIndirectCount,
  DeviceFeatures_12_StorageBuffer8BitAccess,
  DeviceFeatures_12_UniformAndStorageBuffer8BitAccess,
  DeviceFeatures_12_StoragePushConstant8,
  DeviceFeatures_12_ShaderBufferInt64Atomics,
  DeviceFeatures_12_ShaderSharedInt64Atomics,
  DeviceFeatures_12_ShaderFloat16,
  DeviceFeatures_12_ShaderInt8,
  DeviceFeatures_12_DescriptorIndexing,
  DeviceFeatures_12_ShaderInputAttachmentArrayDynamicIndexing,
  DeviceFeatures_12_ShaderUniformTexelBufferArrayDynamicIndexing,
  DeviceFeatures_12_ShaderStorageTexelBufferArrayDynamicIndexing,
  DeviceFeatures_12_ShaderUniformBufferArrayNonUniformIndexing,
  DeviceFeatures_12_ShaderSampledImageArrayNonUniformIndexing,
  DeviceFeatures_12_ShaderStorageBufferArrayNonUniformIndexing,
  DeviceFeatures_12_ShaderStorageImageArrayNonUniformIndexing,
  DeviceFeatures_12_ShaderInputAttachmentArrayNonUniformIndexing,
  DeviceFeatures_12_ShaderUniformTexelBufferArrayNonUniformIndexing,
  DeviceFeatures_12_ShaderStorageTexelBufferArrayNonUniformIndexing,
  DeviceFeatures_12_DescriptorBindingUniformBufferUpdateAfterBind,
  DeviceFeatures_12_DescriptorBindingSampledImageUpdateAfterBind,
  DeviceFeatures_12_DescriptorBindingStorageImageUpdateAfterBind,
  DeviceFeatures_12_DescriptorBindingStorageBufferUpdateAfterBind,
  DeviceFeatures_12_DescriptorBindingUniformTexelBufferUpdateAfterBind,
  DeviceFeatures_12_DescriptorBindingStorageTexelBufferUpdateAfterBind,
  DeviceFeatures_12_DescriptorBindingUpdateUnusedWhilePending,
  DeviceFeatures_12_DescriptorBindingPartiallyBound,
  DeviceFeatures_12_DescriptorBindingVariableDescriptorCount,
  DeviceFeatures_12_RuntimeDescriptorArray,
  DeviceFeatures_12_SamplerFilterMinmax,
  DeviceFeatures_12_ScalarBlockLayout,
  DeviceFeatures_12_ImagelessFramebuffer,
  DeviceFeatures_12_UniformBufferStandardLayout,
  DeviceFeatures_12_ShaderSubgroupExtendedTypes,
  DeviceFeatures_12_SeparateDepthStencilLayouts,
  DeviceFeatures_12_HostQueryReset,
  DeviceFeatures_12_TimelineSemaphore,
  DeviceFeatures_12_BufferDeviceAddress,
  DeviceFeatures_12_BufferDeviceAddressCaptureReplay,
  DeviceFeatures_12_BufferDeviceAddressMultiDevice,
  DeviceFeatures_12_VulkanMemoryModel,
  DeviceFeatures_12_VulkanMemoryModelDeviceScope,
  DeviceFeatures_12_VulkanMemoryModelAvailabilityVisibilityChains,
  DeviceFeatures_12_ShaderOutputViewportIndex,
  DeviceFeatures_12_ShaderOutputLayer,
  DeviceFeatures_12_SubgroupBroadcastDynamicId,
  DeviceFeatures_13_RobustImageAccess = 300,
  DeviceFeatures_13_InlineUniformBlock,
  DeviceFeatures_13_DescriptorBindingInlineUniformBlockUpdateAfterBind,
  DeviceFeatures_13_PipelineCreationCacheControl,
  DeviceFeatures_13_PrivateData,
  DeviceFeatures_13_ShaderDemoteToHelperInvocation,
  DeviceFeatures_13_ShaderTerminateInvocation,
  DeviceFeatures_13_SubgroupSizeControl,
  DeviceFeatures_13_ComputeFullSubgroups,
  DeviceFeatures_13_Synchronization2,
  DeviceFeatures_13_TextureCompressionASTC_HDR,
  DeviceFeatures_13_ShaderZeroInitializeWorkgroupMemory,
  DeviceFeatures_13_DynamicRendering,
  DeviceFeatures_13_ShaderIntegerDotProduct,
  DeviceFeatures_13_Maintenance4,
  DeviceFeatures_14_GlobalPriorityQuery = 400,
  DeviceFeatures_14_ShaderSubgroupRotate,
  DeviceFeatures_14_ShaderSubgroupRotateClustered,
  DeviceFeatures_14_ShaderFloatControls2,
  DeviceFeatures_14_ShaderExpectAssume,
  DeviceFeatures_14_RectangularLines,
  DeviceFeatures_14_BresenhamLines,
  DeviceFeatures_14_SmoothLines,
  DeviceFeatures_14_StippledRectangularLines,
  DeviceFeatures_14_StippledBresenhamLines,
  DeviceFeatures_14_StippledSmoothLines,
  DeviceFeatures_14_VertexAttributeInstanceRateDivisor,
  DeviceFeatures_14_VertexAttributeInstanceRateZeroDivisor,
  DeviceFeatures_14_IndexTypeUint8,
  DeviceFeatures_14_DynamicRenderingLocalRead,
  DeviceFeatures_14_Maintenance5,
  DeviceFeatures_14_Maintenance6,
  DeviceFeatures_14_PipelineProtectedAccess,
  DeviceFeatures_14_PipelineRobustness,
  DeviceFeatures_14_HostImageCopy,
  DeviceFeatures_14_PushDescriptor,
  DeviceFeatures_KHR_SwapchainMaintenance1 = 1000,
};

template <typename HandleType>
class Handle final {
  uint32_t index_ = UINT32_MAX;
  uint32_t generation_ = 0;

public:
  constexpr Handle() noexcept = default;

  constexpr Handle(uint32_t index, uint32_t generation) noexcept
      : index_(index), generation_(generation) {}

  Handle(const Handle& handle) {
    index_ = handle.index_;
    generation_ = handle.generation_;
  }

  Handle& operator=(const Handle& handle) noexcept {
    index_ = handle.index_;
    generation_ = handle.generation_;
    return *this;
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return index_ == UINT32_MAX || generation_ == 0;
  }

  [[nodiscard]]
  bool valid() const noexcept {
    return !empty();
  }

  [[nodiscard]]
  uint32_t index() const noexcept {
    return index_;
  }

  [[nodiscard]]
  uint32_t gen() const noexcept {
    return generation_;
  }

  friend bool operator==(Handle lhs, Handle rhs) noexcept = default;
};

using ShaderModuleHandle = Handle<struct ShaderModule>;
using TextureHandle = Handle<struct Texture>;
using RenderPipelineHandle = Handle<struct RenderPipeline>;

struct Barrier final {
  VkPipelineStageFlags2 srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 srcAccessMask = VK_ACCESS_2_NONE;
  VkPipelineStageFlags2 dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 dstAccessMask = VK_ACCESS_2_NONE;
  VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  TextureHandle textureHandle = {};
  VkImageSubresourceRange subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                              .baseMipLevel = 0,
                                              .levelCount = 1,
                                              .baseArrayLayer = 0,
                                              .layerCount = 1};
};

struct ValidationLayers {
  std::vector<const char*> layers;
  std::vector<const char*> settings;
  std::vector<VkValidationFeatureEnableEXT> fetures;
};

struct HWDeviceDesc {
  enum { MAX_PHYSICAL_DEVICE_NAME_SIZE = 256 };
  VkPhysicalDevice pd = 0;
  HWDeviceType type = HWDeviceType_Software;
  char name[MAX_PHYSICAL_DEVICE_NAME_SIZE] = {0};
};

struct DeviceFeaturesChain final {
  VkPhysicalDeviceFeatures2 features10 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan11Features features11 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
  VkPhysicalDeviceVulkan12Features features12 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  VkPhysicalDeviceVulkan13Features features13 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceVulkan14Features features14 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES};
  VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainMaintence1 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
    .pNext = nullptr};
  void linkChain() {
    features10.pNext = &features11;
    features11.pNext = &features12;
    features12.pNext = &features13;
    features13.pNext = &features14;
    features14.pNext = &swapchainMaintence1;
  }
};

struct DeviceProperties {
  VkPhysicalDeviceProperties2 properties = {.sType =
                                              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
  VkPhysicalDeviceDriverProperties driver = {.sType =
                                               VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
  DeviceProperties() { properties.pNext = &driver; }
};

struct DeviceQueues {
  const static uint32_t INVALID = 0xFFFFFFFF;
  uint32_t graphicsQueueFamilyIndex = INVALID;
  uint32_t computeQueueFamilyIndex = INVALID;
  uint32_t transferQueueFamilyIndex = INVALID;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkQueue computeQueue = VK_NULL_HANDLE;
  VkQueue transferQueue = VK_NULL_HANDLE;
};

struct Result {
  enum Code { Code_Ok, Code_ArgumentOutOfRange, Code_RuntimeError, Code_Error };

  Code code = Code_Ok;
  const char* message = "";
  explicit Result() = default;
  explicit Result(Code code, const char* message = "") : code(code), message(message) {}

  bool isOk() const { return code == Result::Code_Ok; }
};

struct ShaderModuleState final {
  VkShaderModule sm = VK_NULL_HANDLE;
  uint32_t pushConstantsSize = 0;
};

struct PushConstants {
  uint32_t value = 0;
};

struct VulkanImage final {
  VkImage vkImage = VK_NULL_HANDLE;
  VkImageView vkImageView = VK_NULL_HANDLE;
  VkDeviceMemory vkMemory = VK_NULL_HANDLE;
  VkImageUsageFlags vkUsageFlags = 0;
  VkFormat vkImageFormat = VK_FORMAT_MAX_ENUM;
  VkSampleCountFlagBits vkSamples_ = VK_SAMPLE_COUNT_1_BIT;
  void* mappedPtr_ = nullptr;
  bool isSwapchainImage = false;
  uint32_t numLevels_ = 1u;
  uint32_t numLayers_ = 1u;
};

struct ShaderModuleDesc final {
  ShaderStage stage = Stage_Frag;
  const char* data = nullptr;
  size_t dataSize = 0;
  const char* entryPointName = nullptr;
  const char* debugName = "";

  ShaderModuleDesc(const char* source, ShaderStage stage, const char* debugName)
      : stage(stage), data(source), debugName(debugName) {}
  ShaderModuleDesc(const void* data, size_t dataLenght, ShaderStage stage, const char* debugName)
      : data(static_cast<const char*>(data)),
        dataSize(dataLenght),
        stage(stage),
        debugName(debugName) {}
};

struct RenderPipelineDesc final {
  // PipelineRendering
  enum { MAX_COLOR_ATTACHMENT = 16 };
  struct ColorAttachment final {
    VkFormat format = VK_FORMAT_MAX_ENUM;
  } colorAttachment[MAX_COLOR_ATTACHMENT];

  struct DepthAttachment final {
    VkFormat format = VK_FORMAT_MAX_ENUM;
  } depthAttachment = {};

  struct StencilAttachment final {
    VkFormat format = VK_FORMAT_MAX_ENUM;
  } stencilAttachment = {};

  // clang-format off
  uint32_t getNumColorAttachments() const {
    uint32_t i = 0;
    while (i < MAX_COLOR_ATTACHMENT && colorAttachment[i].format != VK_FORMAT_MAX_ENUM) i++;
    return i;
  }
  bool hasDepthAttachment() const { return depthAttachment.format != VK_FORMAT_MAX_ENUM; }
  bool hasStencilAttachment() const { return stencilAttachment.format != VK_FORMAT_MAX_ENUM; }
  // clang-format on

  // ShaderStage
  struct Stage {
    ShaderModuleHandle smHandle = {};
    const char* entryPoint = "main";
    VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    enum { MAX_SPECIALIZATION_CONSTANT = 20 };
    VkSpecializationMapEntry specConstantMapEntries[MAX_SPECIALIZATION_CONSTANT] = {};
    const void* specConstData = nullptr;
    VkSpecializationInfo specializationInfo = {};

    uint32_t getNumSpecConstants() const {
      uint32_t i = 0;
      while (i < MAX_SPECIALIZATION_CONSTANT && specConstantMapEntries[i].size != 0)
        i++;
      return i;
    }
    uint32_t getSpecConstDataSize() const {
      uint32_t size = 0;

      for (uint32_t i = 0; i < MAX_SPECIALIZATION_CONSTANT && specConstantMapEntries[i].size != 0;
           ++i) {
        size = std::max(size, (uint32_t)(specConstantMapEntries[i].offset
                                         + specConstantMapEntries[i].size));
      }
      return size;
    }
  };

  Stage stageVert = {.shaderStage = VK_SHADER_STAGE_VERTEX_BIT};
  Stage stageTesc = {.shaderStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT};
  Stage stageTese = {.shaderStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT};
  Stage stageGeom = {.shaderStage = VK_SHADER_STAGE_GEOMETRY_BIT};
  Stage stageTask = {.shaderStage = VK_SHADER_STAGE_TASK_BIT_EXT};
  Stage stageMesh = {.shaderStage = VK_SHADER_STAGE_MESH_BIT_EXT};
  Stage stageFrag = {.shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT};

  // clang-format off
  uint32_t getNumStages(){
    uint32_t stageCount = 0;
    if(!stageVert.smHandle.empty()) stageCount++;
    if(!stageTesc.smHandle.empty()) stageCount++;
    if(!stageTese.smHandle.empty()) stageCount++;
    if(!stageGeom.smHandle.empty()) stageCount++;
    if(!stageTask.smHandle.empty()) stageCount++;
    if(!stageMesh.smHandle.empty()) stageCount++;
    if(!stageFrag.smHandle.empty()) stageCount++;
    return stageCount;
  }
  // clang-format on

  // VertexInput
  enum { MAX_VERTEX_INPUT_BINDINGS = 16 };
  enum { MAX_VERTEX_INPUT_ATTRIBUTES = 16 };
  std::array<VkVertexInputAttributeDescription, MAX_VERTEX_INPUT_ATTRIBUTES> vertexAttributes = [] {
    std::array<VkVertexInputAttributeDescription, MAX_VERTEX_INPUT_ATTRIBUTES> attributes = {};

    for (auto& attribute : attributes)
      attribute.format = VK_FORMAT_MAX_ENUM;

    return attributes;
  }();
  std::array<VkVertexInputBindingDescription, MAX_VERTEX_INPUT_BINDINGS> vertexBindings = [] {
    std::array<VkVertexInputBindingDescription, MAX_VERTEX_INPUT_BINDINGS> bindings{};

    for (auto& binding : bindings)
      binding.inputRate = VK_VERTEX_INPUT_RATE_MAX_ENUM;

    return bindings;
  }();
  // clang-format off
  uint32_t getNumAttributes() const {
    uint32_t i = 0;
    while (i < MAX_VERTEX_INPUT_ATTRIBUTES && vertexAttributes[i].format != VK_FORMAT_MAX_ENUM) i++;
    return i;
  }
  uint32_t getNumBindings() const {
    uint32_t i = 0;
    while (i < MAX_VERTEX_INPUT_BINDINGS && vertexBindings[i].inputRate != VK_VERTEX_INPUT_RATE_MAX_ENUM) i++;
    return i;
  }
  // clang-format on

  // InputAssembly
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkBool32 primitiveRestart = VK_FALSE;

  // Tessellation
  uint32_t patchControlPoints = 0;

  // Viewport
  enum { MAX_VIEWPORT = 4 };
  VkViewport viewports[MAX_VIEWPORT] = {};
  VkRect2D scissors[MAX_VIEWPORT] = {};

  // clang-format off
  uint32_t getNumViewports() {
    uint32_t i = 0;
    while (i < MAX_VIEWPORT && viewports[i].width != 0 && viewports[i].height != 0) { i++; }
    return i;
  }
  uint32_t getNumScissor() {
    uint32_t i = 0;
    while (i < MAX_VIEWPORT && scissors[i].extent.width != 0 && scissors[i].extent.height != 0) { i++; }
    return i;
  }

  // clang-format on
  // Resterization
  VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
  VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
  VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  // Multisample
  VkSampleCountFlagBits samplesCount = VK_SAMPLE_COUNT_1_BIT;
  float minSampleShading = 0.f;

  // Depth/Stencil
  VkCompareOp depthComperOp = VK_COMPARE_OP_GREATER;
  VkBool32 depthTest = VK_FALSE;
  VkBool32 stencilTest = VK_FALSE;
  VkStencilOpState stencilFront = {};
  VkStencilOpState stencilBack = {};

  // Color Blend
  struct BlendAttachment {
    VkBool32 blendEnable = VK_FALSE;
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
    VkColorComponentFlags colorWriteMask = {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
  };
  BlendAttachment colorBlendAttachments[MAX_COLOR_ATTACHMENT] = {};
  // clang-format off
  uint32_t getNumColorBlendAttachment() const {
    return getNumColorAttachments();
  }
  // clang-format on

  // DynamicState
  enum { MAX_DYNAMIC_STATE = 10 };
  enum DynamicState { NO_STATE = 0, VIEWPORT_STATE, SCISSOR_STATE };

  std::array<DynamicState, MAX_DYNAMIC_STATE> dynamicStates = {};

  uint32_t getNumDynamicStates() {
    size_t i = 0;
    while (i < MAX_DYNAMIC_STATE && dynamicStates[i] != NO_STATE) {
      i++;
    }
    return i;
  }

  const char* debugName = "";
};

struct RenderPipelineState final {
  RenderPipelineDesc desc;

  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

  std::vector<VkDescriptorSetLayout> lastDescriptorSetLayouts;
};

struct BeginRenderingDesc {
  VkRect2D renderArea = {};
  struct Attachment final {
    TextureHandle texture = {};
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE;
    VkImageView resolveImageView = VK_NULL_HANDLE;
    VkImageLayout resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearValue = {.color = {.float32 = {0.f, 0.f, 0.f, 0.f}}};
  };
  Attachment colorAttachments[RenderPipelineDesc::MAX_COLOR_ATTACHMENT];
  Attachment depthAttachment = {.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                .resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                .clearValue = {.depthStencil = {.depth = 1.f, .stencil = 0}}};
  Attachment stencilAttachment = {.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                  .resolveImageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                  .clearValue = {.depthStencil = {.depth = 1.f, .stencil = 0}}};
  uint32_t getNumColorAttachments() {
    uint32_t i = 0;
    while (i < RenderPipelineDesc::MAX_COLOR_ATTACHMENT && !colorAttachments[i].texture.empty())
      i++;
    return i;
  }
};

struct ViewportDesc final {
  float x = 0.f;
  float y = 0.f;
  float width = 0.f;
  float height = 0.f;
  float minDepth = 0.f;
  float maxDepth = 1.f;
};

class VulkanContext;

struct InstanceInitDesc final {
  bool enableValidation = false;
  std::vector<const char*> validationLayers;
  std::vector<const char*> extensions;
  bool enableDebugMassangerExt = false;
  bool enableValidationFeaturesExt = false;
  bool enableHeadlessSurfaceExt = false;
  std::vector<VkValidationFeatureEnableEXT> layerFeatures;
  std::vector<const char*> layerSettings;
};

struct DeviceInitDesc final {
  HWDeviceType deviceType = HWDeviceType::HWDeviceType_Integrated;
  std::vector<const char*> extensions;
  std::vector<DeviceFeature> fetures;
  bool graphicsQueue = false;
  bool computeQueue = false;
  bool transferQueue = false;
};

struct SwapchainInitDesc final {
  VulkanContext* context = nullptr;
  VkExtent2D swapchainSize = {};
  VkSurfaceKHR surface = VK_NULL_HANDLE;
};

struct ImmediateCommandsInitDesc final {
  VulkanContext* context = nullptr;
  uint32_t queueFamilyIndex = UINT32_MAX;
  const char* debugName = "";
};

struct DescriptorManagerInitDesc final {
  VulkanContext* context = nullptr;
};

struct TaskManagerInitDesc final {
  VulkanContext* context = nullptr;
};

struct ContextInitDesc final {
  uint32_t windowWidth = 0;
  uint32_t windowHeight = 0;
  InstanceInitDesc instanceDesc;
  DeviceInitDesc deviceDesc;
};

}  // namespace mrvk
