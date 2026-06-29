#pragma once

#include "SlangCompiler.h"

namespace mr {

class RMServiceLocator {
  static mr::SlangCompiler* m_slangCompiler;

public:
  static void provideSlangCompiler(mr::SlangCompiler* slangCompiler);
  static mr::SlangCompiler* getSlangCompiler();
};

}  // namespace mr
