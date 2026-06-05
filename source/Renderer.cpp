#include "Renderer.h"

#include "vulkan/vulkan_core.h"

#include <iostream>

bool VkRenderer::init() {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Merlin";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.pEngineName = "Merlin";
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;

  if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    std::cout << "Failed to create vulkan instance!" << std::endl;
}
