#include "../WindowSystem.h"
#include "GLFWMacro.h"

#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdint>
#include <string>

void mr::GLFWWindowSystem::init(const std::string& title, uint32_t width, uint32_t height) {
  initWindow(title, width, height);
  setWindowSize(width, height);
  setTitle(title);
  GLFW_LOG("GLFW window successfully created");
}

void mr::GLFWWindowSystem::initWindow(const std::string& title, uint32_t width, uint32_t height) {
  if (!glfwInit()) {
    GLFW_ASSERT();
  }

  const bool wantsWholeArea = (width == 0 || height == 0);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, wantsWholeArea ? GLFW_TRUE : GLFW_FALSE);

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();

  int x = 0;
  int y = 0;

  if (wantsWholeArea) {
    int areaW = 0;
    int areaH = 0;
    glfwGetMonitorWorkarea(monitor, &x, &y, &areaW, &areaH);
  }

  m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(),
                              nullptr, nullptr);
  if (m_window == nullptr) {
    glfwTerminate();
    GLFW_ASSERT();
  }

  // CallBacks
  glfwSetWindowUserPointer(m_window, this);

  glfwSetErrorCallback(
    [](int error, const char* description) { LLOGE("GLFW Error (%i): %s\n", error, description); });

  glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
    GLFWWindowSystem* self = static_cast<GLFWWindowSystem*>(glfwGetWindowUserPointer(window));

    self->setFramebufferSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
  });

  glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int, int action, int) {
    // Key combination for close window
    if ((key == GLFW_KEY_F4 && action == GLFW_PRESS)) {
      if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  });
}

void mr::GLFWWindowSystem::shutdown() {
  glfwDestroyWindow(m_window);
  m_window = nullptr;
  glfwTerminate();
}

void mr::GLFWWindowSystem::pollEvents() {
  glfwPollEvents();
}

std::vector<const char*> mr::GLFWWindowSystem::getVkInstanceExtensions() {
  uint32_t glfwExtCount = 0;
  const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

  if (glfwExts == nullptr) {
    GLFW_ERROR_ASSERT(
      "glfwGetRequiredInstanceExtensions() returned nullptr — Vulkan not supported?");
  }

  return std::vector<const char*>(glfwExts, glfwExts + glfwExtCount);
}

std::vector<const char*> mr::GLFWWindowSystem::getExtension() {
  uint32_t count = 0;
  const char** glfwEx = glfwGetRequiredInstanceExtensions(&count);

  if (!glfwEx) {
    GLFW_ERROR_ASSERT("Failed to get GLFW required Vulkan extensions!");
  }
  std::vector<const char*> extensions(glfwEx, glfwEx + count);
  return extensions;
}

VkSurfaceKHR mr::GLFWWindowSystem::createVkWindowSurface(VkInstance instance) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  const VkResult result = glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
  if (result != VK_SUCCESS) {
    GLFW_ERROR_ASSERT("Failed to create window surface");
  }
  return surface;
}

void mr::GLFWWindowSystem::destroyVkWindowSurface(VkInstance instance, VkSurfaceKHR surface) {}
