#pragma once

#include "Volk/volk.h"
#include "VulkanUtility.h"
#include "slang/slang.h"
//
#include "VulkanStructs.h"
#include "vulkan/vulkan_core.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

namespace mrvk {

enum { MAX_VERTEX_INPUT_BINDING = 16 };

class VulkanContext;

class Instance final {
  VulkanContext* ctx_ = nullptr;
  VkInstance instance_ = VK_NULL_HANDLE;
  InstanceInitDesc desc_;
  VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;

  void initInstance();
  void initDebugMessenger();

public:
  void init(VulkanContext* context = nullptr, const InstanceInitDesc& desc = {});
  VkInstance getVkInstance() { return instance_; }
  const InstanceInitDesc& getDesc() { return desc_; }
};

class Device final {
  enum { MAX_QUEUE_COUNT = 3 };

  VulkanContext* ctx_ = nullptr;
  VkDevice device_ = VK_NULL_HANDLE;
  DeviceInitDesc desc_;
  HWDeviceDesc deviceDesc_;
  std::vector<const char*> extensions_;
  DeviceFeaturesChain features_;
  DeviceProperties properties_;
  DeviceQueues queue_;

  uint32_t queryDevices(HWDeviceType deviceType, HWDeviceDesc* outDevices, uint32_t maxOutDevices);
  static void getPhysicalDeviceProps(VkPhysicalDevice physicalDevice,
                                     std::vector<VkExtensionProperties>& extensions,
                                     const char* validationLayer = nullptr);
  static uint32_t findQueueFamilyIndex(VkPhysicalDevice dev, VkQueueFlagBits flags);
  void initDevice();

public:
  void init(VulkanContext* context = nullptr, const DeviceInitDesc desc = {});
  const VkDevice getVkDevice() const { return device_; }
  const VkPhysicalDevice getPhysicalDevice() const;
  const DeviceProperties& getDeviceProperties() { return properties_; }
  const DeviceQueues& getDeviceQueues() const { return queue_; }
};

class VulkanImmediateCommands final {
public:
  struct SubmitHandle final {
    uint32_t bufferIndex = 0;
    uint32_t submitId = 0;
    SubmitHandle() = default;
    explicit SubmitHandle(uint64_t handle)
        : bufferIndex(uint32_t(handle & 0xffffffff)), submitId(uint32_t(handle >> 32)) {}
    bool empty() const { return submitId == 0; }
    uint64_t handle() const { return (uint64_t(submitId) << 32) + bufferIndex; }
  };

  struct CommandBufferWrapper final {
    VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
    VkCommandBuffer cmdBufAllocated = VK_NULL_HANDLE;
    SubmitHandle handle = {};
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore semaphone = VK_NULL_HANDLE;
    bool isEncoding = false;
  };

private:
  static constexpr uint32_t kMaxCommandBuffers = 64;

  VulkanContext* ctx_;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
  VkCommandPool commandPool_ = VK_NULL_HANDLE;

  uint32_t queueFamilyIndex_ = 0;
  const char* debugName_ = "";
  CommandBufferWrapper buffers_[kMaxCommandBuffers];
  uint32_t numAvailableCommandBuffers_ = kMaxCommandBuffers;
  uint32_t submitCounter_ = 1;
  SubmitHandle nextSubmitHandle_ = SubmitHandle();
  SubmitHandle lastSubmitHandle_ = SubmitHandle();
  VkSemaphoreSubmitInfo lastSubmitSemaphore_ = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                                .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT};
  VkSemaphoreSubmitInfo waitSemaphore_ = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                          .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
  VkSemaphoreSubmitInfo waitTimeline_ = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                         .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
  VkSemaphoreSubmitInfo signalSemaphore_ = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                            .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

  // VkSemaphore lastPresentSemaphore_ = VK_NULL_HANDLE;
  // VkFence lastPresentFence_ = VK_NULL_HANDLE;

  void purge();

public:
  VulkanImmediateCommands() = default;
  ~VulkanImmediateCommands();

  void init(ImmediateCommandsInitDesc desc);

  const CommandBufferWrapper& acquire();
  SubmitHandle submit(const CommandBufferWrapper& wrapper);
  void waitSemaphore(VkSemaphore semaphore);
  void signalSemaphore(VkSemaphore semaphore, uint64_t signalValue);
  VkSemaphore acquireLastSubmitSemaphore();
  SubmitHandle getLastSubmitHandle() const { return lastSubmitHandle_; };
  SubmitHandle getNextSubmitHandle() const { return nextSubmitHandle_; };
  bool isReady(SubmitHandle handle) const;
  void wait(SubmitHandle handle);
  void waitAll();
  // void setLastPresentSyncObj(VkSemaphore semaphore, VkFence fence) {
  //   lastPresentSemaphore_ = semaphore;
  //   lastPresentFence_ = fence;
  // }
};

