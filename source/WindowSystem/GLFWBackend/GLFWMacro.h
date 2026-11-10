#include "minilog/minilog.h"

#define GLFW_ASSERT()                                                                              \
  do {                                                                                             \
    LLOGL("GLFWPlatform ERROR |File name: %s |Function name:%s |line: %i \n", __FILE_NAME__,       \
          __FUNCTION__, __LINE__);                                                                 \
    assert(false);                                                                                 \
  } while (0)

#define GLFW_ERROR_ASSERT(msg)                                                                     \
  do {                                                                                             \
    LLOGL("GLFWPlatform ERROR |File name: %s |Function name:%s |line: %i |MSG: %s\n",              \
          __FILE_NAME__, __FUNCTION__, __LINE__, msg);                                             \
    assert(false);                                                                                 \
  } while (0)

#define GLFW_LOG(msg)                                                                              \
  do {                                                                                             \
    LLOGL("GLFWPlatform LOG |File name: %s |Function name:%s |line: %i |MSG: %s\n", __FILE_NAME__, \
          __FUNCTION__, __LINE__, msg);                                                            \
  } while (0)
