#pragma once
#include "minilog/minilog.h"
#include "vulkan/vk_enum_string_helper.h"

#include <cassert>

// Macros

// ============================================================
// Vulkan result-checking macros
// ============================================================

#define MRVK_ARRAY_NUM_ELEMENTS(array) sizeof(array) / sizeof(array[0])

// Checks VkResult. On error: logs a warning but does not stop execution.
#define MRVK_CHECK(func)                                                                           \
  do {                                                                                             \
    const VkResult vk_result = (func);                                                             \
    if (vk_result != VK_SUCCESS) {                                                                 \
      LLOGW("Vulkan call check |File name: %s |Function name:%s |Line: %i |MSG: %s",               \
            __FILE_NAME__, #func, __LINE__, string_VkResult(vk_result));                           \
    }                                                                                              \
  } while (0)

// Checks VkResult. On error: logs (ERROR) and aborts (assert).
#define MRVK_VERIFY(func)                                                                          \
  do {                                                                                             \
    const VkResult vk_result = (func);                                                             \
    if (vk_result != VK_SUCCESS) {                                                                 \
      LLOGW("\033[31m Vulkan call failed  |File name: %s |Function name:%s |Line: %i |MSG: %s "    \
            "\033[0m",                                                                             \
            __FILE_NAME__, #func, __LINE__, string_VkResult(vk_result));                           \
                                                                                                   \
      assert(false);                                                                               \
    }                                                                                              \
  } while (0)

#define MRVK_VERIFY_BOOL(func)                                                                     \
  ([&]() -> bool {                                                                                 \
    const bool result = static_cast<bool>(func);                                                   \
    if (!result) {                                                                                 \
      LLOGW("Call failed |File name: %s |Function name: %s |Line: %i ", __FILE_NAME__, #func,      \
            __LINE__);                                                                             \
      assert(false);                                                                               \
    }                                                                                              \
    return result;                                                                                 \
  }())
// Same as VK_CHECK / VK_VERIFY, but additionally exits the function via return.
// severity: WARN or ERROR — determines whether the assert triggers.
#define MRVK_CHECK_RETURN(func)                                                                    \
  do {                                                                                             \
    const VkResult vk_result = (func);                                                             \
    if (vk_result != VK_SUCCESS) {                                                                 \
      LLOGW("Vulkan call check  |File name: %s |Function name:%s |Line: %i |MSG: %s",              \
            __FILE_NAME__, #func, __LINE__, string_VkResult(vk_result));                           \
      return getResultFromVkResult(vk_result);                                                     \
    }                                                                                              \
  } while (0)

#define MRVK_VERIFY_RETURN(func)                                                                   \
  do {                                                                                             \
    const VkResult vk_result = (func);                                                             \
    if (vk_result != VK_SUCCESS) {                                                                 \
      LLOGW("Vulkan call verify  |File name: %s |Function name:%s |Line: %i |MSG: %s",             \
            __FILE_NAME__, #func, __LINE__, string_VkResult(vk_result));                           \
      assert(false);                                                                               \
      return getResultFromVkResult(vk_result);                                                     \
    }                                                                                              \
  } while (0)

// ============================================================
// Flat messages (without VkResult) — always fatal
// ============================================================

// Arbitrary number of printf-style arguments: VK_FAIL("msg"), VK_FAIL("msg %s", detail)

#define MRVK_FAIL(...)                                                                             \
  do {                                                                                             \
    char vk_fail_buf[512];                                                                         \
    snprintf(vk_fail_buf, sizeof(vk_fail_buf), __VA_ARGS__);                                       \
    LLOGW("Vulkan call failed |File name: %s |Function name: %s |Line: %i |MSG: %s",               \
          __FILE_NAME__, __FUNCTION__, __LINE__, vk_fail_buf);                                     \
    assert(false);                                                                                 \
  } while (0)

// ============================================================
// Plain logging wrappers (no VkResult, no __FILE__/__LINE__ noise)
// ============================================================

#define MRVK_LOG_ERROR(...)                                                                        \
  do {                                                                                             \
    char vk_log_buf[512];                                                                          \
    snprintf(vk_log_buf, sizeof(vk_log_buf), __VA_ARGS__);                                         \
    LLOGW("\033[31m Vulkan error |File name: %s |Function name: %s |Line: %i |MSG: %s \033[0m",    \
          __FILE_NAME__, __FUNCTION__, __LINE__, vk_log_buf);                                      \
    assert(false);                                                                                 \
  } while (0)

#define MRVK_LOG_WARNING(...)                                                                      \
  do {                                                                                             \
    char vk_log_buf[512];                                                                          \
    snprintf(vk_log_buf, sizeof(vk_log_buf), __VA_ARGS__);                                         \
    LLOGW("\033[33m Vulkan warning  |File name: %s |Function name:%s |Line: %i |MSG: %s \033[0m",  \
          __FILE_NAME__, __FUNCTION__, __LINE__, vk_log_buf);                                      \
  } while (0)

#define MRVK_LOG_INFO(...)                                                                         \
  do {                                                                                             \
    char vk_log_buf[4096];                                                                         \
    snprintf(vk_log_buf, sizeof(vk_log_buf), __VA_ARGS__);                                         \
    LLOGL("\033[32m Vulkan info |File name: %s |Function name:%s |Line: %i |MSG: %s \033[0m",      \
          __FILE_NAME__, __FUNCTION__, __LINE__, vk_log_buf);                                      \
  } while (0)

#define MRVK_LAYER_SETTINGS_BOOL32(name, var)                                                      \
  VkLayerSettingEXT {                                                                              \
    .pLayerName = "VK_LAYER_KHRONOS_validation", .pSettingName = name,                             \
    .type = VK_LAYER_SETTING_TYPE_BOOL32_EXT, .valueCount = 1, .pValues = var                      \
  }

#define MRVK_CHECK_VULKAN_FEATURE(reqFeatures, availFeatures, feature, version, buf)               \
  do {                                                                                             \
    if ((reqFeatures).feature == VK_TRUE && (availFeatures).feature == VK_FALSE) {                 \
      (buf).append("   Vulkan feature |Version: ");                                                \
      (buf).append(version);                                                                       \
      (buf).append(" | Feature: ");                                                                \
      (buf).append(#feature);                                                                      \
      (buf).append("\n");                                                                          \
    }                                                                                              \
  } while (0)

#define MRVK_CHECK_FEATURE_1_0(reqFeatures, availFeatures, feature, buf)                           \
  MRVK_CHECK_VULKAN_FEATURE(reqFeatures.features10.features, availFeatures.features10.features,    \
                            feature, "1.0", buf)

#define MRVK_CHECK_FEATURE_1_1(reqFeatures, availFeatures, feature, buf)                           \
  MRVK_CHECK_VULKAN_FEATURE((reqFeatures).features11, (availFeatures).features11, feature, "1.1",  \
                            buf)

#define MRVK_CHECK_FEATURE_1_2(reqFeatures, availFeatures, feature, buf)                           \
  MRVK_CHECK_VULKAN_FEATURE((reqFeatures).features12, (availFeatures).features12, feature, "1.2",  \
                            buf)

#define MRVK_CHECK_FEATURE_1_3(reqFeatures, availFeatures, feature, buf)                           \
  MRVK_CHECK_VULKAN_FEATURE((reqFeatures).features13, (availFeatures).features13, feature, "1.3",  \
                            buf)

#define MRVK_CHECK_MAINTENANCE1(reqFeatures, availFeatures, feature, buf)                          \
  MRVK_CHECK_VULKAN_FEATURE((reqFeatures).swapchainMaintence1,                                     \
                            (availFeatures).swapchainMaintence1, feature, "1.3", buf)
