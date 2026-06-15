
#include "Renderer.h"

#include "Platform.h"
#include "ServiceLocator.h"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>
#include <strings.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

constexpr VkDeviceCapabilities operator|(VkDeviceCapabilities a, VkDeviceCapabilities b) {
  return static_cast<VkDeviceCapabilities>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

constexpr VkDeviceCapabilities& operator|=(VkDeviceCapabilities& a, VkDeviceCapabilities b) {
  a = a | b;
  return a;
}

constexpr bool hasFlag(VkDeviceCapabilities caps, VkDeviceCapabilities flag) {
  return (static_cast<uint64_t>(caps) & static_cast<uint64_t>(flag)) == static_cast<uint64_t>(flag);
}

void VkRenderer::init() {
  try {
    initValidationLayers();
    initExtensions();
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createDevice();
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

void VkRenderer::createInstance() {
  // Check instance extensions
  checkInstanceExtensions(m_instanceExtensions);

  // Check validation layers
  checkValidationLayerSupport(m_validationLayers);

  // App info
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Merlin";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.pEngineName = "Merlin";
  appInfo.apiVersion = VK_API_VERSION_1_3;

  // debug Create Info
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugCreateInfo.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugCreateInfo.pfnUserCallback = debugCallback;
  debugCreateInfo.pUserData = nullptr;

  // Vk instance create info
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = &debugCreateInfo;
  createInfo.flags = 0;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
  createInfo.ppEnabledLayerNames = m_validationLayers.data();
  createInfo.enabledExtensionCount = m_instanceExtensions.size();
  createInfo.ppEnabledExtensionNames = m_instanceExtensions.data();

  // Creation vulkan instance
  if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create vulkan instance!"};
  }
}

void VkRenderer::initValidationLayers() {
  m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
}

void VkRenderer::initExtensions() {
  GLFWPlatform* platform = static_cast<GLFWPlatform*>(ServiceLocator::getPlatform());
  if (!platform)
    throw std::runtime_error{"[ERROR](VkRenderer::initExtension){Platform pointer is null}"};

  // instance
  m_instanceExtensions = platform->getInstanceExtension();
  m_instanceExtensions.push_back("VK_EXT_DEBUG_UTILS_EXTENSION_NAME");

  // Device
  m_deviceExtension.push_back("VK_KHR_SWAPCHAIN_EXTENSION_NAME");
}

void VkRenderer::createSurface() {
  GLFWwindow* window = static_cast<GLFWwindow*>(ServiceLocator::getPlatform()->getNativeWindow());

  VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
  if (result != VK_SUCCESS)
    throw std::runtime_error{"[ERROR](VkRenderer){Failed to create window surface!}"};
}

void VkRenderer::pickPhysicalDevice() {
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
  if (physicalDeviceCount == 0) {
    throw std::runtime_error(
      "[ERROR](VkRenderer::pickPhysicalDevice){No Vulkan-capable GPU found}");
  }

  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

  VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
  uint32_t bestScore = 0;
  VkDeviceCapabilities bestCaps = VkDeviceCapabilities::None;

  for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
    VkDeviceCapabilities pdp = VkDeviceCapabilities::None;
    VkPhysicalDevice physicalDevice = physicalDevices[i];

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // Api version
    uint32_t major = VK_API_VERSION_MAJOR(deviceProperties.apiVersion);
    uint32_t minor = VK_API_VERSION_MINOR(deviceProperties.apiVersion);
    if (major > 1 || (major == 1 && minor >= 3)) {
      pdp |= VkDeviceCapabilities::ApiVersion13;
    }

    // Discrete GPU
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      pdp |= VkDeviceCapabilities::DiscreteGPU;
    }

    // Extensions
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> available(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                         available.data());

    bool allExtensionsFound = true;
    for (const std::string& requiredExt : m_deviceExtension) {
      bool found = false;
      for (const auto& ext : available) {
        if (requiredExt == ext.extensionName) {
          found = true;
          break;
        }
      }
      if (!found) {
        allExtensionsFound = false;
        break;
      }
    }
    if (!allExtensionsFound)
      continue;
    pdp |= VkDeviceCapabilities::SupportExtension;

    // Queue
    std::optional<uint32_t> familyGraphics =
      getFamilyQueue(physicalDevice, VK_QUEUE_GRAPHICS_BIT,
                     VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);

    std::optional<uint32_t> familyTransfer =
      getFamilyQueue(m_physicalDevice, VK_QUEUE_TRANSFER_BIT);

    if (!(familyGraphics && familyTransfer))
      continue;
    pdp |= VkDeviceCapabilities::SupportQueue;

    // Present
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, *familyGraphics, m_surface, &supported);
    if (!supported)
      continue;
    pdp |= VkDeviceCapabilities::SupportPresent;

    // Geometry shader (optional)
    if (deviceFeatures.geometryShader) {
      pdp |= VkDeviceCapabilities::GeometryShader;
    }

    // Score
    uint32_t score = 0;
    if (hasFlag(pdp, VkDeviceCapabilities::DiscreteGPU))
      score += 1000;
    if (hasFlag(pdp, VkDeviceCapabilities::ApiVersion13))
      score += 100;
    if (hasFlag(pdp, VkDeviceCapabilities::GeometryShader))
      score += 10;
    score += deviceProperties.limits.maxImageDimension2D;

    // std::cout << "Found device: " << deviceProperties.deviceName << " (score: " << score <<
    // ")\n";

    if (score > bestScore) {
      bestScore = score;
      bestDevice = physicalDevice;
      bestCaps = pdp;
    }
  }

  if (bestDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("[ERROR](Vulkan){No suitable GPU found}");
  }

  m_physicalDevice = bestDevice;
  m_deviceCaps = bestCaps;

  m_queues.emplace(VkQueueFamilysIndex::Graphics,
                   getFamilyQueue(m_physicalDevice, VK_QUEUE_GRAPHICS_BIT,
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)
                     .value());

  m_queues.emplace(VkQueueFamilysIndex::Transfer,
                   getFamilyQueue(m_physicalDevice, VK_QUEUE_TRANSFER_BIT));

  VkPhysicalDeviceProperties chosenProps;
  vkGetPhysicalDeviceProperties(bestDevice, &chosenProps);
  std::cout << "Selected GPU: " << chosenProps.deviceName << '\n';
}

