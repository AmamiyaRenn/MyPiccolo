#pragma once

#include <cstring>
#include <set>
#include <vector>

#include "GLFW/glfw3.h"

#include "vulkan/vulkan_core.h"

#include "function/render/interface/rhi.h"
#include "function/render/interface/rhi_struct.h"
#include "function/render/render_type.h"

namespace Piccolo
{
    class VulkanRHI final : public RHI
    {
    public:
        // initialize
        virtual void initialize(RHIInitInfo init_info) override final;

        // allocate and create
        void         extracted(VkExtent2D& chosen_extent);
        void         createSwapchain() override;
        void         createSwapchainImageViews() override;
        RHIShader*   createShaderModule(const std::vector<unsigned char>& shader_code) override;
        bool         createGraphicsPipelines(RHIPipelineCache*                    pipelineCache,
                                             uint32_t                             createInfoCount,
                                             const RHIGraphicsPipelineCreateInfo* pCreateInfos,
                                             RHIPipeline*&                        pPipelines) override;
        virtual bool createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo,
                                          RHIPipelineLayout*&                pPipelineLayout) override;
        virtual bool createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass) override;
        virtual bool createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo,
                                       RHIFramebuffer*&                pFramebuffer) override;

        // query
        RHISwapChainDesc getSwapchainInfo() override;

    private:
        static uint8_t const m_k_max_frames_in_flight {1};

        RHIQueue* m_graphics_queue {nullptr}; // 图形队列句柄

        RHIFormat   m_swapchain_image_format {RHI_FORMAT_UNDEFINED}; // 交换链图片格式
        RHIExtent2D m_swapchain_extent;                              // 交换链图片范围
        std::vector<RHIImageView*> m_swapchain_imageviews; // 图像的视图：描述如何访问图像以及访问图像的哪一部分
        RHIViewport m_viewport;
        RHIRect2D   m_scissor;

        RHICommandBuffer* m_command_buffers[m_k_max_frames_in_flight];

        QueueFamilyIndices m_queue_indices; // 队列家族索引

        GLFWwindow*      m_window {nullptr};
        VkInstance       m_instance {nullptr}; // Vulkan实体
        VkSurfaceKHR     m_surface {nullptr};  // 窗口界面
        VkPhysicalDevice m_physical_device {nullptr};
        VkDevice         m_device {nullptr};        // 逻辑设备
        VkQueue          m_present_queue {nullptr}; // 显示队列句柄

        RHICommandPool* m_rhi_command_pool; // 命令池

        VkSwapchainKHR       m_swapchain {nullptr}; // 交换链句柄
        std::vector<VkImage> m_swapchain_images;    // 交换链图像句柄

        // command pool and buffers
        VkCommandPool   m_command_pools[m_k_max_frames_in_flight];
        VkCommandBuffer m_vk_command_buffers[m_k_max_frames_in_flight];

        void createInstance();
        void initializeDebugMessenger();
        void createWindowSurface();
        void initializePhysicalDevice();
        void createLogicDevice();
        void createCommandPool();
        void createCommandBuffers();

        bool m_enable_validation_layers {true}; // 启用验证层
        bool m_enable_debug_utils_label {true};

        bool                     checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        VkDebugUtilsMessengerEXT m_debug_messenger {nullptr}; // debug句柄
        VkResult                 createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks*              pAllocator,
                                                              VkDebugUtilsMessengerEXT*                 pDebugMessenger);
        QueueFamilyIndices       findQueueFamilies(VkPhysicalDevice physical_device);
        bool                     checkDeviceExtensionSupport(VkPhysicalDevice physical_device);
        SwapChainSupportDetails  querySwapChainSupport(VkPhysicalDevice physical_device);
        bool                     isDeviceSuitable(VkPhysicalDevice physical_device);

        // The VK_LAYER_KHRONOS_validation layer can be used to to assist developers in isolating incorrect usage,
        // and in verifying that applications correctly use the API
        const std::vector<char const*> m_validation_layers {"VK_LAYER_KHRONOS_validation"};
        uint32_t                       m_vulkan_api_version {VK_API_VERSION_1_0};

        // 交换链插件：等待着被显示到屏幕上的图像的队列
        std::vector<char const*> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkSurfaceFormatKHR
        chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats);
        VkPresentModeKHR
                   chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);
    };
} // namespace Piccolo