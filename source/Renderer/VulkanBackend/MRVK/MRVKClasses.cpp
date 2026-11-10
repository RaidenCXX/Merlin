#include "MRVKClasses.h"

#include "SPIRV-Reflect/spirv_reflect.h"
#include "VulkanStructs.h"
#include "VulkanUtility.h"
//
#include "VkMacro.h"
#include "Volk/volk.h"
#include "glsl/glslang_c_interface.h"
#include "scope_guard/scope_guard.hpp"
#include "slang/slang.h"
#include "vulkan/vulkan_core.h"

#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
static void printDeviceInfo(const mrvk::DeviceInitDesc& desc,
                            const mrvk::DeviceProperties& properties);

static void checkDeviceFeatures(std::vector<mrvk::DeviceFeature>& required,
                                mrvk::DeviceFeaturesChain& available,
                                mrvk::DeviceFeaturesChain& enabled);

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats,
                                                  mrvk::ColorSpace colorSpace);
static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
static VkImageUsageFlags chooseUsageFlags(VkPhysicalDevice pd, VkSurfaceKHR surface,
                                          VkFormat format);
static uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& caps);

//===============INSTANCE=================
void mrvk::Instance::init(VulkanContext* context, const InstanceInitDesc& desc) {
  ctx_ = context;
  desc_ = std::move(desc);

  // Volk init
  MRVK_VERIFY(volkInitialize());
  initInstance();
  volkLoadInstance(instance_);
  if (desc_.enableDebugMassangerExt)
    initDebugMessenger();
}

void mrvk::Instance::initInstance() {
  auto checkLayerProperty = [](const std::vector<const char*>& requiredLayers,
                               const std::vector<VkLayerProperties>& availableLayers) -> bool {
    for (const char* requiredLayerName : requiredLayers) {
      bool found = false;
      for (const VkLayerProperties& availableProperties : availableLayers) {
        if (!strcmp(availableProperties.layerName, requiredLayerName)) {
          found = true;
        }
      }
      if (!found) {
        MRVK_LOG_WARNING("Validation layer not suport: %s", requiredLayerName);
        return false;
      }
    }
    return true;
  };

  // Init and check support validation layers
  if (desc_.enableValidation) {
    uint32_t propertyCount = 0;
    vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
    std::vector<VkLayerProperties> availableProperties(propertyCount);
    vkEnumerateInstanceLayerProperties(&propertyCount, availableProperties.data());

    checkLayerProperty(desc_.validationLayers, availableProperties);
    std::vector<const char*> validationLayers(desc_.validationLayers.begin(),
                                              desc_.validationLayers.end());
  }

  // Init instance extension
  if (!desc_.enableHeadlessSurfaceExt) {
#if defined(LINUX)
  #if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    desc_.extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
  #else
    desc_.extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
  #endif  // VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(WINDOWS)
    desc_.extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif  // LINUX
  };
  if (desc_.enableDebugMassangerExt && desc_.enableValidation)
    desc_.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  if (desc_.enableValidationFeaturesExt && desc_.enableValidation)
    desc_.extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
  if (desc_.enableHeadlessSurfaceExt)
    desc_.extensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);

  // Validation layers fetures
  const VkValidationFeaturesEXT fetures = {
    .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
    .enabledValidationFeatureCount =
      desc_.layerFeatures.empty() ? 0u : (uint32_t)desc_.layerFeatures.size(),
    .pEnabledValidationFeatures = desc_.layerFeatures.data()};

  // Validation layers settings
  std::vector<VkLayerSettingEXT> settings(desc_.layerSettings.size());
  std::vector<VkBool32> settingVars(desc_.layerSettings.size());
  for (size_t i = 0; i < desc_.layerSettings.size(); i++) {
    settings[i] = MRVK_LAYER_SETTINGS_BOOL32(desc_.layerSettings[i], &settingVars[i]);
  }

  const VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
    .pNext = desc_.layerFeatures.empty() ? nullptr : &fetures,
    .settingCount = desc_.enableValidation ? (uint32_t)desc_.layerSettings.size() : 0u,
    .pSettings = settings.data()};

  // Instance creation
  const VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                     .pApplicationName = "Merlin/Vulkan",
                                     .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                     .pEngineName = "Merlin/Vulkan",
                                     .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                     .apiVersion = VK_API_VERSION_1_3};

  const VkInstanceCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                           .pNext = &layerSettingsCreateInfo,
                                           .pApplicationInfo = &appInfo,
                                           .enabledLayerCount =
                                             (uint32_t)desc_.validationLayers.size(),
                                           .ppEnabledLayerNames = desc_.validationLayers.data(),
                                           .enabledExtensionCount =
                                             (uint32_t)desc_.extensions.size(),
                                           .ppEnabledExtensionNames = desc_.extensions.data()};

  MRVK_VERIFY(vkCreateInstance(&createInfo, nullptr, &instance_));

  // Info print
  auto printInstanceInfo = [this]() {
    std::string msg;
    if (desc_.enableValidation) {
      msg.append("\tVulkan instance layers: \n");
      for (const char* layerName : desc_.validationLayers) {
        msg.append("\t\t");
        msg.append(layerName);
        msg.append("\n");
      }
    }
    if (!desc_.extensions.empty()) {
      msg.append("\tVulkan instance extension: \n");
      for (const char* extName : desc_.extensions) {
        msg.append("\t\t");
        msg.append(extName);
        msg.append("\n");
      }
    }
    if (!desc_.layerFeatures.empty()) {
      msg.append("\tVulkan instance layer features: \n");
      for (VkValidationFeatureEnableEXT feature : desc_.layerFeatures) {
        msg.append("\t\t");
        msg.append(convertValidationFeatureToString(feature));
        msg.append("\n");
      }
    }
    if (!desc_.layerSettings.empty()) {
      msg.append("\tVulkan instance layer settings: \n");
      for (const char* settingName : desc_.layerSettings) {
        msg.append("\t\t");
        msg.append(settingName);
        msg.append("\n");
      }
    }

    MRVK_LOG_INFO("\nInstance info: \n %s", msg.data());
  };
  printInstanceInfo();
}

void mrvk::Instance::initDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT createInfo{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT),
    .messageType =
      (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
       | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
    .pfnUserCallback = debugCallback,
    .pUserData = nullptr};

  MRVK_VERIFY(vkCreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  const char* typeStr = "GENERAL";
  if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    typeStr = "VALIDATION";
  } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    typeStr = "PERFORMANCE";
  }

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      MRVK_LOG_ERROR("%s : %s\n", typeStr, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      MRVK_LOG_WARNING("%s : %s\n", typeStr, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      MRVK_LOG_INFO("%s : %s\n", typeStr, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    default:
      MRVK_LOG_INFO("%s : %s\n", typeStr, pCallbackData->pMessage);
      break;
  }
  return VK_FALSE;
}

//
//================DEVICE=====================
//
void mrvk::Device::init(VulkanContext* context, DeviceInitDesc desc) {
  ctx_ = context;
  desc_ = std::move(desc);

  initDevice();
}

void mrvk::Device::initDevice() {
  // Searching for suitable devices
  std::array<HWDeviceDesc, 3> outDevices;
  uint32_t numCampatibleDevices =
    queryDevices(desc_.deviceType, outDevices.data(), outDevices.size());

  if (numCampatibleDevices == 0)
    MRVK_LOG_ERROR("0 Campatible devices");

  deviceDesc_ = outDevices[0];

  // All device extension and layer extension
  std::vector<VkExtensionProperties> allDeviceExtension;
  getPhysicalDeviceProps(deviceDesc_.pd, allDeviceExtension);
  if (ctx_->instance_.getDesc().enableValidation) {
    for (const char* layer : ctx_->instance_.getDesc().validationLayers) {
      getPhysicalDeviceProps(deviceDesc_.pd, allDeviceExtension, layer);
    }
  }

  // Check, if the device  has the require extensions
  std::string missingExtension;
  for (const char* reqExt : desc_.extensions) {
    bool found = false;
    for (VkExtensionProperties& avaExt : allDeviceExtension) {
      if (strcmp(reqExt, avaExt.extensionName) == 0)
        found = true;
    }
    if (!found) {
      missingExtension.append("\n   ");
      missingExtension.append(reqExt);
    }
  }
  if (!missingExtension.empty())
    MRVK_LOG_ERROR("Extension not supported. %s", missingExtension.data());

  // Features
  DeviceFeaturesChain allDeviceFeatures;
  allDeviceFeatures.linkChain();
  vkGetPhysicalDeviceFeatures2(deviceDesc_.pd, &allDeviceFeatures.features10);
  features_.linkChain();
  checkDeviceFeatures(desc_.fetures, allDeviceFeatures, features_);

  // Queue
  const float queuePriority = 1.0f;
  std::array<VkDeviceQueueCreateInfo, MAX_QUEUE_COUNT> ciQueue;
  uint32_t queueCount = 0;
  if (desc_.graphicsQueue) {
    queue_.graphicsQueueFamilyIndex = findQueueFamilyIndex(deviceDesc_.pd, VK_QUEUE_GRAPHICS_BIT);

    bool found = false;

    for (uint32_t i = 0; i < queueCount; ++i) {
      if (ciQueue[i].queueFamilyIndex == queue_.graphicsQueueFamilyIndex) {
        found = true;
        break;
      }
    }

    if (!found) {
      ciQueue[queueCount++] = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                               .queueFamilyIndex = queue_.graphicsQueueFamilyIndex,
                               .queueCount = 1,
                               .pQueuePriorities = &queuePriority};
    }
  }

  if (desc_.computeQueue) {
    queue_.computeQueueFamilyIndex = findQueueFamilyIndex(deviceDesc_.pd, VK_QUEUE_COMPUTE_BIT);

    bool found = false;

    for (uint32_t i = 0; i < queueCount; ++i) {
      if (ciQueue[i].queueFamilyIndex == queue_.computeQueueFamilyIndex) {
        found = true;
        break;
      }
    }

    if (!found) {
      ciQueue[queueCount++] = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                               .queueFamilyIndex = queue_.computeQueueFamilyIndex,
                               .queueCount = 1,
                               .pQueuePriorities = &queuePriority};
    }
  }

  if (desc_.transferQueue) {
    queue_.transferQueueFamilyIndex = findQueueFamilyIndex(deviceDesc_.pd, VK_QUEUE_TRANSFER_BIT);

    bool found = false;

    for (uint32_t i = 0; i < queueCount; ++i) {
      if (ciQueue[i].queueFamilyIndex == queue_.transferQueueFamilyIndex) {
        found = true;
        break;
      }
    }

    if (!found) {
      ciQueue[queueCount++] = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                               .queueFamilyIndex = queue_.transferQueueFamilyIndex,
                               .queueCount = 1,
                               .pQueuePriorities = &queuePriority};
    }
  }

  // Get physical device info
  vkGetPhysicalDeviceProperties2(deviceDesc_.pd, &properties_.properties);

  // create Info
  features_.linkChain();
  const VkDeviceCreateInfo ciInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                     .pNext = &features_.features10,
                                     .queueCreateInfoCount = queueCount,
                                     .pQueueCreateInfos = ciQueue.data(),
                                     .enabledExtensionCount = (uint32_t)desc_.extensions.size(),
                                     .ppEnabledExtensionNames = desc_.extensions.data(),
                                     .pEnabledFeatures = nullptr};

  MRVK_VERIFY(vkCreateDevice(deviceDesc_.pd, &ciInfo, nullptr, &device_));
  volkLoadDevice(device_);

  if (desc_.graphicsQueue) {
    vkGetDeviceQueue(device_, queue_.graphicsQueueFamilyIndex, 0, &queue_.graphicsQueue);
  }

  if (desc_.computeQueue) {
    vkGetDeviceQueue(device_, queue_.computeQueueFamilyIndex, 0, &queue_.computeQueue);
  }

  if (desc_.transferQueue) {
    vkGetDeviceQueue(device_, queue_.transferQueueFamilyIndex, 0, &queue_.transferQueue);
  }

  printDeviceInfo(desc_, properties_);
}

