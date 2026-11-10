#include "../../ServiceLocator.h"
#include "../../WindowSystem/WindowSystem.h"
#include "../Renderer.h"
#include "MRVK/MRVKClasses.h"
#include "MRVK/VulkanStructs.h"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>
#include <strings.h>
#include <sys/types.h>

#include <iostream>

void mr::VkRenderer::init() {
  mr::IWindowSystem* ws = mr::ServiceLocator::getWindowSystem();

  ctx_.init({
    .windowWidth = ws->getWidth(),
    .windowHeight = ws->getHeight(),
    .instanceDesc =
      {
        .enableValidation = true,
        .validationLayers = {"VK_LAYER_KHRONOS_validation"},
        .extensions =
          {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
          },
        .layerFeatures =
          {
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
          },
        .layerSettings =
          {
            "gpuav_descriptor_checks",
            "gpuav_indirect_draws_buffers",
            "gpuav_post_process_descriptor_indexing",
            "gpuav_validate_ray_query",
            "gpuav_validate_ray_hit_object",
            "gpuav_mesh_shading",
          },
      },
    .deviceDesc =
      {
        .deviceType = mrvk::HWDeviceType_Discrete,
        .extensions =
          {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
          },
        .fetures =
          {
            mrvk::DeviceFeatures_10_GeometryShader,
            mrvk::DeviceFeatures_10_SampleRateShading,
            mrvk::DeviceFeatures_10_MultiDrawIndirect,
            mrvk::DeviceFeatures_10_FragmentStoresAndAtomics,
            mrvk::DeviceFeatures_10_VertexPipelineStoresAndAtomics,
            mrvk::DeviceFeatures_10_ShaderInt64,
            mrvk::DeviceFeatures_11_StorageBuffer16BitAccess,
            mrvk::DeviceFeatures_11_SamplerYcbcrConversion,
            mrvk::DeviceFeatures_11_ShaderDrawParameters,
            mrvk::DeviceFeatures_12_DrawIndirectCount,  // розібратися в наступних
            mrvk::DeviceFeatures_12_DescriptorIndexing,
            mrvk::DeviceFeatures_12_ShaderSampledImageArrayNonUniformIndexing,
            mrvk::DeviceFeatures_12_DescriptorBindingSampledImageUpdateAfterBind,
            mrvk::DeviceFeatures_12_DescriptorBindingStorageImageUpdateAfterBind,
            mrvk::DeviceFeatures_12_DescriptorBindingUpdateUnusedWhilePending,
            mrvk::DeviceFeatures_12_DescriptorBindingPartiallyBound,
            mrvk::DeviceFeatures_12_DescriptorBindingVariableDescriptorCount,
            mrvk::DeviceFeatures_12_RuntimeDescriptorArray,
            mrvk::DeviceFeatures_12_TimelineSemaphore,
            mrvk::DeviceFeatures_12_VulkanMemoryModel,
            mrvk::DeviceFeatures_12_VulkanMemoryModelDeviceScope,
            mrvk::DeviceFeatures_12_BufferDeviceAddress,
            mrvk::DeviceFeatures_12_ScalarBlockLayout,
            mrvk::DeviceFeatures_12_StorageBuffer8BitAccess,
            mrvk::DeviceFeatures_13_SubgroupSizeControl,
            mrvk::DeviceFeatures_13_Synchronization2,
            mrvk::DeviceFeatures_13_DynamicRendering,
            mrvk::DeviceFeatures_13_Maintenance4,
            mrvk::DeviceFeatures_KHR_SwapchainMaintenance1,
          },
        .graphicsQueue = true,
      },
  });
  std::cout << "init done\n";
  ctx_.initSwapchain({
    .swapchainSize{.width = ws->getWidth(), .height = ws->getHeight()},
    .surface = ws->createVkWindowSurface(ctx_.getVkInstance()),
  });

  shadersModules_.emplace("vertex", std::move(ctx_.loadShaderModule("shaders/main.vert")));
  shadersModules_.emplace("fragment", std::move(ctx_.loadShaderModule("shaders/main.frag")));

  mrvk::RenderPipelineDesc desc = {
    .colorAttachment = {{.format = ctx_.getSwapchainFormat()}},
    .stageVert = {.smHandle = shadersModules_.find("vertex")->second.getHandle(),
                  .shaderStage = VK_SHADER_STAGE_VERTEX_BIT},
    .stageFrag = {.smHandle = shadersModules_.find("fragment")->second.getHandle(),
                  .shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT},

    .viewports = {{.width = 1.f, .height = 1.f}},
    .scissors = {{.extent = {.width = 1, .height = 1}}},
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthTest = VK_FALSE,
    .colorBlendAttachments = {{
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    }},

    .dynamicStates = {mrvk::RenderPipelineDesc::VIEWPORT_STATE,
                      mrvk::RenderPipelineDesc::SCISSOR_STATE},

  };
  renderPipelines_.emplace("triengle", ctx_.createRenderPipeline(desc));
}

void mr::VkRenderer::drawFrame() {
  // std::cout << "--Start draw frame: " << renderFrame << std::endl;
  mr::IWindowSystem* ws = mr::ServiceLocator::getWindowSystem();
  uint32_t width = ws->getWidth();
  uint32_t height = ws->getHeight();

  mrvk::CommandBuffer& cmdBuff = ctx_.acquireCommandBuffer();

  cmdBuff.cmdBindViewport({.width = (float)width, .height = (float)height});
  cmdBuff.cmdBindScissorRect({.offset = {0, 0}, .extent = {width, height}});

  auto swapchainTexture = ctx_.getSwapchainCurrentTexture();

  cmdBuff.cmdBarrier({.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                      .srcAccessMask = VK_ACCESS_2_NONE,
                      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                      .textureHandle = swapchainTexture,
                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                           .baseMipLevel = 0,
                                           .levelCount = 1,
                                           .baseArrayLayer = 0,
                                           .layerCount = 1}});

  cmdBuff.cmdBeginRendering({
    .renderArea = {.extent = {.width = width, .height = height}},
    .colorAttachments = {{
      .texture = swapchainTexture,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .resolveMode = VK_RESOLVE_MODE_NONE,
      .resolveImageView = VK_NULL_HANDLE,
      .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.color = {.float32 = {0.f, 0.f, 0.f, 0.f}}},
    }},
  });
  cmdBuff.cmdBindRenderPipeline(renderPipelines_.find("triengle")->second.getHandle());

  float pushConstValue = 0.f;

  // cmdBuff.cmdPushConstants(mrvk::GRAPHICS_PIPELINE_TYPE,
  //                          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
  //                          &pushConstValue);
  cmdBuff.cmdDraw(3);
  cmdBuff.cmdEndRendering();

  cmdBuff.cmdBarrier({.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                      .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
                      .dstAccessMask = VK_ACCESS_2_NONE,
                      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                      .textureHandle = swapchainTexture,
                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                           .baseMipLevel = 0,
                                           .levelCount = 1,
                                           .baseArrayLayer = 0,
                                           .layerCount = 1}});

  ctx_.submit(cmdBuff);

  ctx_.present();

  // std::cout << "--End draw frame: " << renderFrame++ << std::endl;
}

void mr::VkRenderer::shutdown() {
  ctx_.shutdown();
};
