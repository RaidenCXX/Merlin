#pragma once
#include "vulkan/vulkan.h"

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class VkDeviceCapabilities : uint64_t {
  None = 0,
  DiscreteGPU = 1 << 0,
  SupportExtension = 1 << 1,
  SupportQueue = 1 << 2,
  ApiVersion13 = 1 << 3,
  GeometryShader = 1 << 4,
  SupportPresent = 1 << 5
};

enum class VkQueueFamilysIndex { Graphics = 0, Transfer = 1, Compute = 2, Sparse = 3 };

class Renderer {
public:
  virtual void init() = 0;
  virtual void shutdown() = 0;
};

class VkRenderer : public Renderer {
  VkInstance m_instance;
  VkSurfaceKHR m_surface;
  VkPhysicalDevice m_physicalDevice;
  VkDeviceCapabilities m_deviceCaps;
  std::unordered_map<VkQueueFamilysIndex, uint32_t> m_queues;

  VkDevice m_logicalDevice;

  std::vector<const char*> m_instanceExtensions;
  std::vector<const char*> m_deviceExtension;
  std::vector<const char*> m_validationLayers;

public:
  virtual void init() override;
  virtual void shutdown() override;
  VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }

private:
  void createInstance();
  void createSurface();
  void initValidationLayers();
  void initExtensions();
  void pickPhysicalDevice();
  void createDevice();

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
};