uint32_t mrvk::Device::queryDevices(HWDeviceType deviceType, HWDeviceDesc* outDevices,
                                    uint32_t maxOutDevices) {
  VkInstance inst = ctx_->instance_.getVkInstance();
  uint32_t deviceCount = 0;

  MRVK_VERIFY(vkEnumeratePhysicalDevices(inst, &deviceCount, nullptr));

  std::vector<VkPhysicalDevice> vkDevices(deviceCount);

  MRVK_VERIFY(vkEnumeratePhysicalDevices(inst, &deviceCount, vkDevices.data()));
  const HWDeviceType desiredDeviceType = deviceType;
  uint32_t numCompatibleDevices = 0;

  for (uint32_t i = 0; i < deviceCount; ++i) {
    VkPhysicalDevice physicalDevice = vkDevices[i];

    VkPhysicalDeviceProperties deviceProperties{};

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    const HWDeviceType deviceType = convertVulkanDeviceTypeToMRVK(deviceProperties.deviceType);

    if (desiredDeviceType != HWDeviceType_Software && desiredDeviceType != deviceType) {
      continue;
    }

    if (outDevices && numCompatibleDevices < maxOutDevices) {
      outDevices[numCompatibleDevices] = {
        .pd = physicalDevice,
        .type = deviceType,
      };

      std::strncpy(outDevices[numCompatibleDevices].name, deviceProperties.deviceName,
                   HWDeviceDesc::MAX_PHYSICAL_DEVICE_NAME_SIZE - 1);

      outDevices[numCompatibleDevices].name[HWDeviceDesc::MAX_PHYSICAL_DEVICE_NAME_SIZE - 1] = '\0';
    }

    ++numCompatibleDevices;
  }

  return numCompatibleDevices;
}

void mrvk::Device::getPhysicalDeviceProps(VkPhysicalDevice physicalDevice,
                                          std::vector<VkExtensionProperties>& extensions,
                                          const char* validationLayer) {
  uint32_t extensionsCount = 0;
  MRVK_VERIFY(vkEnumerateDeviceExtensionProperties(physicalDevice, validationLayer,
                                                   &extensionsCount, nullptr));
  std::vector<VkExtensionProperties> p(extensionsCount);
  MRVK_VERIFY(vkEnumerateDeviceExtensionProperties(physicalDevice, validationLayer,
                                                   &extensionsCount, p.data()));
  extensions.insert(extensions.end(), p.begin(), p.end());
}

uint32_t mrvk::Device::findQueueFamilyIndex(VkPhysicalDevice dev, VkQueueFlagBits flags) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties2(dev, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties2> props(queueFamilyCount);
  for (VkQueueFamilyProperties2& p : props) {
    p.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
  }

  vkGetPhysicalDeviceQueueFamilyProperties2(dev, &queueFamilyCount, props.data());

  auto findDedicatedQueueFamilyIndex = [&props](VkQueueFlags require,
                                                VkQueueFlags avoid) -> uint32_t {
    for (uint32_t i = 0; i != props.size(); ++i) {
      const VkQueueFamilyProperties& p = props[i].queueFamilyProperties;
      const bool isSuitable = (p.queueFlags & require) == require;
      const bool isDedicated = (p.queueFlags & avoid) == 0;
      if (p.queueCount && isSuitable && isDedicated)
        return i;
    }
    return mrvk::DeviceQueues::INVALID;
  };

  if (flags & VK_QUEUE_COMPUTE_BIT) {
    const uint32_t q = findDedicatedQueueFamilyIndex(flags, VK_QUEUE_GRAPHICS_BIT);
    if (q != mrvk::DeviceQueues::INVALID)
      return q;
  }

  if (flags & VK_QUEUE_TRANSFER_BIT) {
    const uint32_t q = findDedicatedQueueFamilyIndex(flags, VK_QUEUE_GRAPHICS_BIT);
    if (q != mrvk::DeviceQueues::INVALID)
      return q;
  }

  return findDedicatedQueueFamilyIndex(flags, 0);
}

static void printDeviceInfo(const mrvk::DeviceInitDesc& desc,
                            const mrvk::DeviceProperties& properties) {
  std::string msg;

  const uint32_t apiVersion = properties.properties.properties.apiVersion;
  VkPhysicalDeviceDriverProperties* driverPr =
    (VkPhysicalDeviceDriverProperties*)properties.properties.pNext;

  msg.append("\n\tVulkan physical device: ");
  msg.append(properties.properties.properties.deviceName);
  msg.append("\n");

  msg.append("\tApi version: ");
  msg.append(std::to_string(VK_API_VERSION_MAJOR(apiVersion)).append("."));
  msg.append(std::to_string(VK_API_VERSION_MINOR(apiVersion)).append("."));
  msg.append(std::to_string(VK_API_VERSION_PATCH(apiVersion)).append("."));
  msg.append(std::to_string(VK_API_VERSION_VARIANT(apiVersion)));
  msg.append("\n\tDriver info: ");
  msg.append(driverPr->driverName);
  msg.append(". ");
  msg.append(driverPr->driverInfo);
  msg.append("\n\tVulkan physical device extension:");
  for (const char* ext : desc.extensions) {
    msg.append(std::string("\n\t\t").append(ext));
  }

  msg.append("\n\tVulkan physical device enabled features:");
  for (const mrvk::DeviceFeature& feature : desc.fetures) {
    msg.append("\n\t\t");
    msg.append(deviceFeatureToString(feature));
  }

  MRVK_LOG_INFO("\nDevice info:%s", msg.c_str());
}

