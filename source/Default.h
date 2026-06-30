#pragma once

#include "Platform/Platform.h"
#include "Renderer/Renderer.h"

#include <vector>

struct SystemPaths {
  std::vector<const char*> shadersFolderPath;
};

struct Settings {
  mr::GraphicsAPI graphicsApi = mr::GraphicsAPI::None;
  mr::PlatformType platform = mr::PlatformType::None;
  SystemPaths systemPaths;
};
