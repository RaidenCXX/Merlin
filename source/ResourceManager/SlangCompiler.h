#pragma once

#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace mr {
class SlangCompiler {
  Slang::ComPtr<slang::IGlobalSession> m_globalSesion;

public:
  void init();
  void shutdown();

  void compileVk(const std::string& id, const std::string& path, const std::string& entryPoint,
                 std::vector<char>& outSpirv);
  bool compileD3D();
};

}  // namespace mr