static void checkDeviceFeatures(std::vector<mrvk::DeviceFeature>& required,
                                mrvk::DeviceFeaturesChain& available,
                                mrvk::DeviceFeaturesChain& enabled) {
#define CHECK_FEATURE(featureName, VkFeatureName)                                                  \
  case mrvk::featureName:                                                                          \
    enabled.VkFeatureName = features.VkFeatureName == VK_TRUE ? VK_TRUE : VK_FALSE;                \
    return enabled.VkFeatureName == VK_TRUE ? nullptr                                              \
                                            : mrvk::deviceFeatureToString(requiredFeature);

  auto checkFeature = [&](mrvk::DeviceFeature requiredFeature,
                          mrvk::DeviceFeaturesChain& availableFeatures,
                          mrvk::DeviceFeaturesChain& enabledFeatures) -> const char* {
    const uint32_t feature = static_cast<uint32_t>(requiredFeature);
    uint32_t required = (uint32_t)requiredFeature;
    // Vulkan 1.0
    if (feature < 100) {
      const VkPhysicalDeviceFeatures& features = availableFeatures.features10.features;
      VkPhysicalDeviceFeatures& enabled = enabledFeatures.features10.features;

      switch (required) {
        CHECK_FEATURE(DeviceFeatures_10_RobustBufferAccess, robustBufferAccess);
        CHECK_FEATURE(DeviceFeatures_10_FullDrawIndexUint32, fullDrawIndexUint32);
        CHECK_FEATURE(DeviceFeatures_10_ImageCubeArray, imageCubeArray);
        CHECK_FEATURE(DeviceFeatures_10_IndependentBlend, independentBlend);
        CHECK_FEATURE(DeviceFeatures_10_GeometryShader, geometryShader);
        CHECK_FEATURE(DeviceFeatures_10_TessellationShader, tessellationShader);
        CHECK_FEATURE(DeviceFeatures_10_SampleRateShading, sampleRateShading);
        CHECK_FEATURE(DeviceFeatures_10_DualSrcBlend, dualSrcBlend);
        CHECK_FEATURE(DeviceFeatures_10_LogicOp, logicOp);
        CHECK_FEATURE(DeviceFeatures_10_MultiDrawIndirect, multiDrawIndirect);
        CHECK_FEATURE(DeviceFeatures_10_DrawIndirectFirstInstance, drawIndirectFirstInstance);
        CHECK_FEATURE(DeviceFeatures_10_DepthClamp, depthClamp);
        CHECK_FEATURE(DeviceFeatures_10_DepthBiasClamp, depthBiasClamp);
        CHECK_FEATURE(DeviceFeatures_10_FillModeNonSolid, fillModeNonSolid);
        CHECK_FEATURE(DeviceFeatures_10_DepthBounds, depthBounds);
        CHECK_FEATURE(DeviceFeatures_10_WideLines, wideLines);
        CHECK_FEATURE(DeviceFeatures_10_LargePoints, largePoints);
        CHECK_FEATURE(DeviceFeatures_10_AlphaToOne, alphaToOne);
        CHECK_FEATURE(DeviceFeatures_10_MultiViewport, multiViewport);
        CHECK_FEATURE(DeviceFeatures_10_SamplerAnisotropy, samplerAnisotropy);
        CHECK_FEATURE(DeviceFeatures_10_TextureCompressionETC2, textureCompressionETC2);
        CHECK_FEATURE(DeviceFeatures_10_TextureCompressionASTC_LDR, textureCompressionASTC_LDR);
        CHECK_FEATURE(DeviceFeatures_10_TextureCompressionBC, textureCompressionBC);
        CHECK_FEATURE(DeviceFeatures_10_OcclusionQueryPrecise, occlusionQueryPrecise);
        CHECK_FEATURE(DeviceFeatures_10_PipelineStatisticsQuery, pipelineStatisticsQuery);
        CHECK_FEATURE(DeviceFeatures_10_VertexPipelineStoresAndAtomics,
                      vertexPipelineStoresAndAtomics);
        CHECK_FEATURE(DeviceFeatures_10_FragmentStoresAndAtomics, fragmentStoresAndAtomics);
        CHECK_FEATURE(DeviceFeatures_10_ShaderTessellationAndGeometryPointSize,
                      shaderTessellationAndGeometryPointSize);
        CHECK_FEATURE(DeviceFeatures_10_ShaderImageGatherExtended, shaderImageGatherExtended);
        CHECK_FEATURE(DeviceFeatures_10_ShaderStorageImageExtendedFormats,
                      shaderStorageImageExtendedFormats);
        CHECK_FEATURE(DeviceFeatures_10_ShaderStorageImageMultisample,
                      shaderStorageImageMultisample);
        CHECK_FEATURE(DeviceFeatures_10_ShaderStorageImageReadWithoutFormat,
                      shaderStorageImageReadWithoutFormat);
        CHECK_FEATURE(DeviceFeatures_10_ShaderStorageImageWriteWithoutFormat,
                      shaderStorageImageWriteWithoutFormat);
        CHECK_FEATURE(DeviceFeatures_10_ShaderUniformBufferArrayDynamicIndexing,
                      shaderUniformBufferArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_10_ShaderSampledImageArrayDynamicIndexing,
                      shaderSampledImageArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_10_ShaderStorageBufferArrayDynamicIndexing,
                      shaderStorageBufferArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_10_ShaderStorageImageArrayDynamicIndexing,
                      shaderStorageImageArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_10_ShaderClipDistance, shaderClipDistance);
        CHECK_FEATURE(DeviceFeatures_10_ShaderCullDistance, shaderCullDistance);
        CHECK_FEATURE(DeviceFeatures_10_ShaderFloat64, shaderFloat64);
        CHECK_FEATURE(DeviceFeatures_10_ShaderInt64, shaderInt64);
        CHECK_FEATURE(DeviceFeatures_10_ShaderInt16, shaderInt16);
        CHECK_FEATURE(DeviceFeatures_10_ShaderResourceResidency, shaderResourceResidency);
        CHECK_FEATURE(DeviceFeatures_10_ShaderResourceMinLod, shaderResourceMinLod);
        CHECK_FEATURE(DeviceFeatures_10_SparseBinding, sparseBinding);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidencyBuffer, sparseResidencyBuffer);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidencyImage2D, sparseResidencyImage2D);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidencyImage3D, sparseResidencyImage3D);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidency2Samples, sparseResidency2Samples);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidency4Samples, sparseResidency4Samples);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidency8Samples, sparseResidency8Samples);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidency16Samples, sparseResidency16Samples);
        CHECK_FEATURE(DeviceFeatures_10_SparseResidencyAliased, sparseResidencyAliased);
        CHECK_FEATURE(DeviceFeatures_10_VariableMultisampleRate, variableMultisampleRate);
        CHECK_FEATURE(DeviceFeatures_10_InheritedQueries, inheritedQueries);
      }
    }

    // Vulkan 1.1
    else if (feature >= 100 && feature < 200) {
      const VkPhysicalDeviceVulkan11Features& features = availableFeatures.features11;
      VkPhysicalDeviceVulkan11Features& enabled = enabledFeatures.features11;

      switch (required) {
        CHECK_FEATURE(DeviceFeatures_11_StorageBuffer16BitAccess, storageBuffer16BitAccess);
        CHECK_FEATURE(DeviceFeatures_11_UniformAndStorageBuffer16BitAccess,
                      uniformAndStorageBuffer16BitAccess);
        CHECK_FEATURE(DeviceFeatures_11_StoragePushConstant16, storagePushConstant16);
        CHECK_FEATURE(DeviceFeatures_11_StorageInputOutput16, storageInputOutput16);
        CHECK_FEATURE(DeviceFeatures_11_Multiview, multiview);
        CHECK_FEATURE(DeviceFeatures_11_MultiviewGeometryShader, multiviewGeometryShader);
        CHECK_FEATURE(DeviceFeatures_11_MultiviewTessellationShader, multiviewTessellationShader);
        CHECK_FEATURE(DeviceFeatures_11_VariablePointersStorageBuffer,
                      variablePointersStorageBuffer);
        CHECK_FEATURE(DeviceFeatures_11_VariablePointers, variablePointers);
        CHECK_FEATURE(DeviceFeatures_11_ProtectedMemory, protectedMemory);
        CHECK_FEATURE(DeviceFeatures_11_SamplerYcbcrConversion, samplerYcbcrConversion);
        CHECK_FEATURE(DeviceFeatures_11_ShaderDrawParameters, shaderDrawParameters);
      }
    }

    // Vulkan 1.2
    else if (feature >= 200 && feature < 300) {
      const VkPhysicalDeviceVulkan12Features& features = availableFeatures.features12;
      VkPhysicalDeviceVulkan12Features& enabled = enabledFeatures.features12;

      switch (required) {
        CHECK_FEATURE(DeviceFeatures_12_SamplerMirrorClampToEdge, samplerMirrorClampToEdge);
        CHECK_FEATURE(DeviceFeatures_12_DrawIndirectCount, drawIndirectCount);
        CHECK_FEATURE(DeviceFeatures_12_StorageBuffer8BitAccess, storageBuffer8BitAccess);
        CHECK_FEATURE(DeviceFeatures_12_UniformAndStorageBuffer8BitAccess,
                      uniformAndStorageBuffer8BitAccess);
        CHECK_FEATURE(DeviceFeatures_12_StoragePushConstant8, storagePushConstant8);
        CHECK_FEATURE(DeviceFeatures_12_ShaderBufferInt64Atomics, shaderBufferInt64Atomics);
        CHECK_FEATURE(DeviceFeatures_12_ShaderSharedInt64Atomics, shaderSharedInt64Atomics);
        CHECK_FEATURE(DeviceFeatures_12_ShaderFloat16, shaderFloat16);
        CHECK_FEATURE(DeviceFeatures_12_ShaderInt8, shaderInt8);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorIndexing, descriptorIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderInputAttachmentArrayDynamicIndexing,
                      shaderInputAttachmentArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderUniformTexelBufferArrayDynamicIndexing,
                      shaderUniformTexelBufferArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderStorageTexelBufferArrayDynamicIndexing,
                      shaderStorageTexelBufferArrayDynamicIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderUniformBufferArrayNonUniformIndexing,
                      shaderUniformBufferArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderSampledImageArrayNonUniformIndexing,
                      shaderSampledImageArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderStorageBufferArrayNonUniformIndexing,
                      shaderStorageBufferArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderStorageImageArrayNonUniformIndexing,
                      shaderStorageImageArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderInputAttachmentArrayNonUniformIndexing,
                      shaderInputAttachmentArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderUniformTexelBufferArrayNonUniformIndexing,
                      shaderUniformTexelBufferArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_ShaderStorageTexelBufferArrayNonUniformIndexing,
                      shaderStorageTexelBufferArrayNonUniformIndexing);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingUniformBufferUpdateAfterBind,
                      descriptorBindingUniformBufferUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingSampledImageUpdateAfterBind,
                      descriptorBindingSampledImageUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingStorageImageUpdateAfterBind,
                      descriptorBindingStorageImageUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingStorageBufferUpdateAfterBind,
                      descriptorBindingStorageBufferUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingUniformTexelBufferUpdateAfterBind,
                      descriptorBindingUniformTexelBufferUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingStorageTexelBufferUpdateAfterBind,
                      descriptorBindingStorageTexelBufferUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingUpdateUnusedWhilePending,
                      descriptorBindingUpdateUnusedWhilePending);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingPartiallyBound,
                      descriptorBindingPartiallyBound);
        CHECK_FEATURE(DeviceFeatures_12_DescriptorBindingVariableDescriptorCount,
                      descriptorBindingVariableDescriptorCount);
        CHECK_FEATURE(DeviceFeatures_12_RuntimeDescriptorArray, runtimeDescriptorArray);
        CHECK_FEATURE(DeviceFeatures_12_SamplerFilterMinmax, samplerFilterMinmax);
        CHECK_FEATURE(DeviceFeatures_12_ScalarBlockLayout, scalarBlockLayout);
        CHECK_FEATURE(DeviceFeatures_12_ImagelessFramebuffer, imagelessFramebuffer);
        CHECK_FEATURE(DeviceFeatures_12_UniformBufferStandardLayout, uniformBufferStandardLayout);
        CHECK_FEATURE(DeviceFeatures_12_ShaderSubgroupExtendedTypes, shaderSubgroupExtendedTypes);
        CHECK_FEATURE(DeviceFeatures_12_SeparateDepthStencilLayouts, separateDepthStencilLayouts);
        CHECK_FEATURE(DeviceFeatures_12_HostQueryReset, hostQueryReset);
        CHECK_FEATURE(DeviceFeatures_12_TimelineSemaphore, timelineSemaphore);
        CHECK_FEATURE(DeviceFeatures_12_BufferDeviceAddress, bufferDeviceAddress);
        CHECK_FEATURE(DeviceFeatures_12_BufferDeviceAddressCaptureReplay,
                      bufferDeviceAddressCaptureReplay);
        CHECK_FEATURE(DeviceFeatures_12_BufferDeviceAddressMultiDevice,
                      bufferDeviceAddressMultiDevice);
        CHECK_FEATURE(DeviceFeatures_12_VulkanMemoryModel, vulkanMemoryModel);
        CHECK_FEATURE(DeviceFeatures_12_VulkanMemoryModelDeviceScope, vulkanMemoryModelDeviceScope);
        CHECK_FEATURE(DeviceFeatures_12_VulkanMemoryModelAvailabilityVisibilityChains,
                      vulkanMemoryModelAvailabilityVisibilityChains);
        CHECK_FEATURE(DeviceFeatures_12_ShaderOutputViewportIndex, shaderOutputViewportIndex);
        CHECK_FEATURE(DeviceFeatures_12_ShaderOutputLayer, shaderOutputLayer);
        CHECK_FEATURE(DeviceFeatures_12_SubgroupBroadcastDynamicId, subgroupBroadcastDynamicId);
      }
    }

    // Vulkan 1.3
    else if (feature >= 300 && feature < 400) {
      const VkPhysicalDeviceVulkan13Features& features = availableFeatures.features13;
      VkPhysicalDeviceVulkan13Features& enabled = enabledFeatures.features13;

      switch (required) {
        CHECK_FEATURE(DeviceFeatures_13_RobustImageAccess, robustImageAccess);
        CHECK_FEATURE(DeviceFeatures_13_InlineUniformBlock, inlineUniformBlock);
        CHECK_FEATURE(DeviceFeatures_13_DescriptorBindingInlineUniformBlockUpdateAfterBind,
                      descriptorBindingInlineUniformBlockUpdateAfterBind);
        CHECK_FEATURE(DeviceFeatures_13_PipelineCreationCacheControl, pipelineCreationCacheControl);
        CHECK_FEATURE(DeviceFeatures_13_PrivateData, privateData);
        CHECK_FEATURE(DeviceFeatures_13_ShaderDemoteToHelperInvocation,
                      shaderDemoteToHelperInvocation);
        CHECK_FEATURE(DeviceFeatures_13_ShaderTerminateInvocation, shaderTerminateInvocation);
        CHECK_FEATURE(DeviceFeatures_13_SubgroupSizeControl, subgroupSizeControl);
        CHECK_FEATURE(DeviceFeatures_13_ComputeFullSubgroups, computeFullSubgroups);
        CHECK_FEATURE(DeviceFeatures_13_Synchronization2, synchronization2);
        CHECK_FEATURE(DeviceFeatures_13_TextureCompressionASTC_HDR, textureCompressionASTC_HDR);
        CHECK_FEATURE(DeviceFeatures_13_ShaderZeroInitializeWorkgroupMemory,
                      shaderZeroInitializeWorkgroupMemory);
        CHECK_FEATURE(DeviceFeatures_13_DynamicRendering, dynamicRendering);
        CHECK_FEATURE(DeviceFeatures_13_ShaderIntegerDotProduct, shaderIntegerDotProduct);
        CHECK_FEATURE(DeviceFeatures_13_Maintenance4, maintenance4);
      }
    }

    // Vulkan 1.4
    else if (feature >= 400 && feature < 500) {
      const VkPhysicalDeviceVulkan14Features& features = availableFeatures.features14;
      VkPhysicalDeviceVulkan14Features& enabled = enabledFeatures.features14;

      switch (required) {
        CHECK_FEATURE(DeviceFeatures_14_GlobalPriorityQuery, globalPriorityQuery);
        CHECK_FEATURE(DeviceFeatures_14_ShaderSubgroupRotate, shaderSubgroupRotate);
        CHECK_FEATURE(DeviceFeatures_14_ShaderSubgroupRotateClustered,
                      shaderSubgroupRotateClustered);
        CHECK_FEATURE(DeviceFeatures_14_ShaderFloatControls2, shaderFloatControls2);
        CHECK_FEATURE(DeviceFeatures_14_ShaderExpectAssume, shaderExpectAssume);
        CHECK_FEATURE(DeviceFeatures_14_RectangularLines, rectangularLines);
        CHECK_FEATURE(DeviceFeatures_14_BresenhamLines, bresenhamLines);
        CHECK_FEATURE(DeviceFeatures_14_SmoothLines, smoothLines);
        CHECK_FEATURE(DeviceFeatures_14_StippledRectangularLines, stippledRectangularLines);
        CHECK_FEATURE(DeviceFeatures_14_StippledBresenhamLines, stippledBresenhamLines);
        CHECK_FEATURE(DeviceFeatures_14_StippledSmoothLines, stippledSmoothLines);
        CHECK_FEATURE(DeviceFeatures_14_VertexAttributeInstanceRateDivisor,
                      vertexAttributeInstanceRateDivisor);
        CHECK_FEATURE(DeviceFeatures_14_VertexAttributeInstanceRateZeroDivisor,
                      vertexAttributeInstanceRateZeroDivisor);
        CHECK_FEATURE(DeviceFeatures_14_IndexTypeUint8, indexTypeUint8);
        CHECK_FEATURE(DeviceFeatures_14_DynamicRenderingLocalRead, dynamicRenderingLocalRead);
        CHECK_FEATURE(DeviceFeatures_14_Maintenance5, maintenance5);
        CHECK_FEATURE(DeviceFeatures_14_Maintenance6, maintenance6);
        CHECK_FEATURE(DeviceFeatures_14_PipelineProtectedAccess, pipelineProtectedAccess);
        CHECK_FEATURE(DeviceFeatures_14_PipelineRobustness, pipelineRobustness);
        CHECK_FEATURE(DeviceFeatures_14_HostImageCopy, hostImageCopy);
        CHECK_FEATURE(DeviceFeatures_14_PushDescriptor, pushDescriptor);
      }
    }

    // VK_KHR_swapchain_maintenance1
    else if (requiredFeature == mrvk::DeviceFeatures_KHR_SwapchainMaintenance1) {
      enabledFeatures.swapchainMaintence1.swapchainMaintenance1 =
        availableFeatures.swapchainMaintence1.swapchainMaintenance1 == VK_TRUE ? VK_TRUE : VK_FALSE;
      return enabledFeatures.swapchainMaintence1.swapchainMaintenance1 == VK_TRUE
               ? nullptr
               : mrvk::deviceFeatureToString(requiredFeature);
    }
    return "UnknownDeviceFeature";
  };
