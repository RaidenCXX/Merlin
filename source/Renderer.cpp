
#include "Renderer.h"

#include "Platform.h"
#include "ServiceLocator.h"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>
#include <strings.h>

#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

constexpr DeviceCapabilities operator|(DeviceCapabilities a, DeviceCapabilities b) {
  return static_cast<DeviceCapabilities>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

constexpr DeviceCapabilities& operator|=(DeviceCapabilities& a, DeviceCapabilities b) {
  a = a | b;
  return a;
}

constexpr bool hasFlag(DeviceCapabilities caps, DeviceCapabilities flag) {
  return (static_cast<uint64_t>(caps) & static_cast<uint64_t>(flag)) == static_cast<uint64_t>(flag);
}

void VkRenderer::init() {
  try {
    initExtensions();
    createInstance();
    createSurface();
    pickPhysicalDevice();
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

void VkRenderer::createInstance() {
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
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;

  // Creation vulkan instance
  if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create vulkan instance!"};
  }
}

void VkRenderer::initExtensions() {
  m_requiredExtensions.push_back("VK_KHR_SWAPCHAIN_EXTENSION_NAME");
}

void VkRenderer::createSurface() {
  GLFWwindow* window =
    static_cast<GLFWwindow*>(ServiceLocator::getPlatform<GLFWPlatform>().getNativeWindow());

  VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
  if (result != VK_SUCCESS)
    throw std::runtime_error{"Failed to create vulkan instance!"};
}

void VkRenderer::pickPhysicalDevice() {
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
  if (physicalDeviceCount == 0) {
    throw std::runtime_error("No Vulkan-capable GPU found");
  }

  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

  VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
  uint32_t bestScore = 0;
  DeviceCapabilities bestCaps = DeviceCapabilities::None;
  uint32_t deviceQueue = 0;

  for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
    DeviceCapabilities pdp = DeviceCapabilities::None;
    VkPhysicalDevice physicalDevice = physicalDevices[i];

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // Api version
    uint32_t major = VK_API_VERSION_MAJOR(deviceProperties.apiVersion);
    uint32_t minor = VK_API_VERSION_MINOR(deviceProperties.apiVersion);
    if (major > 1 || (major == 1 && minor >= 3)) {
      pdp |= DeviceCapabilities::ApiVersion13;
    }

    // Discrete GPU
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      pdp |= DeviceCapabilities::DiscreteGPU;
    }

    // Extensions
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> available(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                         available.data());

    bool allExtensionsFound = true;
    for (const std::string& requiredExt : m_requiredExtensions) {
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
    pdp |= DeviceCapabilities::SupportExtension;

    // Queue
    std::optional<uint32_t> family =
      getFamilyQueue(physicalDevice,
                     VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
    if (!family)
      continue;
    pdp |= DeviceCapabilities::SupportQueue;

    // Present
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, *family, m_surface, &supported);
    if (!supported)
      continue;
    pdp |= DeviceCapabilities::SupportPresent;

    // Geometry shader (optional)
    if (deviceFeatures.geometryShader) {
      pdp |= DeviceCapabilities::GeometryShader;
    }

    // Score
    uint32_t score = 0;
    if (hasFlag(pdp, DeviceCapabilities::DiscreteGPU))
      score += 1000;
    if (hasFlag(pdp, DeviceCapabilities::ApiVersion13))
      score += 100;
    if (hasFlag(pdp, DeviceCapabilities::GeometryShader))
      score += 10;
    score += deviceProperties.limits.maxImageDimension2D;

    // std::cout << "Found device: " << deviceProperties.deviceName << " (score: " << score <<
    // ")\n";

    if (score > bestScore) {
      bestScore = score;
      bestDevice = physicalDevice;
      bestCaps = pdp;
      deviceQueue = family.value();
    }
  }

  if (bestDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("[ERROR](Vulkan){No suitable GPU found}");
  }

  m_physicalDevice = bestDevice;
  m_deviceCaps = bestCaps;
  m_queues.emplace("graphics", deviceQueue);

  VkPhysicalDeviceProperties chosenProps;
  vkGetPhysicalDeviceProperties(bestDevice, &chosenProps);
  std::cout << "Selected GPU: " << chosenProps.deviceName << '\n';
}

void VkRenderer::createDevice() {}

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
