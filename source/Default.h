#pragma once

#include "Platform/Platform.h"
#include "Renderer/Renderer.h"

struct Settings {
  mr::GraphicsAPI m_graphicsApi = mr::GraphicsAPI::None;
  mr::PlatformType m_platform = mr::PlatformType::None;
};