#undef CHECK_FEATURE

  std::string missingFeatures = {};
  for (mrvk::DeviceFeature feature : required) {
    const char* featureName = checkFeature(feature, available, enabled);
    if (featureName != nullptr) {
      missingFeatures.append(featureName);
      missingFeatures.append("\n");
    }
  }
  if (!missingFeatures.empty())
    MRVK_LOG_ERROR("Missing device feature: \n %s", missingFeatures.data());
}

const VkPhysicalDevice mrvk::Device::getPhysicalDevice() const {
  if (deviceDesc_.pd == VK_NULL_HANDLE)
    MRVK_LOG_ERROR("The device needs to be initialized.");
  return reinterpret_cast<VkPhysicalDevice>(deviceDesc_.pd);
}

//
//
//================SWAPCHAIN=================
//
//

void mrvk::VulkanSwapchain::init(mrvk::SwapchainInitDesc desc) {
  ctx_ = desc.context;

  std::vector<VkFormat> deviceDepthFormat;
  std::vector<VkSurfaceFormatKHR> deviceSurfaceFormats;
  std::vector<VkPresentModeKHR> devicePresentModes;

  querySurfaceCapabilities(deviceDepthFormat, surfaceCapabilities_, deviceSurfaceFormats,
                           devicePresentModes);

  surfaceFormat_ = chooseSwapSurfaceFormat(deviceSurfaceFormats, ColorSpace_SRGB_NONLINEAR);

  VkPhysicalDevice pd = ctx_->device_.getPhysicalDevice();
  VkImageUsageFlags usageFlags = chooseUsageFlags(pd, surface_, surfaceFormat_.format);

  presentMode_ = chooseSwapPresentMode(devicePresentModes);

  uint32_t imageCount = chooseSwapImageCount(surfaceCapabilities_);

  // Check support presentation for graphics queue
  uint32_t graphicsQueueIndex = ctx_->device_.getDeviceQueues().graphicsQueueFamilyIndex;
  VkBool32 queueFamilySupportsPresentation = VK_FALSE;
  vkGetPhysicalDeviceSurfaceSupportKHR(pd, graphicsQueueIndex, surface_,
                                       &queueFamilySupportsPresentation);

  const bool isCompositeAlphaOpaqueSupported =
    surfaceCapabilities_.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  const VkSwapchainCreateInfoKHR ci = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                       .surface = surface_,
                                       .minImageCount = imageCount,
                                       .imageFormat = surfaceFormat_.format,
                                       .imageColorSpace = surfaceFormat_.colorSpace,
                                       .imageExtent = {.width = desc.swapchainSize.width,
                                                       .height = desc.swapchainSize.height},
                                       .imageArrayLayers = 1,
                                       .imageUsage = usageFlags,
                                       .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                       .queueFamilyIndexCount = 1,
                                       .pQueueFamilyIndices = &graphicsQueueIndex,
                                       .preTransform = surfaceCapabilities_.currentTransform,
                                       .compositeAlpha = isCompositeAlphaOpaqueSupported
                                                           ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
                                                           : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
                                       .presentMode = presentMode_,
                                       .clipped = VK_TRUE,
                                       .oldSwapchain = VK_NULL_HANDLE

  };
  const VkDevice dev = ctx_->device_.getVkDevice();
  MRVK_VERIFY(vkCreateSwapchainKHR(dev, &ci, nullptr, &swapchain_));

  // Swapchain image view
  std::array<VkImage, 5> swapchainImages;
  uint32_t numSwapchainImages;
  vkGetSwapchainImagesKHR(dev, swapchain_, &numSwapchainImages, nullptr);
  if (numSwapchainImages > MRVK_MAX_SWAPCHAIN_IMAGES)
    numSwapchainImages = MRVK_MAX_SWAPCHAIN_IMAGES;
  vkGetSwapchainImagesKHR(dev, swapchain_, &numSwapchainImages, swapchainImages.data());

  swapchainTextures_.resize(numSwapchainImages);
  VkImageViewCreateInfo imgViewCi = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                     .pNext = nullptr,
                                     .flags = 0,
                                     .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                     .format = surfaceFormat_.format,
                                     .components = {.r = VK_COMPONENT_SWIZZLE_R,
                                                    .g = VK_COMPONENT_SWIZZLE_G,
                                                    .b = VK_COMPONENT_SWIZZLE_B,
                                                    .a = VK_COMPONENT_SWIZZLE_A},
                                     .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                          .baseMipLevel = 0,
                                                          .levelCount = 1,
                                                          .baseArrayLayer = 0,
                                                          .layerCount = 1}};
  size_t i = 0;
  for (size_t i = 0; i < numSwapchainImages; ++i) {
    imgViewCi.image = swapchainImages[i];

    VkImageView view;
    vkCreateImageView(dev, &imgViewCi, nullptr, &view);

    swapchainTextures_[i] = ctx_->texturePool_.create(VulkanImage{
      .vkImage = swapchainImages[i],
      .vkImageView = view,
      .vkUsageFlags = usageFlags,
      .vkImageFormat = surfaceFormat_.format,
    });
  }

  acquireSemaphores_.resize(numSwapchainImages);
  presentFence_.resize(numSwapchainImages);
  frameSlots_.resize(numSwapchainImages);
  for (uint32_t i = 0; i < numSwapchainImages; i++) {
    acquireSemaphores_[i] = createSemaphore(dev, "Semaphone: swapchain-acquire");
    presentFence_[i] = createFence(dev, "Fance: swapchin-present");
    // vkResetFences(dev, 1, &presentFence_[i]);
  }
}

mrvk::TextureHandle mrvk::VulkanSwapchain::getCurrentTexture() {
  VkDevice dev = ctx_->device_.getVkDevice();
  VulkanImmediateCommands& immediate = ctx_->immediateCommands_;

  if (!getNextImage_)
    return swapchainTextures_[acquireImageIndex_];

  FrameSlot& currentSlot = frameSlots_[swapchainFrame_];
  currentSlot.slotIndex = swapchainFrame_;
  currentSlot.presentCompliteFence = presentFence_[swapchainFrame_];
  currentSlot.acquireCompliteSemaphore = acquireSemaphores_[swapchainFrame_];

  vkWaitForFences(dev, 1, &currentSlot.presentCompliteFence, VK_TRUE, UINT64_MAX);

  // When the image becomes available, the semaphore and barrier will be set to the signaled state
  VkResult r =
    vkAcquireNextImageKHR(dev, swapchain_, UINT64_MAX, currentSlot.acquireCompliteSemaphore,
                          nullptr, &acquireImageIndex_);

  if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) {
    MRVK_VERIFY(r);
  }
  if (r != VK_ERROR_OUT_OF_DATE_KHR) {}
  // The swapchein image mast be writen, befor request next image
  getNextImage_ = false;

  // Submit a semaphore to Vulkan for immediate commands, which must be waited on before
  // submission
  immediate.waitSemaphore(currentSlot.acquireCompliteSemaphore);

  return swapchainTextures_[acquireImageIndex_];
}