template <typename HandleType>
class Holder {
  VulkanContext* context_ = nullptr;
  HandleType handle_ = {};

public:
  Holder() noexcept = default;
  Holder(VulkanContext* context, HandleType handle) : handle_(handle), context_(context) {}
  Holder(const Holder<HandleType>& holder) = delete;
  Holder(Holder<HandleType>&& holder) : context_(holder.context_), handle_(holder.handle_) {
    holder.context_ = nullptr;
    holder.handle_ = HandleType{};
  }

  Holder& operator=(const Holder&) = delete;
  Holder& operator=(Holder&& holder) {
    if (this == &holder) {
      return *this;
    }

    reset();

    context_ = holder.context_;
    handle_ = holder.handle_;
    holder.context_ = nullptr;
    holder.handle_ = HandleType{};
    return *this;
  }

  ~Holder() {
    destroy(context_, handle_);
    context_ = nullptr;
    handle_ = HandleType{};
  };

  void reset() {
    destroy(context_, handle_);

    context_ = nullptr;
    handle_ = HandleType{};
  };

  HandleType getHandle() { return handle_; }
};

template <typename Type, typename Data>
class Pool final {
  struct Slot {
    Data data;
    uint32_t generation = 1;
  };

  std::vector<Slot> slots_;
  std::vector<uint32_t> freeList_;

  Handle<Type> allocate() {
    uint32_t index;
    if (!freeList_.empty()) {
      index = freeList_.back();
      freeList_.pop_back();

      Slot& slot = slots_[index];
      slot.generation++;

      return Handle<Type>{index, slot.generation};
    }

    index = (uint32_t)slots_.size();
    slots_.push_back(Slot{});

    return Handle<Type>{index, 1};
  }

  void release(Handle<Type> handle) {
    Slot& slot = slots_[handle.index()];
    freeList_.push_back(handle.index());
    slot.data = Data{};
  }

public:
  Pool() = default;

  Pool(const Pool&) = delete;
  Pool& operator=(const Pool&) = delete;

  Pool(Pool&&) noexcept = delete;
  Pool& operator=(Pool&&) noexcept = delete;

  ~Pool() = default;

  Handle<Type> create(Data state) {
    Handle<Type> handle = allocate();
    slots_[handle.index()].data = std::move(state);

    return handle;
  }

  Data* get(Handle<Type> handle) {
    Data& res = slots_[handle.index()].data;
    return &res;
  }

  void destroy(Handle<Type> handle) { release(handle); }
};

class VulkanSwapchain final {
  enum { MRVK_MAX_SWAPCHAIN_IMAGES = 16 };

  VulkanContext* ctx_;

  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkSurfaceCapabilitiesKHR surfaceCapabilities_;
  VkSurfaceFormatKHR surfaceFormat_;
  VkPresentModeKHR presentMode_;
  std::vector<TextureHandle> swapchainTextures_;
  uint32_t acquireImageIndex_ = UINT32_MAX;
  bool getNextImage_ = true;
  uint32_t swapchainFrame_ = 0;
  std::vector<VkSemaphore> acquireSemaphores_;
  std::vector<VkFence> presentFence_;
  struct FrameSlot {
    uint32_t slotIndex = 0;
    VkSemaphore acquireCompliteSemaphore = VK_NULL_HANDLE;
    VkFence presentCompliteFence = VK_NULL_HANDLE;
  };
  std::vector<FrameSlot> frameSlots_;

  void querySurfaceCapabilities(std::vector<VkFormat>& deviceDepthFormat,
                                VkSurfaceCapabilitiesKHR& deviceSurfaceCapabilities,
                                std::vector<VkSurfaceFormatKHR>& deviceSurfaceFormats,
                                std::vector<VkPresentModeKHR>& devicePresentModes);

public:
  void init(mrvk::SwapchainInitDesc desc);
  TextureHandle getCurrentTexture();
  VkFormat getSwapchainFormat() { return surfaceFormat_.format; }
  Result present(VkSemaphore waitSemaphore);
  void provideSurface(VkSurfaceKHR surface) { surface_ = surface; }
};

class CommandBuffer final {
  VulkanContext* ctx_ = nullptr;
  VulkanImmediateCommands* immediate_ = nullptr;
  const VulkanImmediateCommands::CommandBufferWrapper* wrapper_ = nullptr;
  uint32_t queueFamilyIndex_ = 0;

  RenderPipelineHandle currentPipelineGraphics_ = {};

