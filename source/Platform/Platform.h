#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <cstdint>
#include <string>

namespace mr {

enum class PlatformType { None = 0, GLFWPlatform = 1, SDL2Platform = 2, SDL3Platform = 3 };

class Platform {
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  std::string m_title;

public:
  virtual ~Platform() = default;
  virtual bool init(const std::string& title, uint32_t width, uint32_t height) = 0;
  virtual void shutdown() = 0;
  virtual bool pollEvents() = 0;
  virtual void* getNativeWindow() = 0;
  virtual bool windowShouldClose() = 0;
  virtual std::vector<const char*> getInstanceExtension() = 0;
  virtual void getFramebufferSize(int& width, int& height) = 0;

  uint32_t getWidth() { return m_width; }
  uint32_t getHeight() { return m_height; }
  const std::string& getTitle() { return m_title; }

  void setWidth(uint32_t width) { m_width = width; }
  void setHeight(uint32_t height) { m_height = height; }
  void setTitle(const std::string& title) { m_title = title; }
};

class GLFWPlatform : public Platform {
  GLFWwindow* m_window = nullptr;
  VkSurfaceKHR m_surface;

public:
  virtual ~GLFWPlatform() = default;
  virtual bool init(const std::string& title, uint32_t width, uint32_t height) override;
  virtual void shutdown() override;
  virtual bool pollEvents() override;
  virtual void* getNativeWindow() override;
  virtual bool windowShouldClose() override { return glfwWindowShouldClose(m_window); }
  virtual std::vector<const char*> getInstanceExtension() override;
  virtual void getFramebufferSize(int& width, int& height) override;
};

}  // namespace mr
