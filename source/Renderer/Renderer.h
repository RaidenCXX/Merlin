#pragma once
#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace mr {

enum class GraphicsAPI { None = 0, Vulkan = 1, DirectX = 2 };

enum class VkDeviceCapabilities : uint64_t {
  None = 0,
  DiscreteGPU = 1 << 0,
  SupportExtension = 1 << 1,
  SupportQueue = 1 << 2,
  ApiVersion13 = 1 << 3,
  GeometryShader = 1 << 4,
  SupportPresent = 1 << 5,
  SupportSwapchain = 1 << 6
};

enum class VkQueueFamilysIndex { Graphics = 0, Transfer = 1, Compute = 2, Sparse = 3, Present = 4 };

class Renderer {
public:
  virtual void init() = 0;
  virtual void shutdown() = 0;
};

class VkRenderer : public Renderer {
  // Devices
  VkInstance m_instance;
  VkSurfaceKHR m_surface;
  VkPhysicalDevice m_physicalDevice;
  VkDeviceCapabilities m_deviceCaps;
  VkDevice m_logicalDevice;

  // Queues
  std::unordered_map<VkQueueFamilysIndex, uint32_t> m_queues;
  VkQueue m_graphicsQueue;
  VkQueue m_transferQueue;

  // Swapchain
  VkSwapchainKHR m_swapchain = nullptr;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;
  VkSurfaceFormatKHR m_swapchainSurfaceFormat;
  VkExtent2D m_swapchainExtent;

  // Graphics pipeline
  VkPipeline m_graphicsPipeline;
  VkRenderPass m_renderPass = VK_NULL_HANDLE;

  // Extensions, layers
  std::vector<const char*> m_instanceExtensions;
  std::vector<const char*> m_deviceExtension;
  std::vector<const char*> m_validationLayers;

public:
  virtual void init() override;
  virtual void shutdown() override;
  VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }

private:
  void initValidationLayers();
  void initExtensions();
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapchain();
  void createPipeline();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

  static std::optional<uint32_t> getFamilyQueue(VkPhysicalDevice physicalDevice,
                                                VkQueueFlags supportedQueue,
                                                VkQueueFlags forbiddenFlags = 0);

  static std::optional<uint32_t> findMemoryType(VkPhysicalDevice physicalDevice,
                                                uint32_t memoryTypes,
                                                VkMemoryPropertyFlags properties);
  static void checkValidationLayerSupport(const std::vector<const char*>& required);

  static void checkInstanceExtensions(const std::vector<const char*>& requiredExt);
  static std::optional<uint32_t> findPresentQueueSupport(VkPhysicalDevice physicalDevice,
                                                         VkSurfaceKHR surface);
};

}  // namespace mr