mrvk::Result mrvk::VulkanSwapchain::present(VkSemaphore waitSemaphore) {
  VkDevice dev = ctx_->device_.getVkDevice();
  VkQueue queue = ctx_->device_.getDeviceQueues().graphicsQueue;

  FrameSlot& currentSlot = frameSlots_[swapchainFrame_];

  // Reset present fence in current frame
  vkResetFences(dev, 1, &currentSlot.presentCompliteFence);

  const VkSwapchainPresentFenceInfoKHR presentInfoFence = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR,
    .pNext = nullptr,
    .swapchainCount = 1,
    .pFences = &currentSlot.presentCompliteFence,
  };

  VkPresentInfoKHR presentInfo{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = &presentInfoFence,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &waitSemaphore,
    .swapchainCount = 1,
    .pSwapchains = &swapchain_,
    .pImageIndices = &acquireImageIndex_,
  };

  VkResult result = vkQueuePresentKHR(queue, &presentInfo);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR) {}

  getNextImage_ = true;
  swapchainFrame_ = (swapchainFrame_ + 1) % swapchainTextures_.size();

  return Result{};
}

void mrvk::VulkanSwapchain::querySurfaceCapabilities(
  std::vector<VkFormat>& deviceDepthFormat, VkSurfaceCapabilitiesKHR& deviceSurfaceCapabilities,
  std::vector<VkSurfaceFormatKHR>& deviceSurfaceFormats,
  std::vector<VkPresentModeKHR>& devicePresentModes) {
  const VkPhysicalDevice& pd = ctx_->device_.getPhysicalDevice();

  // Depth format check
  const VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,
                                   VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT,
                                   VK_FORMAT_D16_UNORM};
  for (const VkFormat& depthFormat : depthFormats) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(pd, depthFormat, &formatProps);
    if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT)
      deviceDepthFormat.push_back(depthFormat);
  }

  if (surface_ == VK_NULL_HANDLE)
    return;

  // Surface capabililies
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface_, &deviceSurfaceCapabilities);

  // Surface formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface_, &formatCount, nullptr);
  if (formatCount) {
    deviceSurfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface_, &formatCount, deviceSurfaceFormats.data());
  }

  // Present mode
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface_, &presentModeCount, nullptr);
  if (presentModeCount) {
    devicePresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface_, &presentModeCount,
                                              devicePresentModes.data());
  }
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats,
                                                  mrvk::ColorSpace colorSpace) {
  auto isNativeSwapChainBGR = [](const std::vector<VkSurfaceFormatKHR>& formats) -> bool {
    for (const VkSurfaceFormatKHR& format : formats) {
      const VkFormat fmt = format.format;
      // The preferred format should be the one which is closer to the beginning of the formats
      // container. If BGR is encountered earlier, it should be picked as the format of choice. If
      // RGB happens to be earlier, take it.
      if (fmt == VK_FORMAT_R8G8B8A8_UNORM || fmt == VK_FORMAT_R8G8B8A8_SRGB
          || fmt == VK_FORMAT_A2R10G10B10_UNORM_PACK32) {
        return false;
      }
      if (fmt == VK_FORMAT_B8G8R8A8_UNORM || fmt == VK_FORMAT_B8G8R8A8_SRGB
          || fmt == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
        return true;
      }
    }
    return false;
  };

  auto colorSpaceToVkSurfaceFormat = [](mrvk::ColorSpace colorSpace,
                                        bool isBGR) -> VkSurfaceFormatKHR {
    switch (colorSpace) {
      case mrvk::ColorSpace_SRGB_NONLINEAR:
        return VkSurfaceFormatKHR{isBGR ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
      case mrvk::ColorSpace_SRGB_EXTENDED_LINEAR:
        return VkSurfaceFormatKHR{VK_FORMAT_R16G16B16A16_SFLOAT,
                                  VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT};
      case mrvk::ColorSpace_HDR10:
        return VkSurfaceFormatKHR{isBGR ? VK_FORMAT_A2B10G10R10_UNORM_PACK32
                                        : VK_FORMAT_A2R10G10B10_UNORM_PACK32,
                                  VK_COLOR_SPACE_HDR10_ST2084_EXT};
      default:
        // default to normal sRGB non linear.
        return VkSurfaceFormatKHR{isBGR ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
  };

  const VkSurfaceFormatKHR preferred =
    colorSpaceToVkSurfaceFormat(colorSpace, isNativeSwapChainBGR(formats));

  for (const VkSurfaceFormatKHR& fmt : formats) {
    if (fmt.format == preferred.format && fmt.colorSpace == preferred.colorSpace) {
      return fmt;
    }
  }

  for (const VkSurfaceFormatKHR& fmt : formats) {
    if (fmt.format == preferred.format) {
      return fmt;
    }
  }

  return formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) {
#if defined(LINUX) || defined(M_ARM64)
  if (std::find(modes.cbegin(), modes.cend(), VK_PRESENT_MODE_IMMEDIATE_KHR) != modes.cend()) {
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
  }
#endif
  if (std::find(modes.cbegin(), modes.cend(), VK_PRESENT_MODE_MAILBOX_KHR) != modes.cend()) {
    return VK_PRESENT_MODE_MAILBOX_KHR;
  }
  return VK_PRESENT_MODE_FIFO_KHR;
};

static VkImageUsageFlags chooseUsageFlags(VkPhysicalDevice pd, VkSurfaceKHR surface,
                                          VkFormat format) {
  VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                 | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                 | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &caps);
  const bool isStorageSupported =
    (caps.supportedUsageFlags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) > 0;
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(pd, format, &props);
  const bool isTilingOptimalSupported =
    (props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) > 0;
  if (isStorageSupported && isTilingOptimalSupported) {
    usageFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
  }
  return usageFlags;
};

static uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& caps) {
  const uint32_t desired = caps.minImageCount + 1;
  const bool exceeded = caps.maxImageCount > 0 && desired > caps.maxImageCount;
  return exceeded ? caps.maxImageCount : desired;
};
//
// ================VulkanImmediateCommands===========================
//

mrvk::VulkanImmediateCommands::~VulkanImmediateCommands() {
  waitAll();
  for (CommandBufferWrapper& buf : buffers_) {
    vkDestroyFence(device_, buf.fence, nullptr);
    vkDestroySemaphore(device_, buf.semaphone, nullptr);
  }
  vkDestroyCommandPool(device_, commandPool_, nullptr);
}

void mrvk::VulkanImmediateCommands::init(ImmediateCommandsInitDesc desc) {
  ctx_ = desc.context;
  device_ = ctx_->device_.getVkDevice();

  queueFamilyIndex_ = desc.queueFamilyIndex;
  debugName_ = desc.debugName;
  vkGetDeviceQueue(device_, queueFamilyIndex_, 0, &queue_);

  const VkCommandPoolCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                                               | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                      .queueFamilyIndex = queueFamilyIndex_};

  vkCreateCommandPool(device_, &ci, nullptr, &commandPool_);

  const VkCommandBufferAllocateInfo ai = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPool_,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  for (uint32_t i = 0; i != kMaxCommandBuffers; ++i) {
    CommandBufferWrapper& buf = buffers_[i];
    char fenceName[256] = {0};
    char semaphoreName[256] = {0};
    if (debugName_) {
      strcpy(semaphoreName, "VulkanImmediateCommands Semaphone");
      strcpy(fenceName, "VulkanImmediateCommands Fence");
    }
    buf.semaphone = createSemaphore(device_, semaphoreName);
    buf.fence = createFence(device_, fenceName);
    vkResetFences(device_, 1, &buf.fence);
    vkAllocateCommandBuffers(device_, &ai, &buf.cmdBufAllocated);
    buf.handle.bufferIndex = i;
  }
}

const mrvk::VulkanImmediateCommands::CommandBufferWrapper&
mrvk::VulkanImmediateCommands::acquire() {
  // If dont have available commands buffers
  while (!numAvailableCommandBuffers_)
    purge();

  // Try find free command buffer
  VulkanImmediateCommands::CommandBufferWrapper* current = nullptr;
  size_t bufferIndex = 0;
  for (CommandBufferWrapper& buf : buffers_) {
    if (buf.cmdBuf == VK_NULL_HANDLE) {
      current = &buf;
      break;
    }
    bufferIndex++;
  }

  numAvailableCommandBuffers_--;

  // Write in handle info
  current->handle.submitId = submitCounter_;
  current->handle.bufferIndex = bufferIndex;

  // Indicate that the buffer is not free and that a write operation is currently in progress
  current->cmdBuf = current->cmdBufAllocated;
  current->isEncoding = true;

  const VkCommandBufferBeginInfo bi = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  MRVK_VERIFY(vkBeginCommandBuffer(current->cmdBuf, &bi));
  // Next buffer who will be submitted
  nextSubmitHandle_ = current->handle;

  return *current;
}

void mrvk::VulkanImmediateCommands::purge() {
  const uint32_t numBuffers = MRVK_ARRAY_NUM_ELEMENTS(buffers_);
  for (uint32_t index = 0; index < numBuffers; ++index) {
    CommandBufferWrapper& buf = buffers_[index % numBuffers];
    // If command buffer free or buffer is encoding right now, skip. Find only buffers who is busy
    // right now(executed on gpu)
    if (buf.cmdBuf == VK_NULL_HANDLE || buf.isEncoding)
      continue;
    // Just check fence status with out blocking. If fance is busy(VK_TIMEOUT) just skip, check
    // next buffer and a fence within it
    const VkResult result = vkWaitForFences(device_, 1, &buf.fence, VK_TRUE, 0);
    if (result == VK_SUCCESS) {
      vkResetCommandBuffer(buf.cmdBuf, VkCommandBufferResetFlags{0});
      vkResetFences(device_, 1, &buf.fence);
      buf.cmdBuf = VK_NULL_HANDLE;
      numAvailableCommandBuffers_++;
    } else {
      if (result != VK_TIMEOUT)
        MRVK_VERIFY(result);
    }
  }
}