void VkRenderer::createDevice() {
  // pQueueCreateInfos
  std::array<VkDeviceQueueCreateInfo, 2> queueInfos{};

  VkDeviceQueueCreateInfo graphicsQueueInfo{};
  queueInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfos[0].pNext = nullptr;
  queueInfos[0].flags = 0;
  queueInfos[0].queueFamilyIndex = m_queues[VkQueueFamilysIndex::Graphics];
  queueInfos[0].queueCount = 1;
  queueInfos[0].pQueuePriorities = nullptr;

  VkDeviceQueueCreateInfo transferQueueInfo{};
  queueInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfos[1].pNext = nullptr;
  queueInfos[1].flags = 0;
  queueInfos[1].queueFamilyIndex = m_queues[VkQueueFamilysIndex::Transfer];
  queueInfos[1].queueCount = 1;
  queueInfos[1].pQueuePriorities = nullptr;

  VkPhysicalDeviceFeatures enabledFeatures{};
  enabledFeatures.samplerAnisotropy = VK_TRUE;
  enabledFeatures.fillModeNonSolid = VK_TRUE;
  enabledFeatures.textureCompressionBC = VK_TRUE;
  enabledFeatures.depthClamp = VK_TRUE;

  VkDeviceCreateInfo deviceInfo{};
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.pNext = nullptr;
  deviceInfo.flags = 0;
  deviceInfo.queueCreateInfoCount = queueInfos.size();
  deviceInfo.pQueueCreateInfos = queueInfos.data();
  deviceInfo.enabledLayerCount = 0;
  deviceInfo.ppEnabledLayerNames = nullptr;
  deviceInfo.enabledExtensionCount = m_deviceExtension.size();
  deviceInfo.ppEnabledExtensionNames = m_deviceExtension.data();
  deviceInfo.pEnabledFeatures = &enabledFeatures;

  vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_logicalDevice);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VkRenderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                          VkDebugUtilsMessageTypeFlagsEXT messageType,
                          const VkDebugUtilsMessengerCallbackDataEXT* data, void* pUserData) {
  char saverity[16];
  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      strcpy(saverity, "[VERBOSE]");
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      strcpy(saverity, "[INFO]");
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      strcpy(saverity, "[WARNING]");
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      strcpy(saverity, "[ERROR]");
      break;
    default:
      strcpy(saverity, "[UNDEFINED]");
      break;
  }

  std::cerr << saverity << "(Vulkan)" << '{' << data->pMessage << '}' << std::endl;
  return VK_FALSE;
}

std::optional<uint32_t> VkRenderer::getFamilyQueue(VkPhysicalDevice physicalDevice,
                                                   VkQueueFlags requiredFlags,
                                                   VkQueueFlags forbiddenFlags) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  if (queueFamilyCount == 0)
    return std::nullopt;

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    const VkQueueFamilyProperties& family = queueFamilies[i];

    if (family.queueCount == 0)
      continue;

    if ((family.queueFlags & requiredFlags) != requiredFlags)
      continue;

    if ((family.queueFlags & forbiddenFlags) != 0)
      continue;

    return i;
  }

  return std::nullopt;
}

std::optional<uint32_t> findMemoryType(VkPhysicalDevice physicalDevice, uint32_t memoryTypes,
                                       VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

  for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
    // If memory type not available
    if (!((memoryTypes & (1u << i)) != 0))
      continue;

    // If properties of memory type not fit
    if (!((memProps.memoryTypes[i].propertyFlags & properties) == properties))
      continue;

    return i;
  }
  return std::nullopt;
}

void VkRenderer::checkValidationLayerSupport(const std::vector<const char*>& required) {
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> available(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, available.data());

  for (const char* req : required) {
    bool found = false;
    for (const auto& layer : available) {
      if (strcmp(req, layer.layerName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      std::string msg{
        "[ERROR](VkRenderer::checkValidationLayerSupport){Validation layer missing: "};
      msg += req;
      msg.push_back('}');
      std::runtime_error{msg};
    }
  }
}

void VkRenderer::checkInstanceExtensions(const std::vector<const char*>& requiredExt) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> available(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, available.data());

  for (const char* req : requiredExt) {
    bool found = false;
    for (const auto& ext : available) {
      if (strcmp(req, ext.extensionName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      std::string msg = "[ERROR](VkRenderer::checkInstanceExtensions){Missing instance extension: ";
      msg += req;
      msg += "}";
      throw std::runtime_error(msg);
    }
  }
}
