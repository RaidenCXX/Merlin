#pragma once

#include <slang/slang-com-ptr.h>
#include <slang/slang.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace mr {
class SlangCompiler {
  Slang::ComPtr<slang::IGlobalSession> m_globalSesion;

public:
  bool init();
  void shutdown();

  bool compile(const std::string& path, const std::string& entryPoint, std::vector<char>& outSpirv);
};

}  // namespace mr