mrvk::VulkanImmediateCommands::SubmitHandle mrvk::VulkanImmediateCommands::submit(
  const CommandBufferWrapper& wrapper) {
  MRVK_VERIFY(vkEndCommandBuffer(wrapper.cmdBuf));

  VkSemaphoreSubmitInfo waitSemaphores[] = {{}, {}};
  uint32_t numWaitSemaphores = 0;

  if (waitSemaphore_.semaphore) {
    waitSemaphores[numWaitSemaphores] = waitSemaphore_;
    numWaitSemaphores++;
  }
  // if (lastSubmitSemaphore_.semaphore) {
  //   waitSemaphores[numWaitSemaphores] = lastSubmitSemaphore_;
  //   numWaitSemaphores++;
  // }

  VkSemaphoreSubmitInfo signalSemaphores[] = {VkSemaphoreSubmitInfo{
                                                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                                .semaphore = wrapper.semaphone,
                                                .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                              },
                                              {}

  };

  uint32_t numSignalSemaphores = 1;
  if (signalSemaphore_.semaphore) {
    signalSemaphores[numSignalSemaphores] = signalSemaphore_;
    numSignalSemaphores++;
  }

  const VkCommandBufferSubmitInfo bufferSI = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                                              .commandBuffer = wrapper.cmdBuf};

  const VkSubmitInfo2 si = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
    .waitSemaphoreInfoCount = numWaitSemaphores,
    .pWaitSemaphoreInfos = waitSemaphores,
    .commandBufferInfoCount = 1u,
    .pCommandBufferInfos = &bufferSI,
    .signalSemaphoreInfoCount = numSignalSemaphores,
    .pSignalSemaphoreInfos = signalSemaphores,
  };

  MRVK_VERIFY(vkQueueSubmit2(queue_, 1u, &si, wrapper.fence));
  lastSubmitSemaphore_.semaphore = wrapper.semaphone;
  lastSubmitHandle_ = wrapper.handle;

  waitSemaphore_.semaphore = VK_NULL_HANDLE;
  signalSemaphore_.semaphore = VK_NULL_HANDLE;
  const_cast<CommandBufferWrapper&>(wrapper).isEncoding = false;
  submitCounter_++;
  if (!submitCounter_)
    submitCounter_++;
  return lastSubmitHandle_;
}

void mrvk::VulkanImmediateCommands::waitSemaphore(VkSemaphore semaphore) {
  waitSemaphore_.semaphore = semaphore;
}

void mrvk::VulkanImmediateCommands::signalSemaphore(VkSemaphore semaphore, uint64_t signalValue) {
  signalSemaphore_.semaphore = semaphore;
  signalSemaphore_.value = signalValue;
}

VkSemaphore mrvk::VulkanImmediateCommands::acquireLastSubmitSemaphore() {
  return lastSubmitSemaphore_.semaphore;
}

bool mrvk::VulkanImmediateCommands::isReady(SubmitHandle handle) const {
  if (handle.empty())
    return true;

  const CommandBufferWrapper& buf = buffers_[handle.bufferIndex];
  if (buf.cmdBuf == VK_NULL_HANDLE)
    return true;

  if (buf.handle.submitId != handle.submitId)
    return true;

  return vkWaitForFences(device_, 1, &buf.fence, VK_TRUE, 0) == VK_SUCCESS;
}

void mrvk::VulkanImmediateCommands::wait(SubmitHandle handle) {
  if (handle.empty()) {
    vkDeviceWaitIdle(device_);
    return;
  }
  if (isReady(handle))
    return;
  if (!MRVK_VERIFY_BOOL(!buffers_[handle.bufferIndex].isEncoding))
    return;
  MRVK_VERIFY(
    vkWaitForFences(device_, 1, &buffers_[handle.bufferIndex].fence, VK_TRUE, UINT64_MAX));
  purge();
}

void mrvk::VulkanImmediateCommands::waitAll() {
  VkFence fences[kMaxCommandBuffers];
  uint32_t numFences = 0;
  for (const CommandBufferWrapper& buf : buffers_) {
    if (buf.cmdBuf != VK_NULL_HANDLE && !buf.isEncoding)
      fences[numFences++] = buf.fence;
  }
  if (numFences)
    MRVK_VERIFY(vkWaitForFences(device_, numFences, fences, VK_TRUE, UINT64_MAX));
  purge();
}

//=================VulkanContext===============================
//

void mrvk::VulkanContext::init(mrvk::ContextInitDesc desc) {
  instance_.init(this, desc.instanceDesc);
  // MRVK_LOG_INFO("Instance creation was success");
  device_.init(this, desc.deviceDesc);
  // MRVK_LOG_INFO("Device creation was success");
  immediateCommands_.init({.context = this,
                           .queueFamilyIndex = device_.getDeviceQueues().graphicsQueueFamilyIndex,
                           .debugName = "ImmediateCommands"});

  // MRVK_LOG_INFO("Commands creation was success");
  descriptorManager_.init({.context = this});
  // MRVK_LOG_INFO("DM creation was success");

  SlangResult result = slang::createGlobalSession(&slangGlobalSession_);
  if (SLANG_FAILED(result))
    MRVK_LOG_ERROR("Failed to create the Slang global session");
}

mrvk::Holder<mrvk::ShaderModuleHandle> mrvk::VulkanContext::loadShaderModule(const char* fileName) {
  const std::string code = readShaderFile(fileName);
  const ShaderStage stage = shaderStageFromFileName(fileName);
  const std::string debugName = std::string("Shader Module: ") + fileName;
  Holder<ShaderModuleHandle> handle = createShaderModule({code.c_str(), stage, debugName.c_str()});
  return std::move(handle);
}

void mrvk::VulkanContext::shutdown() {}

void mrvk::VulkanContext::initSwapchain(SwapchainInitDesc desc) {
  desc.context = this;
  swapchain_.provideSurface(desc.surface);

  swapchain_.init(desc);
}

mrvk::Holder<mrvk::ShaderModuleHandle> mrvk::VulkanContext::createShaderModule(
  ShaderModuleDesc desc) {
  Result result;
  ShaderModuleState sm =
    desc.dataSize ? createShaderModuleFromSPIRV(desc.data, desc.dataSize, desc.debugName, &result)
                  : createShaderModuleFromGLSL(desc.stage, desc.data, desc.debugName, &result);

  return {this, shaderModulesPool_.create(std::move(sm))};
}

mrvk::ShaderModuleState mrvk::VulkanContext::createShaderModuleFromSPIRV(const void* spirv,
                                                                         size_t numBytes,
                                                                         const char* debugName,
                                                                         Result* outResult) {
  std::cout << "Shader " << debugName << ". Spirv butes num: " << numBytes << std::endl;

  VkShaderModule shaderModule = VK_NULL_HANDLE;
  const VkShaderModuleCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                       .codeSize = numBytes,
                                       .pCode = (const uint32_t*)spirv};

  const VkResult createResult =
    vkCreateShaderModule(device_.getVkDevice(), &ci, nullptr, &shaderModule);
  if (createResult != VK_SUCCESS)
    outResult->code = Result::Code_Error;

  SpvReflectShaderModule mdl;
  SpvReflectResult result = spvReflectCreateShaderModule(numBytes, spirv, &mdl);

  MRVK_VERIFY_BOOL(result == SPV_REFLECT_RESULT_SUCCESS);

  SCOPE_EXIT {
    spvReflectDestroyShaderModule(&mdl);
  };

  uint32_t pushConstantsSize = 0;
  for (uint32_t i = 0; i < mdl.push_constant_block_count; ++i) {
    const SpvReflectBlockVariable& block = mdl.push_constant_blocks[i];
    pushConstantsSize = std::max(pushConstantsSize, block.offset + block.size);
  }

  return {.sm = shaderModule, .pushConstantsSize = pushConstantsSize};
}

mrvk::ShaderModuleState mrvk::VulkanContext::createShaderModuleFromGLSL(ShaderStage stage,
                                                                        const char* source,
                                                                        const char* debugName,
                                                                        Result* outResult) {
  const VkShaderStageFlagBits vkStage = shaderStageToVk(stage);

  std::string sourcePatched;
  if (strstr(source, "#version ") == nullptr) {
    if (vkStage == VK_SHADER_STAGE_TASK_BIT_EXT || vkStage == VK_SHADER_STAGE_MESH_BIT_EXT) {
      sourcePatched += "#version 460\n"
                       "#extension GL_EXT_buffer_reference : require\n";
    }
    sourcePatched += source;
    source = sourcePatched.c_str();
  }

  const glslang_resource_t glslangResource =
    getGlslangResource(device_.getDeviceProperties().properties.properties.limits);

  std::vector<uint8_t> spirv;
  const Result result = compileShader(vkStage, source, &spirv, &glslangResource);
  if (result.code != Result::Code_Ok) {
    MRVK_LOG_ERROR("Compile shader is failed: %s", debugName);
  }
  return createShaderModuleFromSPIRV(spirv.data(), spirv.size(), debugName, outResult);
}

mrvk::Holder<mrvk::RenderPipelineHandle> mrvk::VulkanContext::createRenderPipeline(
  const RenderPipelineDesc& desc, Result* outResult) {
  const bool hasColorAttachment = desc.getNumColorAttachments();
  const bool hasDepthAttachment = desc.hasDepthAttachment();
  const bool hasAnyAttachment = hasColorAttachment || hasDepthAttachment;

  if (!hasAnyAttachment)
    return {};
  if (desc.stageFrag.smHandle.empty() && desc.stageVert.smHandle.empty())
    return {};

  RenderPipelineState renderPipelineState = {.desc = desc};

  for (size_t i = 0; i < DescriptorManager::DESCRIPTOR_SETS_COUNT; i++) {
    DescriptorManager::DescriptorSet dSet =
      descriptorManager_.getDSet((DescriptorManager::DescriptorSetIndex)i);
    renderPipelineState.lastDescriptorSetLayouts.push_back(dSet.layout);
  }

  return {this, renderPipelinePool_.create(std::move(renderPipelineState))};
}

mrvk::CommandBuffer& mrvk::VulkanContext::acquireCommandBuffer() {
  currentCommandBuffer_ =
    CommandBuffer{this, immediateCommands_, device_.getDeviceQueues().graphicsQueueFamilyIndex};

  return currentCommandBuffer_;
}

void mrvk::VulkanContext::submit(CommandBuffer& commandBuffer) {
  const VulkanImmediateCommands::CommandBufferWrapper& wrapper = *commandBuffer.getWrapper();

  VulkanImmediateCommands::SubmitHandle subHandle = immediateCommands_.submit(wrapper);
}

void mrvk::VulkanContext::present() {
  VkSemaphore renderFinished = immediateCommands_.acquireLastSubmitSemaphore();

  Result result = swapchain_.present(renderFinished);
}

