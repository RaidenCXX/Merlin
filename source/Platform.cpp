#include "Platform.h"

#include <GLFW/glfw3.h>

bool GLFWPlatform::init(const std::string& title, uint32_t width, uint32_t height) {
  if (!glfwInit())
    return false;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

  if (m_window == nullptr)
    return false;

  setWidth(width);
  setHeight(height);
  setTitle(title);

  return true;
}

void GLFWPlatform::shutdown() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

bool GLFWPlatform::pollEvents() {
  return true;
}

void* GLFWPlatform::getNativeWindow() {
  return nullptr;
}

std::vector<const char*> GLFWPlatform::getInstanceExtension() {
  uint32_t glfwExtCount = 0;
  const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

  std::vector<const char*> ext;
  for (uint32_t i = 0; i < glfwExtCount; ++i) {
    ext.push_back(glfwExts[i]);
  }
  return ext;
}
