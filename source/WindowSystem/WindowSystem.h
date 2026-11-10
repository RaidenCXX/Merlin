#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <cstdint>
#include <string>

namespace mr {

enum WindowSystemType {
  WindowSystem_None = 0,
  WindowSystem_GLFW = 1,
  WindowSystem_SDL2 = 2,
  WindowSystem_SDL3 = 3
};

class IWindowSystem {
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  uint32_t m_framebufferWidth = 0;
  uint32_t m_framebufferHeight = 0;
  bool m_frameufferResized = false;
  std::string m_title;

  void getFramebufferSize(int& width, int& height) {
    width = m_framebufferWidth;
    height = m_framebufferHeight;
  };

public:
  virtual ~IWindowSystem() = default;
  virtual void init(const std::string& title, uint32_t width, uint32_t height) = 0;
  virtual void shutdown() = 0;
  virtual void pollEvents() = 0;
  virtual bool windowShouldClose() = 0;

  virtual VkSurfaceKHR createVkWindowSurface(VkInstance instance) = 0;
  virtual std::vector<const char*> getVkInstanceExtensions() = 0;
  virtual void destroyVkWindowSurface(VkInstance instance, VkSurfaceKHR surface) = 0;
  virtual std::vector<const char*> getExtension() = 0;

  uint32_t getWidth() const { return m_width; }
  uint32_t getHeight() const { return m_height; }
  const std::string& getTitle() { return m_title; }
  void setWindowSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
  }
  void setFramebufferSize(uint32_t width, uint32_t height) {
    m_framebufferWidth = width;
    m_framebufferHeight = height;
    m_frameufferResized = true;
  }
  void setTitle(const std::string& title) { m_title = title; }
  bool frameufferResized() { return m_frameufferResized; }
  void clearFramebufferResized() { m_frameufferResized = false; }
};

class GLFWWindowSystem : public IWindowSystem {
  GLFWwindow* m_window = nullptr;

public:
  ~GLFWWindowSystem() override = default;
  void init(const std::string& title, uint32_t width, uint32_t height) override;
  void shutdown() override;
  void pollEvents() override;
  bool windowShouldClose() override { return glfwWindowShouldClose(m_window); }

  VkSurfaceKHR createVkWindowSurface(VkInstance instance) override;
  std::vector<const char*> getVkInstanceExtensions() override;
  void destroyVkWindowSurface(VkInstance instance, VkSurfaceKHR surface) override;
  virtual std::vector<const char*> getExtension() override;

private:
  void initWindow(const std::string& title, uint32_t width, uint32_t height);
};

}  // namespace mr