VkPipeline mrvk::VulkanContext::getVkPipeline(RenderPipelineHandle handle) {
  RenderPipelineState* renderPipelineState = renderPipelinePool_.get(handle);
  if (!renderPipelineState)
    return VK_NULL_HANDLE;
  RenderPipelineDesc& ldesc = renderPipelineState->desc;

  bool pipelineNotAlign = false;
  for (uint32_t i = 0; i < DescriptorManager::DESCRIPTOR_SETS_COUNT; i++) {
    VkDescriptorSetLayout dSetLayout =
      descriptorManager_.getDSet((DescriptorManager::DescriptorSetIndex)i).layout;
    if (dSetLayout != renderPipelineState->lastDescriptorSetLayouts[i]) {
      pipelineNotAlign = true;
      break;
    }
  }

  if (!pipelineNotAlign && renderPipelineState->pipeline != VK_NULL_HANDLE)
    return renderPipelineState->pipeline;

  // Clenap
  VkDevice dev = device_.getVkDevice();
  VkPipeline p = renderPipelineState->pipeline;
  VkPipelineLayout pl = renderPipelineState->pipelineLayout;
  taskManager_.addTask(std::function<void()>([dev, p]() { vkDestroyPipeline(dev, p, nullptr); }));
  taskManager_.addTask(
    std::function<void()>([dev, pl]() { vkDestroyPipelineLayout(dev, pl, nullptr); }));

  renderPipelineState->pipeline = VK_NULL_HANDLE;
  renderPipelineState->pipelineLayout = VK_NULL_HANDLE;

  for (uint32_t i = 0; i < DescriptorManager::DESCRIPTOR_SETS_COUNT; i++) {
    renderPipelineState->lastDescriptorSetLayouts[i] =
      descriptorManager_.getDSet((DescriptorManager::DescriptorSetIndex)i).layout;
  }

  // Create new Pipeline Layout
  auto addPushConstantRange = [this](RenderPipelineDesc::Stage& stage) -> VkPushConstantRange {
    if (stage.smHandle.empty())
      return {.stageFlags = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};

    ShaderModuleState* smState = shaderModulesPool_.get(stage.smHandle);
    if (smState == nullptr || smState->pushConstantsSize == 0)
      return {.stageFlags = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};

    return VkPushConstantRange{.stageFlags = stage.shaderStage,
                               .offset = 0,
                               .size = smState->pushConstantsSize};
  };
  std::vector<VkPushConstantRange> pushConstantsRenge;
  VkPushConstantRange vertConstants = addPushConstantRange(ldesc.stageVert);
  if (vertConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(vertConstants);
  VkPushConstantRange tescConstants = addPushConstantRange(ldesc.stageTesc);
  if (tescConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(tescConstants);
  VkPushConstantRange teseConstants = addPushConstantRange(ldesc.stageTese);
  if (teseConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(teseConstants);
  VkPushConstantRange geomConstants = addPushConstantRange(ldesc.stageGeom);
  if (geomConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(geomConstants);
  VkPushConstantRange taskConstants = addPushConstantRange(ldesc.stageTask);
  if (taskConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(taskConstants);
  VkPushConstantRange meshConstants = addPushConstantRange(ldesc.stageMesh);
  if (meshConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(meshConstants);
  VkPushConstantRange fragConstants = addPushConstantRange(ldesc.stageFrag);
  if (fragConstants.stageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM)
    pushConstantsRenge.push_back(fragConstants);

  const VkPipelineLayoutCreateInfo plci{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .setLayoutCount = (uint32_t)renderPipelineState->lastDescriptorSetLayouts.size(),
    .pSetLayouts = renderPipelineState->lastDescriptorSetLayouts.data(),
    .pushConstantRangeCount = (uint32_t)pushConstantsRenge.size(),
    .pPushConstantRanges = pushConstantsRenge.data()};

  MRVK_VERIFY(vkCreatePipelineLayout(dev, &plci, nullptr, &renderPipelineState->pipelineLayout));

  // Rendering attachment info
  VkFormat colorAttachments[RenderPipelineDesc::MAX_COLOR_ATTACHMENT];
  for (size_t i = 0; i < ldesc.getNumColorAttachments(); i++) {
    colorAttachments[i] = ldesc.colorAttachment[i].format;
  }

  const VkPipelineRenderingCreateInfo rci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .pNext = nullptr,
    .viewMask = 0,
    .colorAttachmentCount = ldesc.getNumColorAttachments(),
    .pColorAttachmentFormats = colorAttachments,
    .depthAttachmentFormat =
      ldesc.hasDepthAttachment() ? ldesc.depthAttachment.format : VK_FORMAT_UNDEFINED,
    .stencilAttachmentFormat =
      ldesc.hasStencilAttachment() ? ldesc.stencilAttachment.format : VK_FORMAT_UNDEFINED};

  // Shader stages
  // Local lambda function for creating a shader stage
  auto createShaderStageInfo =
    [this, &ldesc](RenderPipelineDesc::Stage& stage) -> VkPipelineShaderStageCreateInfo {
    ShaderModuleState* sm = shaderModulesPool_.get(stage.smHandle);

    stage.specializationInfo = {.mapEntryCount = stage.getNumSpecConstants(),
                                .pMapEntries = stage.specConstantMapEntries,
                                .dataSize = stage.getSpecConstDataSize(),
                                .pData = stage.specConstData};

    const VkPipelineShaderStageCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = stage.shaderStage,
      .module = sm->sm,
      .pName = stage.entryPoint,
      .pSpecializationInfo = &stage.specializationInfo};

    return ci;
  };

  std::vector<VkPipelineShaderStageCreateInfo> ssci;
  auto fillShaderStageCreateInfo = [&ssci, &ldesc, createShaderStageInfo,
                                    this](RenderPipelineDesc::Stage& stage) -> void {
    if (stage.smHandle.empty())
      return;
    VkPipelineShaderStageCreateInfo stageInfo = createShaderStageInfo(stage);
    ssci.push_back(stageInfo);
  };

  fillShaderStageCreateInfo(ldesc.stageVert);
  fillShaderStageCreateInfo(ldesc.stageTesc);
  fillShaderStageCreateInfo(ldesc.stageTese);
  fillShaderStageCreateInfo(ldesc.stageGeom);
  fillShaderStageCreateInfo(ldesc.stageTask);
  fillShaderStageCreateInfo(ldesc.stageMesh);
  fillShaderStageCreateInfo(ldesc.stageFrag);
  std::cout << "Count shader stage: " << ssci.size() << std::endl;

  // Vertex input
  VkPipelineVertexInputStateCreateInfo visci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount = ldesc.getNumBindings(),
    .pVertexBindingDescriptions = ldesc.vertexBindings.data(),
    .vertexAttributeDescriptionCount = ldesc.getNumAttributes(),
    .pVertexAttributeDescriptions = ldesc.vertexAttributes.data()};

  const VkPipelineInputAssemblyStateCreateInfo iasci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .topology = ldesc.topology,
    .primitiveRestartEnable = ldesc.primitiveRestart};

  const VkPipelineTessellationStateCreateInfo tsci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .patchControlPoints = ldesc.patchControlPoints};

  uint32_t viewportCount = ldesc.getNumViewports();
  const VkPipelineViewportStateCreateInfo vsci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .viewportCount = ldesc.getNumViewports(),
    .pViewports = ldesc.viewports,
    .scissorCount = ldesc.getNumScissor(),
    .pScissors = ldesc.scissors};

  std::cout << "Viewport count: " << viewportCount << " scissorCount: " << ldesc.getNumScissor()
            << std::endl;

  VkPipelineRasterizationStateCreateInfo rsci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = ldesc.polygonMode,
    .cullMode = ldesc.cullMode,
    .frontFace = ldesc.frontFace,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.f,
  };

  VkPipelineMultisampleStateCreateInfo msci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .rasterizationSamples = ldesc.samplesCount,
    .sampleShadingEnable = ldesc.samplesCount <= VK_SAMPLE_COUNT_4_BIT ? VK_FALSE : VK_TRUE,
    .minSampleShading = 0.5f,
    .pSampleMask = nullptr,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE};

  std::cout << "Sample count: " << ldesc.samplesCount << " sample shading anable? "
            << msci.sampleShadingEnable << std::endl;

  VkPipelineDepthStencilStateCreateInfo dssci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable = ldesc.depthTest,
    .depthWriteEnable = VK_FALSE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable = VK_FALSE,
  };

  // Color blend state
  VkPipelineColorBlendAttachmentState
    colorBlendAttachmentsState[RenderPipelineDesc::MAX_COLOR_ATTACHMENT];
  for (size_t i = 0; i < ldesc.getNumColorBlendAttachment(); i++) {
    colorBlendAttachmentsState[i] = convertToVkBlendAttachmentState(ldesc.colorBlendAttachments[i]);
  }

  VkPipelineColorBlendStateCreateInfo cbsci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_NO_OP,
    .attachmentCount = ldesc.getNumColorBlendAttachment(),
    .pAttachments = colorBlendAttachmentsState,
    .blendConstants = {1.f, 1.f, 1.f, 1.f}};

  size_t numStates = ldesc.getNumDynamicStates();
  VkDynamicState states[RenderPipelineDesc::MAX_DYNAMIC_STATE];
  auto convDsToVkDs = [&states, &numStates](RenderPipelineDesc::DynamicState* st) -> void {
    for (size_t i = 0; i < numStates; i++) {
      states[i] = convertToVkDynamicState(st[i]);
    }
  };
  convDsToVkDs(ldesc.dynamicStates.data());

  VkPipelineDynamicStateCreateInfo dsci = {.sType =
                                             VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                           .pNext = nullptr,
                                           .flags = 0,
                                           .dynamicStateCount = ldesc.getNumDynamicStates(),
                                           .pDynamicStates = states};

  std::cout << "Count dynamic state: " << numStates << std::endl;
  for (size_t i = 0; i < numStates; i++) {
    std::cout << "State " << i << ": " << states[i] << std::endl;
  }

  const VkGraphicsPipelineCreateInfo gpci = {.sType =
                                               VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                             .pNext = &rci,
                                             .flags = 0,
                                             .stageCount = (uint32_t)ssci.size(),
                                             .pStages = ssci.data(),
                                             .pVertexInputState = &visci,
                                             .pInputAssemblyState = &iasci,
                                             .pTessellationState = &tsci,
                                             .pViewportState = &vsci,
                                             .pRasterizationState = &rsci,
                                             .pMultisampleState = &msci,
                                             .pDepthStencilState = &dssci,
                                             .pColorBlendState = &cbsci,
                                             .pDynamicState = &dsci,
                                             .layout = renderPipelineState->pipelineLayout,
                                             .renderPass = VK_NULL_HANDLE,
                                             .subpass = 0,
                                             .basePipelineHandle = VK_NULL_HANDLE,
                                             .basePipelineIndex = 0};

  MRVK_VERIFY(vkCreateGraphicsPipelines(dev, 0, 1, &gpci, nullptr, &renderPipelineState->pipeline));

  return renderPipelineState->pipeline;
}