  void colorAttachmentChangeLayoutToWrite(BeginRenderingDesc::Attachment attachments[],
                                          uint32_t num);

  void colorAttachmentChangeLayoutToPresent(TextureHandle textureHandle);

public:
  CommandBuffer() = default;
  CommandBuffer(VulkanContext* ctx, VulkanImmediateCommands& immediate, uint32_t queueFamilyIndex);

  CommandBuffer& operator=(CommandBuffer&& other) = default;
  const VulkanImmediateCommands::CommandBufferWrapper* getWrapper() { return wrapper_; };

  void cmdBeginRendering(BeginRenderingDesc desc);
  void cmdBindRenderPipeline(RenderPipelineHandle renderPipelineHandle);
  void cmdBindViewport(const ViewportDesc& desc);
  void cmdBindScissorRect(const VkRect2D& rect);
  void cmdPushConstants(PipelineType pipelineType, VkShaderStageFlags stageFlags, uint32_t offset,
                        const void* data);
  void cmdBarrier(const Barrier& barrier);
  void cmdDraw(uint32_t vertexCount);
  void cmdEndRendering();
};

class DescriptorManager {
public:
  enum DescriptorSetIndex : size_t { TEXTURE_SAMPLER_SET_INDEX = 0, DESCRIPTOR_SETS_COUNT = 1 };

  enum { DEFAULT_TEXTURE_DESCRIPTOR_COUNT = 100, DEFAULT_SAMPLER_DESCRIPTOR_COUNT = 10 };
  uint32_t maxTextures_ = 0;
  uint32_t maxSamplers_ = 0;

  struct DescriptorSet {
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;
  };

private:
  VulkanContext* ctx_ = nullptr;
  std::vector<DescriptorSet> dsets_;

public:
  void init(const DescriptorManagerInitDesc desc);
  void resize(DescriptorSetIndex setIndex, uint32_t binding, uint32_t size);
  DescriptorManager::DescriptorSet getDSet(DescriptorSetIndex index) const;
  const std::vector<DescriptorSet>& getDSets() { return dsets_; };
};

class DeferredTaskManager {
  std::vector<std::function<void()>> tasks_;

public:
  void addTask(std::function<void()> task);
  void processDeferredTasks();
};

class VulkanContext final {
  friend class Instance;
  friend class Device;
  friend class DescriptorManager;
  friend class CommandBuffer;
  friend class VulkanSwapchain;
  friend class VulkanImmediateCommands;

  Instance instance_;
  Device device_;
  VulkanSwapchain swapchain_;
  VulkanImmediateCommands immediateCommands_;
  DescriptorManager descriptorManager_;
  DeferredTaskManager taskManager_;

  slang::IGlobalSession* slangGlobalSession_ = nullptr;

  CommandBuffer currentCommandBuffer_;

  bool awaitingCreation_ = false;

  uint32_t numRenderPipelinesCreated = 0;

  Pool<ShaderModule, ShaderModuleState> shaderModulesPool_;
  Pool<Texture, VulkanImage> texturePool_;
  Pool<RenderPipeline, RenderPipelineState> renderPipelinePool_;

public:
  VulkanContext() = default;
  ~VulkanContext() = default;

  void init(mrvk::ContextInitDesc desc);
  void initSwapchain(SwapchainInitDesc desc);

  void shutdown();
  Holder<ShaderModuleHandle> loadShaderModule(const char* fileName);
  Holder<ShaderModuleHandle> createShaderModule(ShaderModuleDesc desc);
  ShaderModuleState createShaderModuleFromSPIRV(const void* spirv, size_t numBytes,
                                                const char* debugName, Result* outResult);
  ShaderModuleState createShaderModuleFromGLSL(ShaderStage stage, const char* source,
                                               const char* debugName, Result* outResult);

  Holder<RenderPipelineHandle> createRenderPipeline(const RenderPipelineDesc& desc,
                                                    Result* outResult = nullptr);
  VkPipeline getVkPipeline(RenderPipelineHandle handle);
  CommandBuffer& acquireCommandBuffer();
  void submit(CommandBuffer& commandBuffer);
  void present();

  void destroy(ShaderModuleHandle handle) { shaderModulesPool_.destroy(handle); }
  void destroy(RenderPipelineHandle handle) { renderPipelinePool_.destroy(handle); }

  VkFormat getSwapchainFormat() { return swapchain_.getSwapchainFormat(); }
  TextureHandle getSwapchainCurrentTexture() { return swapchain_.getCurrentTexture(); }
  const VkInstance getVkInstance() { return instance_.getVkInstance(); }
};

}  // namespace mrvk