//
//========================CommandBuffer=================================================
//
mrvk::CommandBuffer::CommandBuffer(VulkanContext* ctx, VulkanImmediateCommands& immediate,
                                   uint32_t queueFamilyIndex)
    : ctx_(ctx),
      wrapper_(&immediate.acquire()),
      immediate_(&immediate),
      queueFamilyIndex_(queueFamilyIndex) {}

void mrvk::CommandBuffer::cmdBeginRendering(BeginRenderingDesc desc) {
  auto fromMRVKToVulkanAttachment =
    [this](BeginRenderingDesc::Attachment& attachment) -> VkRenderingAttachmentInfo {
    return {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = nullptr,
      .imageView = ctx_->texturePool_.get(attachment.texture)->vkImageView,
      .imageLayout = attachment.imageLayout,
      .resolveMode = attachment.resolveMode,
      .resolveImageView = attachment.resolveImageView,
      .resolveImageLayout = attachment.resolveImageLayout,
      .loadOp = attachment.loadOp,
      .storeOp = attachment.storeOp,
      .clearValue = attachment.clearValue,
    };
  };

  VkRenderingAttachmentInfo color[RenderPipelineDesc::MAX_COLOR_ATTACHMENT];
  for (uint32_t i = 0; i < desc.getNumColorAttachments(); ++i) {
    color[i] = fromMRVKToVulkanAttachment(desc.colorAttachments[i]);
  }

  VkRenderingAttachmentInfo depth = desc.depthAttachment.texture.empty()
                                      ? VkRenderingAttachmentInfo{}
                                      : fromMRVKToVulkanAttachment(desc.depthAttachment);
  VkRenderingAttachmentInfo stencil = desc.stencilAttachment.texture.empty()
                                        ? VkRenderingAttachmentInfo{}
                                        : fromMRVKToVulkanAttachment(desc.stencilAttachment);

  VkRenderingInfo ri = {.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                        .pNext = nullptr,
                        .flags = 0,
                        .renderArea = desc.renderArea,
                        .layerCount = 1,
                        .viewMask = 0,
                        .colorAttachmentCount = desc.getNumColorAttachments(),
                        .pColorAttachments = desc.getNumColorAttachments() == 0 ? nullptr : color,
                        .pDepthAttachment = desc.depthAttachment.texture.empty() ? nullptr : &depth,
                        .pStencilAttachment =
                          desc.stencilAttachment.texture.empty() ? nullptr : &stencil};

  // change color attachments layout
  // colorAttachmentChangeLayoutToWrite(desc.colorAttachments, desc.getNumColorAttachments());

  vkCmdBeginRendering(wrapper_->cmdBuf, &ri);
}

void mrvk::CommandBuffer::colorAttachmentChangeLayoutToWrite(
  BeginRenderingDesc::Attachment attachments[], uint32_t num) {
  VkImageMemoryBarrier2 barriers[RenderPipelineDesc::MAX_COLOR_ATTACHMENT];

  VkImageMemoryBarrier2 barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                   .pNext = nullptr,
                                   .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                                   .srcAccessMask = VK_ACCESS_2_NONE,
                                   .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                   .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   .subresourceRange = {
                                     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                   }};
  for (size_t i = 0; i < num; i++) {
    barriers[i] = barrier;
    barriers[i].image = ctx_->texturePool_.get(attachments[i].texture)->vkImage;
  }

  const VkDependencyInfo di = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                               .imageMemoryBarrierCount = num,
                               .pImageMemoryBarriers = barriers};

  vkCmdPipelineBarrier2(wrapper_->cmdBuf, &di);
}

void mrvk::CommandBuffer::colorAttachmentChangeLayoutToPresent(TextureHandle textureHandle) {
  VkImage texture = ctx_->texturePool_.get(textureHandle)->vkImage;

  VkImageMemoryBarrier2 barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                   .pNext = nullptr,
                                   .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
                                   .dstAccessMask = VK_ACCESS_2_NONE,
                                   .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                   .image = texture,
                                   .subresourceRange = {
                                     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                   }};

  const VkDependencyInfo di = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                               .imageMemoryBarrierCount = 1,
                               .pImageMemoryBarriers = &barrier};

  vkCmdPipelineBarrier2(wrapper_->cmdBuf, &di);
}

void mrvk::CommandBuffer::cmdBarrier(const Barrier& barrier) {
  VkImage texture = ctx_->texturePool_.get(barrier.textureHandle)->vkImage;

  const VkImageMemoryBarrier2 vkBarrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                           .srcStageMask = barrier.srcStageMask,
                                           .srcAccessMask = barrier.srcAccessMask,
                                           .dstStageMask = barrier.dstStageMask,
                                           .dstAccessMask = barrier.dstAccessMask,
                                           .oldLayout = barrier.oldLayout,
                                           .newLayout = barrier.newLayout,
                                           .srcQueueFamilyIndex = barrier.srcQueueFamilyIndex,
                                           .dstQueueFamilyIndex = barrier.dstQueueFamilyIndex,
                                           .image = texture,
                                           .subresourceRange = barrier.subresourceRange};

  const VkDependencyInfo di = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                               .imageMemoryBarrierCount = 1,
                               .pImageMemoryBarriers = &vkBarrier};

  vkCmdPipelineBarrier2(wrapper_->cmdBuf, &di);
}

void mrvk::CommandBuffer::cmdBindRenderPipeline(RenderPipelineHandle pipelineHande) {
  if (pipelineHande.empty())
    return;

  currentPipelineGraphics_ = pipelineHande;

  VkPipeline pipeline = ctx_->getVkPipeline(pipelineHande);

  if (pipeline == VK_NULL_HANDLE) {
    return;
  }

  vkCmdBindPipeline(wrapper_->cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void mrvk::CommandBuffer::cmdBindViewport(const ViewportDesc& desc) {
  const VkViewport vp = {.x = desc.x,
                         .y = desc.y,
                         .width = desc.width,
                         .height = desc.height,
                         .minDepth = desc.minDepth,
                         .maxDepth = desc.maxDepth};

  vkCmdSetViewport(wrapper_->cmdBuf, 0, 1, &vp);
}

void mrvk::CommandBuffer::cmdBindScissorRect(const VkRect2D& rect) {
  const VkRect2D sc = rect;

  vkCmdSetScissor(wrapper_->cmdBuf, 0, 1, &sc);
}

void mrvk::CommandBuffer::cmdPushConstants(PipelineType pipelineType, VkShaderStageFlags stageFlags,
                                           uint32_t offset, const void* data) {
  RenderPipelineState* state = nullptr;

  switch (pipelineType) {
    case GRAPHICS_PIPELINE_TYPE:
      state = ctx_->renderPipelinePool_.get(currentPipelineGraphics_);
      break;
    case COMPUTE_PIPELINE_TYPE:
      break;
  }

  uint32_t size = ctx_->shaderModulesPool_.get(state->desc.stageVert.smHandle)->pushConstantsSize;

  vkCmdPushConstants(wrapper_->cmdBuf, state->pipelineLayout, stageFlags, offset, size, data);
}

void mrvk::CommandBuffer::cmdDraw(uint32_t vertexCount) {
  if (vertexCount == 0)
    return;

  vkCmdDraw(wrapper_->cmdBuf, vertexCount, 1, 0, 0);
}

void mrvk::CommandBuffer::cmdEndRendering() {
  vkCmdEndRendering(wrapper_->cmdBuf);
}

//
//=====================DescriptorManager================================================
//

void mrvk::DescriptorManager::init(const DescriptorManagerInitDesc desc) {
  ctx_ = desc.context;

  VkDevice dev = ctx_->device_.getVkDevice();

  DescriptorSet textureAndSemplerSet{};
  {
    const VkDescriptorSetLayoutBinding lb[] = {
      {.binding = 0,
       .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
       .descriptorCount = DEFAULT_TEXTURE_DESCRIPTOR_COUNT,
       .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
       .pImmutableSamplers = nullptr},

      {.binding = 1,
       .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
       .descriptorCount = DEFAULT_SAMPLER_DESCRIPTOR_COUNT,
       .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
       .pImmutableSamplers = nullptr}};

    const VkDescriptorBindingFlags bf[] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                                           VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo dslbfci = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .pNext = nullptr,
      .bindingCount = MRVK_ARRAY_NUM_ELEMENTS(bf),
      .pBindingFlags = bf};

    const VkDescriptorSetLayoutCreateInfo lci = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &dslbfci,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
      .bindingCount = MRVK_ARRAY_NUM_ELEMENTS(lb),
      .pBindings = lb};

    MRVK_VERIFY(vkCreateDescriptorSetLayout(dev, &lci, nullptr, &textureAndSemplerSet.layout));

    const VkDescriptorPoolSize poolSize[] = {{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                              .descriptorCount = DEFAULT_TEXTURE_DESCRIPTOR_COUNT},
                                             {.type = VK_DESCRIPTOR_TYPE_SAMPLER,
                                              .descriptorCount = DEFAULT_SAMPLER_DESCRIPTOR_COUNT}};

    const VkDescriptorPoolCreateInfo dpci = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                             .pNext = nullptr,
                                             .flags =
                                               VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                                             .maxSets = 1,
                                             .poolSizeCount = MRVK_ARRAY_NUM_ELEMENTS(poolSize),
                                             .pPoolSizes = poolSize};

    MRVK_VERIFY(vkCreateDescriptorPool(dev, &dpci, nullptr, &textureAndSemplerSet.pool));

    VkDescriptorSetAllocateInfo ai = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                      .pNext = nullptr,
                                      .descriptorPool = textureAndSemplerSet.pool,
                                      .descriptorSetCount = 1,
                                      .pSetLayouts = &textureAndSemplerSet.layout};

    maxTextures_ = DEFAULT_TEXTURE_DESCRIPTOR_COUNT;
    maxSamplers_ = DEFAULT_SAMPLER_DESCRIPTOR_COUNT;
  }

  dsets_.push_back(std::move(textureAndSemplerSet));
}

void mrvk::DescriptorManager::resize(DescriptorSetIndex setIndex, uint32_t binding, uint32_t size) {
  DescriptorSet& dset = dsets_[setIndex];
}

mrvk::DescriptorManager::DescriptorSet mrvk::DescriptorManager::getDSet(
  DescriptorSetIndex index) const {
  return dsets_[index];
}

//
//===============DeferredTaskManager=====================
//

void mrvk::DeferredTaskManager::addTask(std::function<void()> task) {
  tasks_.push_back(std::move(task));
}

void mrvk::DeferredTaskManager::processDeferredTasks() {
  auto it = tasks_.begin();
  for (uint32_t i = 0; i < tasks_.size(); i++) {
    tasks_[i]();
  }

  tasks_.clear();
}
