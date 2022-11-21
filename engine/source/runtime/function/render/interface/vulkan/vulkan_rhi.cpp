#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan_core.h"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <stdint.h>
#include <utility>

#include "core/base/macro.h"
#include "function/render/interface/vulkan/vulkan_rhi.h"
#include "function/render/interface/vulkan/vulkan_rhi_resource.h"
#include "function/render/interface/vulkan/vulkan_util.h"
#include "function/render/render_type.h"

namespace Piccolo
{
    void VulkanRHI::initialize(RHIInitInfo init_info)
    {
        // Vulkan窗口对象初始化
        m_window                       = init_info.m_window_system->getWindow();
        std::array<int, 2> window_size = init_info.m_window_system->getWindowSize();

        // 视口初始化（详见视口变换与裁剪坐标）
        m_viewport = {0.0f, 0.0f, static_cast<float>(window_size[0]), static_cast<float>(window_size[1]), 0.0f, 1.0f};
        // 裁剪坐标初始化
        m_scissor = {{0, 0}, {static_cast<uint32_t>(window_size[0]), static_cast<uint32_t>(window_size[1])}};

        // 是否使用Vulkan Debug工具（vscode cmake tools会自动配置NDEBUG宏）
#ifndef NDEBUG
        m_enable_validation_layers = true;
        m_enable_debug_utils_label = true;
#else
        m_enable_validation_layers = false;
        m_enable_debug_utils_label = false;
#endif

        // Vulkan初始化步骤
        createInstance();
        initializeDebugMessenger();
        createWindowSurface();
        initializePhysicalDevice();
        createLogicDevice();
        createSwapchain();
        createSwapchainImageViews();
        createCommandPool();
        createCommandBuffers();
        createSyncPrimitives();
    }

    void VulkanRHI::prepareContext()
    {
        m_vk_current_command_buffer = m_vk_command_buffers[m_current_frame_index];
        static_cast<VulkanCommandBuffer*>(m_current_command_buffer)->setResource(m_vk_current_command_buffer);
    }

    // 初始化Vulkan实例：设置应用名称、版本、拓展模块等
    void VulkanRHI::createInstance()
    {
        if (m_enable_validation_layers && !checkValidationLayerSupport())
        {
            LOG_ERROR("Vulkan validation layers requested, but not available!");
        }

        m_vulkan_api_version = VK_API_VERSION_1_0;

        // app Info
        VkApplicationInfo app_info {};
        app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO; // 显式指定类型
        app_info.pApplicationName   = "piccolo_renderer";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName        = "Piccolo";
        app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion         = m_vulkan_api_version;

        // create info: 设置全局的扩展以及验证层
        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;

        auto extensions                              = getRequiredExtensions();
        instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        instance_create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
        if (m_enable_validation_layers)
        {
            instance_create_info.enabledLayerCount   = static_cast<uint32_t>(m_validation_layers.size());
            instance_create_info.ppEnabledLayerNames = m_validation_layers.data();

            populateDebugMessengerCreateInfo(debug_create_info);
            instance_create_info.pNext = &debug_create_info;
        }
        else
        {
            instance_create_info.enabledLayerCount = 0;
            instance_create_info.pNext             = nullptr;
        }

        // create vulkan_context._instance
        if (vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS)
        {
            LOG_ERROR("Vk create instance failed!");
        }
    }

    // 启动验证层从而在debug版本中发现可能存在的错误
    void VulkanRHI::initializeDebugMessenger()
    {
        if (m_enable_validation_layers)
        {
            VkDebugUtilsMessengerCreateInfoEXT create_info;
            populateDebugMessengerCreateInfo(create_info);
            if (createDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
            {
                LOG_ERROR("Failed to set debug messenger!");
            }
        }
    }

    // 链接之前的glfw，使vulkan与当前运行平台窗口系统兼容
    void VulkanRHI::createWindowSurface()
    {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        {
            LOG_ERROR("glfwCreateWindowSurface failed!");
        }
    }

    // 选择物理设备，并进行一些兼容性检查（比如你的rtx2060）
    void VulkanRHI::initializePhysicalDevice()
    {
        uint32_t physical_device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);
        if (physical_device_count == 0)
        {
            LOG_ERROR("Enumerate physical devices failed!");
        }
        else
        {
            // find one device that matches our requirement
            // or find which is the best
            std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
            vkEnumeratePhysicalDevices(m_instance, &physical_device_count, physical_devices.data());

            std::vector<std::pair<int, VkPhysicalDevice>> ranked_physical_devices;
            for (const auto& device : physical_devices)
            {
                VkPhysicalDeviceProperties physical_device_properties;
                vkGetPhysicalDeviceProperties(device, &physical_device_properties);

                int score = 0;
                if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    score = 1000;
                else if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                    score = 100;

                ranked_physical_devices.push_back({score, device});
            }

            std::sort(ranked_physical_devices.begin(),
                      ranked_physical_devices.end(),
                      [](const std::pair<int, VkPhysicalDevice>& p1, const std::pair<int, VkPhysicalDevice>& p2) {
                          return p1 > p2;
                      });

            for (const auto& device : ranked_physical_devices)
                if (isDeviceSuitable(device.second))
                {
                    m_physical_device = device.second;
                    break;
                }

            if (m_physical_device == VK_NULL_HANDLE)
            {
                LOG_ERROR("Failed to find suitable physical device!");
            }
        }
    }

    // 创建逻辑设备与准备队列，从而抽象你的物理设备为一些接口
    void VulkanRHI::createLogicDevice()
    {
        m_queue_indices = findQueueFamilies(m_physical_device);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos; // all queues that need to be created
        std::set<uint32_t>                   queue_families = {m_queue_indices.graphics_family.value()};

        float queue_priority = 1.0f;
        for (uint32_t queue_family : queue_families) // for every queue family
        {
            // queue create info
            VkDeviceQueueCreateInfo queue_create_info {};
            queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount       = 1; // 只需要一个队列
            // 分配优先级以影响命令缓冲执行的调度。就算只有一个队列，这个优先级也是必需的
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        // physical device features
        VkPhysicalDeviceFeatures physical_device_features = {};

        // device create info
        VkDeviceCreateInfo device_create_info {};
        device_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos       = queue_create_infos.data();
        device_create_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pEnabledFeatures        = &physical_device_features;
        device_create_info.enabledExtensionCount   = static_cast<uint32_t>(m_device_extensions.size());
        device_create_info.ppEnabledExtensionNames = m_device_extensions.data();
        device_create_info.enabledLayerCount       = 0;

        if (vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device) != VK_SUCCESS)
        {
            LOG_ERROR("Failed create logic device!");
        }

        // initialize queues of this device
        VkQueue vk_graphics_queue;
        vkGetDeviceQueue(m_device, m_queue_indices.graphics_family.value(), 0, &vk_graphics_queue);
        m_graphics_queue = new VulkanQueue();
        static_cast<VulkanQueue*>(m_graphics_queue)->setResource(vk_graphics_queue); // 绑定资源

        vkGetDeviceQueue(m_device, m_queue_indices.present_family.value(), 0, &m_present_queue);

        // create some function pointers that may not be available in some devices
        fn_vk_begin_command_buffer =
            reinterpret_cast<PFN_vkBeginCommandBuffer>(vkGetDeviceProcAddr(m_device, "vkBeginCommandBuffer"));
        fn_vk_end_command_buffer =
            reinterpret_cast<PFN_vkEndCommandBuffer>(vkGetDeviceProcAddr(m_device, "vkEndCommandBuffer"));
        fn_vk_cmd_bind_vertex_buffers =
            reinterpret_cast<PFN_vkCmdBindVertexBuffers>(vkGetDeviceProcAddr(m_device, "vkCmdBindVertexBuffers"));
        fn_vk_cmd_begin_render_pass =
            reinterpret_cast<PFN_vkCmdBeginRenderPass>(vkGetDeviceProcAddr(m_device, "vkCmdBeginRenderPass"));
        fn_vk_cmd_end_render_pass =
            reinterpret_cast<PFN_vkCmdEndRenderPass>(vkGetDeviceProcAddr(m_device, "vkCmdEndRenderPass"));
        fn_vk_cmd_bind_pipeline =
            reinterpret_cast<PFN_vkCmdBindPipeline>(vkGetDeviceProcAddr(m_device, "vkCmdBindPipeline"));
        fn_vk_cmd_set_viewport =
            reinterpret_cast<PFN_vkCmdSetViewport>(vkGetDeviceProcAddr(m_device, "vkCmdSetViewport"));
        fn_vk_cmd_set_scissor = reinterpret_cast<PFN_vkCmdSetScissor>(vkGetDeviceProcAddr(m_device, "vkCmdSetScissor"));
        fn_vk_wait_for_fences = reinterpret_cast<PFN_vkWaitForFences>(vkGetDeviceProcAddr(m_device, "vkWaitForFences"));
        fn_vk_reset_fences    = reinterpret_cast<PFN_vkResetFences>(vkGetDeviceProcAddr(m_device, "vkResetFences"));
    }

    // 创建交换链，选择最适合的属性
    void VulkanRHI::createSwapchain()
    {
        // query all supports of this physical device
        SwapChainSupportDetails swapchain_support_details = querySwapChainSupport(m_physical_device);

        // choose the best or fitting format
        VkSurfaceFormatKHR chosen_surface_format =
            chooseSwapchainSurfaceFormatFromDetails(swapchain_support_details.formats);
        // choose the best or fitting present mode
        VkPresentModeKHR chosen_present_mode =
            chooseSwapchainPresentModeFromDetails(swapchain_support_details.presentModes);
        // choose the best or fitting extent
        VkExtent2D chosen_extent = chooseSwapchainExtentFromDetails(swapchain_support_details.capabilities);

        // 图像渲染队列长度：尝试比默认最小值多一来实现三重缓冲
        uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1;
        // maxImageCount为0时意味着不限制内存使用，不为0时要放置image_count过界
        if (swapchain_support_details.capabilities.maxImageCount > 0 &&
            image_count > swapchain_support_details.capabilities.maxImageCount)
            image_count = swapchain_support_details.capabilities.maxImageCount;

        // 创建交换链对象
        VkSwapchainCreateInfoKHR create_info {};
        create_info.sType           = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface         = m_surface;
        create_info.minImageCount   = image_count;
        create_info.imageFormat     = chosen_surface_format.format;
        create_info.imageColorSpace = chosen_surface_format.colorSpace;
        create_info.imageExtent     = chosen_extent;
        // 每个图像所包含的图层的数量。除非你在开发一个立体的3D程序，它的值始终应该被设为1
        create_info.imageArrayLayers = 1;
        // 通过“颜色附件”的方式使用图像
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queue_family_indices[] = {m_queue_indices.graphics_family.value(),
                                           m_queue_indices.present_family.value()};
        if (m_queue_indices.graphics_family != m_queue_indices.present_family)
        {
            create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT; // 队列家族不同时用并发模式
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices   = queue_family_indices;
        }
        else
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // 队列家族相同时用显式转移图像所有权模式

        create_info.preTransform = swapchain_support_details.capabilities.currentTransform; // 不对交换链进行图像变换
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                     // 不进行alpha混合
        create_info.presentMode    = chosen_present_mode;
        create_info.clipped        = VK_TRUE; // 对遮挡的的像素进行裁剪

        create_info.oldSwapchain = VK_NULL_HANDLE; // 交换链重新创建的设置，此处假定不会重新创建

        if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
        {
            LOG_ERROR("Vk create swapchain khr failed!");
        }

        // 虽然已经显式的指明了image数量，但vulkan实际上可能创造更多，所以需要查询一次
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
        m_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data());

        // 保存交换链中的图像所选择的格式与范围到成员变量
        m_swapchain_image_format = static_cast<RHIFormat>(chosen_surface_format.format);
        m_swapchain_extent       = {chosen_extent.width, chosen_extent.height};
    }

    // 创建交换链图像视图
    void VulkanRHI::createSwapchainImageViews()
    {
        m_swapchain_imageviews.resize(m_swapchain_images.size());

        // create imageview (one for each this time) for all swapchain images
        for (size_t i = 0; i < m_swapchain_images.size(); i++)
        {
            VkImageView vk_image_view;
            vk_image_view             = VulkanUtil::createImageView(m_device,
                                                        m_swapchain_images[i],
                                                        static_cast<VkFormat>(m_swapchain_image_format),
                                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                                        VK_IMAGE_VIEW_TYPE_2D,
                                                        1,
                                                        1);
            m_swapchain_imageviews[i] = new VulkanImageView();
            static_cast<VulkanImageView*>(m_swapchain_imageviews[i])->setResource(vk_image_view);
        }
    }

    // 创建命令池，用于管理命令缓存的内存
    void VulkanRHI::createCommandPool()
    {
        // graphics command pool
        { // TODO: implement
            m_rhi_command_pool = new VulkanCommandPool();
            VkCommandPoolCreateInfo command_pool_create_info {};
            command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext            = nullptr;
            command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = m_queue_indices.graphics_family.value();

            for (auto& m_command_pool : m_command_pools)
                if (vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &m_command_pool) != VK_SUCCESS)
                {
                    LOG_ERROR("Vulkan failed to create command pool");
                }
        }
    }

    // 创建命令缓冲
    void VulkanRHI::createCommandBuffers()
    {
        // set the command buffer allocator information
        VkCommandBufferAllocateInfo command_buffer_allocate_info {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        // can be pushed to a queue to execute but can not be called from any other command
        command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1U;

        // bind command buffer to command pool

        // 下面的代码将会与piccolo有所出入
        for (uint32_t i = 0; i < m_k_max_frames_in_flight; i++)
        {
            command_buffer_allocate_info.commandPool = m_command_pools[i];
            VkCommandBuffer vk_command_buffer;
            if (vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &vk_command_buffer) != VK_SUCCESS)
            {
                LOG_ERROR("Vulkan failed to allocate command buffers!");
            }
            // command buffer resource binding
            m_vk_command_buffers[i] = vk_command_buffer;
            m_command_buffers[i]    = new VulkanCommandBuffer();
            static_cast<VulkanCommandBuffer*>(m_command_buffers[i])->setResource(vk_command_buffer);
        }
    }

    // 创造同步元：信号量与栅栏
    // semaphore : signal an image is ready for rendering // ready for presentation
    // (m_vulkan_context._swapchain_images --> semaphores, fences)
    void VulkanRHI::createSyncPrimitives()
    {
        // sem: thread will wait for another thread signal specific sem (it makes it > 0);
        VkSemaphoreCreateInfo semaphore_create_info {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // thread will stop until the work(be fenced) has done
        VkFenceCreateInfo fence_create_info {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // the fence is initialized as signaled to allow the first frame to be renderded
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t i = 0; i < m_k_max_frames_in_flight; i++)
        {
            if (vkCreateSemaphore(
                    m_device, &semaphore_create_info, nullptr, &m_image_available_for_render_semaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateSemaphore(
                    m_device, &semaphore_create_info, nullptr, &m_image_finished_for_presentation_semaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateFence(m_device, &fence_create_info, nullptr, &m_is_frame_in_flight_fences[i]) != VK_SUCCESS)
            {
                LOG_ERROR("Vulkan failed to  create semaphore");
            }
        }
    }

    // 检查是否所有被请求的层都可用
    bool VulkanRHI::checkValidationLayerSupport()
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr); // 获得所有可用的层数量

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()); // 获得所有可用的层属性

        for (const char* layer_name : m_validation_layers)
        { // 查找所有的验证层是否都可用
            bool layer_found = false;

            for (const auto& layer_properties : available_layers)
                if (std::strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }

            if (!layer_found)
                return false;
        }

        return RHI_SUCCESS;
    }

    // 根据启用的验证层返回我们需要的插件列表(glfw)
    std::vector<const char*> VulkanRHI::getRequiredExtensions()
    {
        uint32_t     glfw_extension_count = 0;
        const char** glfw_extensions      = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        if (m_enable_validation_layers || m_enable_debug_utils_label)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug消息回调函数需要这个插件

        return extensions;
    }

    // debug callback
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                        VkDebugUtilsMessageTypeFlagsEXT,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void*)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    void VulkanRHI::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo                 = {};
        createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = // 回调函数在何种严重等级下被触发
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = // 过滤回调函数的消息类型
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback; // debug回调函数
    }

    // 创建Debug信使扩展对象
    VkResult VulkanRHI::createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks*              pAllocator,
                                                     VkDebugUtilsMessengerEXT*                 pDebugMessenger)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // 获得所需队列家族索引
    QueueFamilyIndices VulkanRHI::findQueueFamilies(VkPhysicalDevice physical_device) // for device and surface
    {
        QueueFamilyIndices indices;
        uint32_t           queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        int i = 0;
        for (const auto& queue_family : queue_families)
        {
            // find if support graphics command queue
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphics_family = i;

            // find if support surface presentation
            VkBool32 is_present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, m_surface, &is_present_support);
            if (is_present_support)
                indices.present_family = i;

            // 各个被需求的队列家族都拥有对应索引后退出
            if (indices.isComplete())
                break; // 很有可能最后得到了同一个队列家族，但是在整个程序中，我们将它们视为两个不同的独立队列
            i++;
        }

        return indices;
    }

    // 检查物理设备是否支持需要的插件
    bool VulkanRHI::checkDeviceExtensionSupport(VkPhysicalDevice physical_device)
    {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

        // 检查是否每个被需要的插件都已经在可用插件集中了
        std::set<std::string> required_extensions(m_device_extensions.begin(), m_device_extensions.end());
        for (const auto& extension : available_extensions)
            required_extensions.erase(extension.extensionName);

        return required_extensions.empty();
    }

    // 获得物理设备对所需交换链属性的支持性
    SwapChainSupportDetails VulkanRHI::querySwapChainSupport(VkPhysicalDevice physical_device)
    {
        SwapChainSupportDetails details;

        // capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_surface, &details.capabilities);

        // formats
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, nullptr);
        if (format_count != 0)
        {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, details.formats.data());
        }

        // present modes
        uint32_t presentmode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &presentmode_count, nullptr);
        if (presentmode_count != 0)
        {
            details.presentModes.resize(presentmode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device, m_surface, &presentmode_count, details.presentModes.data());
        }

        return details;
    }

    // 检查该物理设备是否适合：①所需队列家族是否存在②交换链与物理设备设备的兼容性检查
    bool VulkanRHI::isDeviceSuitable(VkPhysicalDevice physical_device)
    {
        // ①所需队列家族是否存在
        auto queue_indices = findQueueFamilies(physical_device);

        // ②交换链所需属性与物理设备设备的兼容性检查
        bool is_extensions_supported = checkDeviceExtensionSupport(physical_device);
        bool is_swapchain_adequate   = false;
        if (is_extensions_supported)
        {
            SwapChainSupportDetails swapchain_support_details = querySwapChainSupport(physical_device);
            is_swapchain_adequate =
                !swapchain_support_details.formats.empty() && !swapchain_support_details.presentModes.empty();
        }

        return queue_indices.isComplete() && is_swapchain_adequate;
    }

    // 选择可用交换链中最好的表面格式（色彩深度）
    VkSurfaceFormatKHR
    VulkanRHI::chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats)
    {
        for (const auto& surface_format : available_surface_formats)
        {
            // TODO: select the VK_FORMAT_B8G8R8A8_SRGB surface format,
            // there is no need to do gamma correction in the fragment shader
            if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return surface_format; // 最好格式：sRGB
        }
        return available_surface_formats[0]; // 当找不到sRGB支持时直接返回第一个表面格式
    }

    // 选择可用交换链中最好的显示模式（在屏幕上“交换”图像的条件）
    VkPresentModeKHR
    VulkanRHI::chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes)
    {
        for (VkPresentModeKHR present_mode : available_present_modes)
            if (VK_PRESENT_MODE_MAILBOX_KHR == present_mode)
                return VK_PRESENT_MODE_MAILBOX_KHR; // 最好模式为：三缓冲（需求更多显存，但延迟相对低）

        return VK_PRESENT_MODE_FIFO_KHR; // 默认情况为双缓冲：队列储存图像
    }

    VkExtent2D VulkanRHI::chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        else // 如果当前的宽高为UINT32_MAX，意味着窗体管理器允许我们做出自己的设置，则选择在minImageExtent与maxImageExtent之间最符合窗口分辨率的分辨率
        {
            int width, height;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            actual_extent.width =
                std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(
                actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actual_extent;
        }
    }

    // 封装一个RHIShader(存储VkShaderModule)
    RHIShader* VulkanRHI::createShaderModule(const std::vector<unsigned char>& shader_code)
    {
        RHIShader*     shader    = new VulkanShader();
        VkShaderModule vk_shader = VulkanUtil::createShaderModule(m_device, shader_code);
        static_cast<VulkanShader*>(shader)->setResource(vk_shader);
        return shader;
    }

    bool VulkanRHI::createGraphicsPipelines(RHIPipelineCache*                    pipelineCache,
                                            uint32_t                             createInfoCount,
                                            const RHIGraphicsPipelineCreateInfo* pCreateInfo,
                                            RHIPipeline*&                        pPipelines)
    {
        // convert shader
        int                                          pipeline_shader_stage_create_info_size = pCreateInfo->stageCount;
        std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_info_list(
            pipeline_shader_stage_create_info_size);

        for (int i = 0; i < pipeline_shader_stage_create_info_size; i++)
        {
            const auto& rhi_pipeline_shader_stage_create_info_element = pCreateInfo->pStages[i];
            auto&       vk_pipeline_shader_stage_create_info_element  = vk_pipeline_shader_stage_create_info_list[i];

            vk_pipeline_shader_stage_create_info_element.sType =
                static_cast<VkStructureType>(rhi_pipeline_shader_stage_create_info_element.sType);
            vk_pipeline_shader_stage_create_info_element.stage =
                static_cast<VkShaderStageFlagBits>(rhi_pipeline_shader_stage_create_info_element.stage);
            vk_pipeline_shader_stage_create_info_element.module =
                static_cast<VulkanShader*>(rhi_pipeline_shader_stage_create_info_element.module)->getResource();
            vk_pipeline_shader_stage_create_info_element.pName = rhi_pipeline_shader_stage_create_info_element.pName;
        }

        // convert vertex input
        int vertex_input_binding_description_size = pCreateInfo->pVertexInputState->vertexBindingDescriptionCount;
        std::vector<VkVertexInputBindingDescription> vk_vertex_input_binding_description_list(
            vertex_input_binding_description_size);
        for (int i = 0; i < vertex_input_binding_description_size; ++i)
        {
            const auto& rhi_vertex_input_binding_description_element =
                pCreateInfo->pVertexInputState->pVertexBindingDescriptions[i];
            auto& vk_vertex_input_binding_description_element = vk_vertex_input_binding_description_list[i];

            vk_vertex_input_binding_description_element.binding = rhi_vertex_input_binding_description_element.binding;
            vk_vertex_input_binding_description_element.stride  = rhi_vertex_input_binding_description_element.stride;
            vk_vertex_input_binding_description_element.inputRate =
                static_cast<VkVertexInputRate>(rhi_vertex_input_binding_description_element.inputRate);
        };

        // specify the vertex input attribute
        int vertex_input_attribute_description_size = pCreateInfo->pVertexInputState->vertexAttributeDescriptionCount;
        std::vector<VkVertexInputAttributeDescription> vk_vertex_input_attribute_description_list(
            vertex_input_attribute_description_size);
        for (int i = 0; i < vertex_input_attribute_description_size; ++i)
        {
            const auto& rhi_vertex_input_attribute_description_element =
                pCreateInfo->pVertexInputState->pVertexAttributeDescriptions[i];
            auto& vk_vertex_input_attribute_description_element = vk_vertex_input_attribute_description_list[i];

            vk_vertex_input_attribute_description_element.location =
                rhi_vertex_input_attribute_description_element.location;
            vk_vertex_input_attribute_description_element.binding =
                rhi_vertex_input_attribute_description_element.binding;
            vk_vertex_input_attribute_description_element.format =
                static_cast<VkFormat>(rhi_vertex_input_attribute_description_element.format);
            vk_vertex_input_attribute_description_element.offset =
                rhi_vertex_input_attribute_description_element.offset;
        };

        // set aside
        VkPipelineVertexInputStateCreateInfo vk_pipeline_vertex_input_state_create_info {};
        vk_pipeline_vertex_input_state_create_info.sType =
            static_cast<VkStructureType>(pCreateInfo->pVertexInputState->sType);
        vk_pipeline_vertex_input_state_create_info.pNext = pCreateInfo->pVertexInputState->pNext;
        vk_pipeline_vertex_input_state_create_info.flags =
            static_cast<VkPipelineVertexInputStateCreateFlags>(pCreateInfo->pVertexInputState->flags);
        vk_pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount =
            pCreateInfo->pVertexInputState->vertexBindingDescriptionCount;
        vk_pipeline_vertex_input_state_create_info.pVertexBindingDescriptions =
            vk_vertex_input_binding_description_list.data();
        vk_pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount =
            pCreateInfo->pVertexInputState->vertexAttributeDescriptionCount;
        vk_pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions =
            vk_vertex_input_attribute_description_list.data();

        VkPipelineInputAssemblyStateCreateInfo vk_pipeline_input_assembly_state_create_info {};
        vk_pipeline_input_assembly_state_create_info.sType =
            static_cast<VkStructureType>(pCreateInfo->pInputAssemblyState->sType);
        vk_pipeline_input_assembly_state_create_info.pNext = pCreateInfo->pInputAssemblyState->pNext;
        vk_pipeline_input_assembly_state_create_info.flags =
            static_cast<VkPipelineInputAssemblyStateCreateFlags>(pCreateInfo->pInputAssemblyState->flags);
        vk_pipeline_input_assembly_state_create_info.topology =
            static_cast<VkPrimitiveTopology>(pCreateInfo->pInputAssemblyState->topology);
        vk_pipeline_input_assembly_state_create_info.primitiveRestartEnable =
            static_cast<VkBool32>(pCreateInfo->pInputAssemblyState->primitiveRestartEnable);

        // viewport
        int                     viewport_size = pCreateInfo->pViewportState->viewportCount;
        std::vector<VkViewport> vk_viewport_list(viewport_size);
        for (int i = 0; i < viewport_size; ++i)
        {
            const auto& rhi_viewport_element = pCreateInfo->pViewportState->pViewports[i];
            auto&       vk_viewport_element  = vk_viewport_list[i];

            vk_viewport_element.x        = rhi_viewport_element.x;
            vk_viewport_element.y        = rhi_viewport_element.y;
            vk_viewport_element.width    = rhi_viewport_element.width;
            vk_viewport_element.height   = rhi_viewport_element.height;
            vk_viewport_element.minDepth = rhi_viewport_element.minDepth;
            vk_viewport_element.maxDepth = rhi_viewport_element.maxDepth;
        };

        // scissor
        int                   rect_2d_size = pCreateInfo->pViewportState->scissorCount;
        std::vector<VkRect2D> vk_rect_2d_list(rect_2d_size);
        for (int i = 0; i < rect_2d_size; ++i)
        {
            const auto& rhi_rect_2d_element = pCreateInfo->pViewportState->pScissors[i];
            auto&       vk_rect_2d_element  = vk_rect_2d_list[i];

            VkOffset2D offset2d {};
            offset2d.x = rhi_rect_2d_element.offset.x;
            offset2d.y = rhi_rect_2d_element.offset.y;

            VkExtent2D extend2d {};
            extend2d.width  = rhi_rect_2d_element.extent.width;
            extend2d.height = rhi_rect_2d_element.extent.height;

            vk_rect_2d_element.offset = offset2d;
            vk_rect_2d_element.extent = extend2d;
        };

        VkPipelineViewportStateCreateInfo vk_pipeline_viewport_state_create_info {};
        vk_pipeline_viewport_state_create_info.sType = static_cast<VkStructureType>(pCreateInfo->pViewportState->sType);
        vk_pipeline_viewport_state_create_info.pNext = pCreateInfo->pViewportState->pNext;
        vk_pipeline_viewport_state_create_info.flags =
            static_cast<VkPipelineViewportStateCreateFlags>(pCreateInfo->pViewportState->flags);
        vk_pipeline_viewport_state_create_info.viewportCount = pCreateInfo->pViewportState->viewportCount;
        vk_pipeline_viewport_state_create_info.pViewports    = vk_viewport_list.data();
        vk_pipeline_viewport_state_create_info.scissorCount  = pCreateInfo->pViewportState->scissorCount;
        vk_pipeline_viewport_state_create_info.pScissors     = vk_rect_2d_list.data();

        // rasterization
        VkPipelineRasterizationStateCreateInfo vk_pipeline_rasterization_state_create_info {};
        vk_pipeline_rasterization_state_create_info.sType =
            static_cast<VkStructureType>(pCreateInfo->pRasterizationState->sType);
        vk_pipeline_rasterization_state_create_info.pNext = pCreateInfo->pRasterizationState->pNext;
        vk_pipeline_rasterization_state_create_info.flags =
            static_cast<VkPipelineRasterizationStateCreateFlags>(pCreateInfo->pRasterizationState->flags);
        vk_pipeline_rasterization_state_create_info.depthClampEnable =
            static_cast<VkBool32>(pCreateInfo->pRasterizationState->depthClampEnable);
        vk_pipeline_rasterization_state_create_info.rasterizerDiscardEnable =
            static_cast<VkBool32>(pCreateInfo->pRasterizationState->rasterizerDiscardEnable);
        vk_pipeline_rasterization_state_create_info.polygonMode =
            static_cast<VkPolygonMode>(pCreateInfo->pRasterizationState->polygonMode);
        vk_pipeline_rasterization_state_create_info.cullMode =
            static_cast<VkCullModeFlags>(pCreateInfo->pRasterizationState->cullMode);
        vk_pipeline_rasterization_state_create_info.frontFace =
            static_cast<VkFrontFace>(pCreateInfo->pRasterizationState->frontFace);
        vk_pipeline_rasterization_state_create_info.depthBiasEnable =
            static_cast<VkBool32>(pCreateInfo->pRasterizationState->depthBiasEnable);
        vk_pipeline_rasterization_state_create_info.depthBiasConstantFactor =
            pCreateInfo->pRasterizationState->depthBiasConstantFactor;
        vk_pipeline_rasterization_state_create_info.depthBiasClamp = pCreateInfo->pRasterizationState->depthBiasClamp;
        vk_pipeline_rasterization_state_create_info.depthBiasSlopeFactor =
            pCreateInfo->pRasterizationState->depthBiasSlopeFactor;
        vk_pipeline_rasterization_state_create_info.lineWidth = pCreateInfo->pRasterizationState->lineWidth;

        // MSAA
        VkPipelineMultisampleStateCreateInfo vk_pipeline_multisample_state_create_info {};
        vk_pipeline_multisample_state_create_info.sType =
            static_cast<VkStructureType>(pCreateInfo->pMultisampleState->sType);
        vk_pipeline_multisample_state_create_info.pNext = pCreateInfo->pMultisampleState->pNext;
        vk_pipeline_multisample_state_create_info.flags =
            static_cast<VkPipelineMultisampleStateCreateFlags>(pCreateInfo->pMultisampleState->flags);
        vk_pipeline_multisample_state_create_info.rasterizationSamples =
            static_cast<VkSampleCountFlagBits>(pCreateInfo->pMultisampleState->rasterizationSamples);
        vk_pipeline_multisample_state_create_info.sampleShadingEnable =
            static_cast<VkBool32>(pCreateInfo->pMultisampleState->sampleShadingEnable);
        vk_pipeline_multisample_state_create_info.minSampleShading = pCreateInfo->pMultisampleState->minSampleShading;
        vk_pipeline_multisample_state_create_info.pSampleMask =
            reinterpret_cast<const RHISampleMask*>(pCreateInfo->pMultisampleState->pSampleMask);
        vk_pipeline_multisample_state_create_info.alphaToCoverageEnable =
            static_cast<VkBool32>(pCreateInfo->pMultisampleState->alphaToCoverageEnable);
        vk_pipeline_multisample_state_create_info.alphaToOneEnable =
            static_cast<VkBool32>(pCreateInfo->pMultisampleState->alphaToOneEnable);

        // color blend
        int pipeline_color_blend_attachment_state_size = pCreateInfo->pColorBlendState->attachmentCount;
        std::vector<VkPipelineColorBlendAttachmentState> vk_pipeline_color_blend_attachment_state_list(
            pipeline_color_blend_attachment_state_size);
        for (int i = 0; i < pipeline_color_blend_attachment_state_size; ++i)
        {
            const auto& rhi_pipeline_color_blend_attachment_state_element =
                pCreateInfo->pColorBlendState->pAttachments[i];
            auto& vk_pipeline_color_blend_attachment_state_element = vk_pipeline_color_blend_attachment_state_list[i];

            vk_pipeline_color_blend_attachment_state_element.blendEnable =
                static_cast<VkBool32>(rhi_pipeline_color_blend_attachment_state_element.blendEnable);
            vk_pipeline_color_blend_attachment_state_element.srcColorBlendFactor =
                static_cast<VkBlendFactor>(rhi_pipeline_color_blend_attachment_state_element.srcColorBlendFactor);
            vk_pipeline_color_blend_attachment_state_element.dstColorBlendFactor =
                static_cast<VkBlendFactor>(rhi_pipeline_color_blend_attachment_state_element.dstColorBlendFactor);
            vk_pipeline_color_blend_attachment_state_element.colorBlendOp =
                static_cast<VkBlendOp>(rhi_pipeline_color_blend_attachment_state_element.colorBlendOp);
            vk_pipeline_color_blend_attachment_state_element.srcAlphaBlendFactor =
                static_cast<VkBlendFactor>(rhi_pipeline_color_blend_attachment_state_element.srcAlphaBlendFactor);
            vk_pipeline_color_blend_attachment_state_element.dstAlphaBlendFactor =
                static_cast<VkBlendFactor>(rhi_pipeline_color_blend_attachment_state_element.dstAlphaBlendFactor);
            vk_pipeline_color_blend_attachment_state_element.alphaBlendOp =
                static_cast<VkBlendOp>(rhi_pipeline_color_blend_attachment_state_element.alphaBlendOp);
            vk_pipeline_color_blend_attachment_state_element.colorWriteMask =
                static_cast<VkColorComponentFlags>(rhi_pipeline_color_blend_attachment_state_element.colorWriteMask);
        };

        VkPipelineColorBlendStateCreateInfo vk_pipeline_color_blend_state_create_info {};
        vk_pipeline_color_blend_state_create_info.sType =
            static_cast<VkStructureType>(pCreateInfo->pColorBlendState->sType);
        vk_pipeline_color_blend_state_create_info.pNext         = pCreateInfo->pColorBlendState->pNext;
        vk_pipeline_color_blend_state_create_info.flags         = pCreateInfo->pColorBlendState->flags;
        vk_pipeline_color_blend_state_create_info.logicOpEnable = pCreateInfo->pColorBlendState->logicOpEnable;
        vk_pipeline_color_blend_state_create_info.logicOp =
            static_cast<VkLogicOp>(pCreateInfo->pColorBlendState->logicOp);
        vk_pipeline_color_blend_state_create_info.attachmentCount = pCreateInfo->pColorBlendState->attachmentCount;
        vk_pipeline_color_blend_state_create_info.pAttachments = vk_pipeline_color_blend_attachment_state_list.data();
        for (int i = 0; i < 4; ++i)
        {
            vk_pipeline_color_blend_state_create_info.blendConstants[i] =
                pCreateInfo->pColorBlendState->blendConstants[i];
        };

        // dynamic
        int                         dynamic_state_size = pCreateInfo->pDynamicState->dynamicStateCount;
        std::vector<VkDynamicState> vk_dynamic_state_list(dynamic_state_size);
        for (int i = 0; i < dynamic_state_size; ++i)
        {
            const auto& rhi_dynamic_state_element = pCreateInfo->pDynamicState->pDynamicStates[i];
            auto&       vk_dynamic_state_element  = vk_dynamic_state_list[i];

            vk_dynamic_state_element = static_cast<VkDynamicState>(rhi_dynamic_state_element);
        };

        VkPipelineDynamicStateCreateInfo vk_pipeline_dynamic_state_create_info {};
        vk_pipeline_dynamic_state_create_info.sType = static_cast<VkStructureType>(pCreateInfo->pDynamicState->sType);
        vk_pipeline_dynamic_state_create_info.pNext = pCreateInfo->pDynamicState->pNext;
        vk_pipeline_dynamic_state_create_info.flags =
            static_cast<VkPipelineDynamicStateCreateFlags>(pCreateInfo->pDynamicState->flags);
        vk_pipeline_dynamic_state_create_info.dynamicStateCount = pCreateInfo->pDynamicState->dynamicStateCount;
        vk_pipeline_dynamic_state_create_info.pDynamicStates    = vk_dynamic_state_list.data();

        VkGraphicsPipelineCreateInfo create_info {};
        create_info.sType               = static_cast<VkStructureType>(pCreateInfo->sType);
        create_info.pNext               = pCreateInfo->pNext;
        create_info.flags               = static_cast<VkPipelineCreateFlags>(pCreateInfo->flags);
        create_info.stageCount          = pCreateInfo->stageCount;
        create_info.pStages             = vk_pipeline_shader_stage_create_info_list.data();
        create_info.pVertexInputState   = &vk_pipeline_vertex_input_state_create_info;
        create_info.pInputAssemblyState = &vk_pipeline_input_assembly_state_create_info;
        // create_info.pTessellationState  = vk_pipeline_tessellation_state_create_info_ptr;
        create_info.pViewportState      = &vk_pipeline_viewport_state_create_info;
        create_info.pRasterizationState = &vk_pipeline_rasterization_state_create_info;
        create_info.pMultisampleState   = &vk_pipeline_multisample_state_create_info;
        create_info.pColorBlendState    = &vk_pipeline_color_blend_state_create_info;
        // create_info.pDepthStencilState  = &vk_pipeline_depth_stencil_state_create_info;
        create_info.pDynamicState = &vk_pipeline_dynamic_state_create_info;
        create_info.layout        = static_cast<VulkanPipelineLayout*>(pCreateInfo->layout)->getResource();
        create_info.renderPass    = static_cast<VulkanRenderPass*>(pCreateInfo->renderPass)->getResource();
        create_info.subpass       = pCreateInfo->subpass;
        if (pCreateInfo->basePipelineHandle != nullptr)
            create_info.basePipelineHandle =
                static_cast<VulkanPipeline*>(pCreateInfo->basePipelineHandle)->getResource();
        else
            create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = pCreateInfo->basePipelineIndex;

        pPipelines = new VulkanPipeline();
        VkPipeline      vk_pipelines;
        VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
        if (pipelineCache != nullptr)
            vk_pipeline_cache = static_cast<VulkanPipelineCache*>(pipelineCache)->getResource();
        VkResult result = vkCreateGraphicsPipelines(
            m_device, vk_pipeline_cache, createInfoCount, &create_info, nullptr, &vk_pipelines);
        static_cast<VulkanPipeline*>(pPipelines)->setResource(vk_pipelines);

        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Vulkan failed to create GraphicsPipelines!");
            return false;
        }
        return RHI_SUCCESS;
    }

    bool VulkanRHI::createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo,
                                         RHIPipelineLayout*&                pPipelineLayout)
    {
        VkPipelineLayoutCreateInfo create_info {};
        create_info.sType          = static_cast<VkStructureType>(pCreateInfo->sType);
        create_info.pNext          = pCreateInfo->pNext;
        create_info.flags          = static_cast<VkPipelineLayoutCreateFlags>(pCreateInfo->flags);
        create_info.setLayoutCount = pCreateInfo->setLayoutCount;
        create_info.pSetLayouts    = nullptr; // TODO: layout descriptor

        pPipelineLayout = new VulkanPipelineLayout();
        VkPipelineLayout vk_pipeline_layout;
        VkResult         result = vkCreatePipelineLayout(m_device, &create_info, nullptr, &vk_pipeline_layout);
        static_cast<VulkanPipelineLayout*>(pPipelineLayout)->setResource(vk_pipeline_layout);

        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create Vulkan pipeline layout!");
            return false;
        }
        return RHI_SUCCESS;
    }

    bool VulkanRHI::createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass)
    {
        // attachment convert
        std::vector<VkAttachmentDescription> vk_attachments(pCreateInfo->attachmentCount);
        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i)
        {
            const auto& rhi_desc = pCreateInfo->pAttachments[i];
            auto&       vk_desc  = vk_attachments[i];

            vk_desc.format         = static_cast<VkFormat>((rhi_desc).format);
            vk_desc.samples        = static_cast<VkSampleCountFlagBits>((rhi_desc).samples);
            vk_desc.loadOp         = static_cast<VkAttachmentLoadOp>((rhi_desc).loadOp);
            vk_desc.storeOp        = static_cast<VkAttachmentStoreOp>((rhi_desc).storeOp);
            vk_desc.stencilLoadOp  = static_cast<VkAttachmentLoadOp>((rhi_desc).stencilLoadOp);
            vk_desc.stencilStoreOp = static_cast<VkAttachmentStoreOp>((rhi_desc).stencilStoreOp);
            vk_desc.initialLayout  = static_cast<VkImageLayout>((rhi_desc).initialLayout);
            vk_desc.finalLayout    = static_cast<VkImageLayout>((rhi_desc).finalLayout);
        };

        // subpass convert
        int total_attachment_refenrence = 0;
        for (uint32_t i = 0; i < pCreateInfo->subpassCount; i++)
        {
            const auto& rhi_desc = pCreateInfo->pSubpasses[i];
            total_attachment_refenrence += rhi_desc.colorAttachmentCount; // pColorAttachments
        }
        std::vector<VkSubpassDescription>  vk_subpass_description(pCreateInfo->subpassCount);
        std::vector<VkAttachmentReference> vk_attachment_reference(total_attachment_refenrence);
        int                                current_attachment_refence = 0;
        for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i)
        {
            const auto& rhi_desc = pCreateInfo->pSubpasses[i];
            auto&       vk_desc  = vk_subpass_description[i];

            vk_desc.pipelineBindPoint    = static_cast<VkPipelineBindPoint>((rhi_desc).pipelineBindPoint);
            vk_desc.colorAttachmentCount = (rhi_desc).colorAttachmentCount;
            vk_desc.pColorAttachments    = &vk_attachment_reference[current_attachment_refence];
            for (uint32_t i = 0; i < (rhi_desc).colorAttachmentCount; ++i)
            {
                const auto& rhi_attachment_refence_color = (rhi_desc).pColorAttachments[i];
                auto&       vk_attachment_refence_color  = vk_attachment_reference[current_attachment_refence];

                vk_attachment_refence_color.attachment = rhi_attachment_refence_color.attachment;
                vk_attachment_refence_color.layout = static_cast<VkImageLayout>(rhi_attachment_refence_color.layout);

                current_attachment_refence += 1;
            };
        }
        if (current_attachment_refence != total_attachment_refenrence)
        {
            LOG_ERROR("currentAttachmentRefence != totalAttachmentRefenrence");
            return false;
        }

        // specify the subpass dependency
        std::vector<VkSubpassDependency> vk_subpass_depandecy(pCreateInfo->dependencyCount);
        for (int i = 0; i < pCreateInfo->dependencyCount; ++i)
        {
            const auto& rhi_desc = pCreateInfo->pDependencies[i];
            auto&       vk_desc  = vk_subpass_depandecy[i];

            vk_desc.srcSubpass = rhi_desc.srcSubpass; // which will be depended on
            vk_desc.dstSubpass = rhi_desc.dstSubpass; // which has dependency
            // in which stage will the depend happen
            vk_desc.srcStageMask = static_cast<VkPipelineStageFlags>((rhi_desc).srcStageMask);
            vk_desc.dstStageMask = static_cast<VkPipelineStageFlags>((rhi_desc).dstStageMask);
            // which operation needs dependency
            vk_desc.srcAccessMask   = static_cast<VkAccessFlags>((rhi_desc).srcAccessMask);
            vk_desc.dstAccessMask   = static_cast<VkAccessFlags>((rhi_desc).dstAccessMask);
            vk_desc.dependencyFlags = static_cast<VkDependencyFlags>((rhi_desc).dependencyFlags);
        };

        // render pass convert
        VkRenderPassCreateInfo create_info {};
        create_info.sType           = static_cast<VkStructureType>(pCreateInfo->sType);
        create_info.attachmentCount = pCreateInfo->attachmentCount;
        create_info.pAttachments    = vk_attachments.data();
        create_info.subpassCount    = pCreateInfo->subpassCount;
        create_info.pSubpasses      = vk_subpass_description.data();
        create_info.dependencyCount = pCreateInfo->dependencyCount;
        create_info.pDependencies   = vk_subpass_depandecy.data();

        pRenderPass = new VulkanRenderPass();
        VkRenderPass vk_render_pass;
        VkResult     result = vkCreateRenderPass(m_device, &create_info, nullptr, &vk_render_pass);
        static_cast<VulkanRenderPass*>(pRenderPass)->setResource(vk_render_pass);

        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Vulkan failed to create RenderPass!");
            return false;
        }
        return RHI_SUCCESS;
    }

    bool VulkanRHI::createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer)
    {
        // image view
        int                      image_view_size = pCreateInfo->attachmentCount;
        std::vector<VkImageView> vk_image_view_list(image_view_size);
        for (int i = 0; i < image_view_size; i++)
        {
            const auto& rhi_image_view_element = pCreateInfo->pAttachments[i];
            auto&       vk_image_view_element  = vk_image_view_list[i];

            vk_image_view_element = static_cast<VulkanImageView*>(rhi_image_view_element)->getResource();
        }

        // frame buffer
        VkFramebufferCreateInfo create_info {};
        create_info.sType           = static_cast<VkStructureType>(pCreateInfo->sType);
        create_info.pNext           = pCreateInfo->pNext;
        create_info.flags           = static_cast<VkFramebufferCreateFlags>(pCreateInfo->flags);
        create_info.renderPass      = static_cast<VulkanRenderPass*>(pCreateInfo->renderPass)->getResource();
        create_info.attachmentCount = pCreateInfo->attachmentCount;
        create_info.pAttachments    = vk_image_view_list.data();
        create_info.width           = pCreateInfo->width;
        create_info.height          = pCreateInfo->height;
        create_info.layers          = pCreateInfo->layers;

        pFramebuffer = new VulkanFramebuffer();
        VkFramebuffer vk_framebuffer;
        VkResult      result = vkCreateFramebuffer(m_device, &create_info, nullptr, &vk_framebuffer);
        static_cast<VulkanFramebuffer*>(pFramebuffer)->setResource(vk_framebuffer);

        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Vulkan failed to create Framebuffer!");
            return false;
        }
        return RHI_SUCCESS;
    }

    RHICommandBuffer* VulkanRHI::getCurrentCommandBuffer() const { return m_current_command_buffer; }

    RHISwapChainDesc VulkanRHI::getSwapchainInfo()
    {
        RHISwapChainDesc desc;
        desc.image_format = m_swapchain_image_format;
        desc.extent       = m_swapchain_extent;
        desc.viewport     = &m_viewport;
        desc.scissor      = &m_scissor;
        desc.imageViews   = m_swapchain_imageviews;
        return desc;
    }

    // 重建交换链
    void VulkanRHI::recreateSwapchain()
    {
        // get current window width and height
        int width  = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0) // minimized 0,0, pause for now
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents(); // pause
        }

        if (fn_vk_wait_for_fences(
                m_device, m_k_max_frames_in_flight, m_is_frame_in_flight_fences, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
        {
            LOG_ERROR("vkWaitForFences failed");
            return;
        }

        // destroy old swapchain(and the associated imageview)
        for (auto* imageview : m_swapchain_imageviews)
            vkDestroyImageView(m_device, static_cast<VulkanImageView*>(imageview)->getResource(), nullptr);
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

        // create new swapchain(and the associated imageview)
        createSwapchain();
        createSwapchainImageViews();
        // createFramebufferImageAndView();
    }

    void VulkanRHI::createBuffer(RHIDeviceSize          size,
                                 RHIBufferUsageFlags    usage,
                                 RHIMemoryPropertyFlags properties,
                                 RHIBuffer*&            buffer,
                                 RHIDeviceMemory*&      buffer_memory)
    {
        VkBuffer       vk_buffer;
        VkDeviceMemory vk_device_memory;

        VulkanUtil::createBuffer(m_physical_device, m_device, size, usage, properties, vk_buffer, vk_device_memory);

        buffer        = new VulkanBuffer();
        buffer_memory = new VulkanDeviceMemory();
        static_cast<VulkanBuffer*>(buffer)->setResource(vk_buffer);
        static_cast<VulkanDeviceMemory*>(buffer_memory)->setResource(vk_device_memory);
    }

    void VulkanRHI::cmdBindVertexBuffersPFN(RHICommandBuffer*    commandBuffer,
                                            uint32_t             firstBinding,
                                            uint32_t             bindingCount,
                                            RHIBuffer* const*    pBuffers,
                                            const RHIDeviceSize* pOffsets)
    {
        // buffer
        int                   buffer_size = bindingCount;
        std::vector<VkBuffer> vk_buffer_list(buffer_size);
        for (int i = 0; i < buffer_size; ++i)
        {
            const auto& rhi_buffer_element = pBuffers[i];
            auto&       vk_buffer_element  = vk_buffer_list[i];

            vk_buffer_element = static_cast<VulkanBuffer*>(rhi_buffer_element)->getResource();
        };

        // offset
        int                       offset_size = bindingCount;
        std::vector<VkDeviceSize> vk_device_size_list(offset_size);
        for (int i = 0; i < offset_size; ++i)
        {
            const auto& rhi_offset_element = pOffsets[i];
            auto&       vk_offset_element  = vk_device_size_list[i];

            vk_offset_element = rhi_offset_element;
        };

        return fn_vk_cmd_bind_vertex_buffers(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource(),
                                             firstBinding,
                                             bindingCount,
                                             vk_buffer_list.data(),
                                             vk_device_size_list.data());
    }

    // 启动渲染过程
    void VulkanRHI::cmdBeginRenderPassPFN(RHICommandBuffer*             commandBuffer,
                                          const RHIRenderPassBeginInfo* pRenderPassBegin,
                                          RHISubpassContents            contents)
    { // TODO: implement
        VkOffset2D offset_2d {};
        offset_2d.x = pRenderPassBegin->renderArea.offset.x;
        offset_2d.y = pRenderPassBegin->renderArea.offset.y;

        VkExtent2D extent_2d {};
        extent_2d.width  = pRenderPassBegin->renderArea.extent.width;
        extent_2d.height = pRenderPassBegin->renderArea.extent.height;

        VkRect2D rect_2d {};
        rect_2d.offset = offset_2d;
        rect_2d.extent = extent_2d;

        // clear_values
        int                       clear_value_size = pRenderPassBegin->clearValueCount;
        std::vector<VkClearValue> vk_clear_value_list(clear_value_size);
        for (int i = 0; i < clear_value_size; ++i)
        {
            const auto& rhi_clear_value_element = pRenderPassBegin->pClearValues[i];
            auto&       vk_clear_value_element  = vk_clear_value_list[i];

            VkClearColorValue vk_clear_color_value;
            vk_clear_color_value.float32[0] = rhi_clear_value_element.color.float32[0];
            vk_clear_color_value.float32[1] = rhi_clear_value_element.color.float32[1];
            vk_clear_color_value.float32[2] = rhi_clear_value_element.color.float32[2];
            vk_clear_color_value.float32[3] = rhi_clear_value_element.color.float32[3];
            vk_clear_color_value.int32[0]   = rhi_clear_value_element.color.int32[0];
            vk_clear_color_value.int32[1]   = rhi_clear_value_element.color.int32[1];
            vk_clear_color_value.int32[2]   = rhi_clear_value_element.color.int32[2];
            vk_clear_color_value.int32[3]   = rhi_clear_value_element.color.int32[3];
            vk_clear_color_value.uint32[0]  = rhi_clear_value_element.color.uint32[0];
            vk_clear_color_value.uint32[1]  = rhi_clear_value_element.color.uint32[1];
            vk_clear_color_value.uint32[2]  = rhi_clear_value_element.color.uint32[2];
            vk_clear_color_value.uint32[3]  = rhi_clear_value_element.color.uint32[3];

            VkClearDepthStencilValue vk_clear_depth_stencil_value;
            vk_clear_depth_stencil_value.depth   = rhi_clear_value_element.depthStencil.depth;
            vk_clear_depth_stencil_value.stencil = rhi_clear_value_element.depthStencil.stencil;

            vk_clear_value_element.color        = vk_clear_color_value;
            vk_clear_value_element.depthStencil = vk_clear_depth_stencil_value;
        };

        VkRenderPassBeginInfo vk_render_pass_begin_info {};
        vk_render_pass_begin_info.sType = static_cast<VkStructureType>(pRenderPassBegin->sType);
        vk_render_pass_begin_info.pNext = pRenderPassBegin->pNext;
        vk_render_pass_begin_info.renderPass =
            static_cast<VulkanRenderPass*>(pRenderPassBegin->renderPass)->getResource();
        vk_render_pass_begin_info.framebuffer =
            static_cast<VulkanFramebuffer*>(pRenderPassBegin->framebuffer)->getResource();
        vk_render_pass_begin_info.renderArea      = rect_2d; // where the shader effects
        vk_render_pass_begin_info.clearValueCount = pRenderPassBegin->clearValueCount;
        vk_render_pass_begin_info.pClearValues    = vk_clear_value_list.data();

        return fn_vk_cmd_begin_render_pass(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource(),
                                           &vk_render_pass_begin_info,
                                           static_cast<VkSubpassContents>(contents));
    }

    void VulkanRHI::cmdBindPipelinePFN(RHICommandBuffer*    commandBuffer,
                                       RHIPipelineBindPoint pipelineBindPoint,
                                       RHIPipeline*         pipeline)
    {
        return fn_vk_cmd_bind_pipeline(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource(),
                                       static_cast<VkPipelineBindPoint>(pipelineBindPoint),
                                       static_cast<VulkanPipeline*>(pipeline)->getResource());
    }

    void VulkanRHI::cmdDraw(RHICommandBuffer* commandBuffer,
                            uint32_t          vertexCount,
                            uint32_t          instanceCount,
                            uint32_t          firstVertex,
                            uint32_t          firstInstance)
    {
        vkCmdDraw(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource(),
                  vertexCount,
                  instanceCount,
                  firstVertex,
                  firstInstance);
    }

    void VulkanRHI::cmdEndRenderPassPFN(RHICommandBuffer* commandBuffer)
    {
        return fn_vk_cmd_end_render_pass(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource());
    }

    void VulkanRHI::cmdSetViewportPFN(RHICommandBuffer*  commandBuffer,
                                      uint32_t           firstViewport,
                                      uint32_t           viewportCount,
                                      const RHIViewport* pViewports)
    {
        // viewport
        int                     viewport_size = viewportCount;
        std::vector<VkViewport> vk_viewport_list(viewport_size);
        for (int i = 0; i < viewport_size; ++i)
        {
            const auto& rhi_viewport_element = pViewports[i];
            auto&       vk_viewport_element  = vk_viewport_list[i];

            vk_viewport_element.x        = rhi_viewport_element.x;
            vk_viewport_element.y        = rhi_viewport_element.y;
            vk_viewport_element.width    = rhi_viewport_element.width;
            vk_viewport_element.height   = rhi_viewport_element.height;
            vk_viewport_element.minDepth = rhi_viewport_element.minDepth;
            vk_viewport_element.maxDepth = rhi_viewport_element.maxDepth;
        };

        return fn_vk_cmd_set_viewport(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource(),
                                      firstViewport,
                                      viewportCount,
                                      vk_viewport_list.data());
    }

    void VulkanRHI::cmdSetScissorPFN(RHICommandBuffer* commandBuffer,
                                     uint32_t          firstScissor,
                                     uint32_t          scissorCount,
                                     const RHIRect2D*  pScissors)
    {
        // rect_2d
        int                   rect_2d_size = scissorCount;
        std::vector<VkRect2D> vk_rect_2d_list(rect_2d_size);
        for (int i = 0; i < rect_2d_size; ++i)
        {
            const auto& rhi_rect_2d_element = pScissors[i];
            auto&       vk_rect_2d_element  = vk_rect_2d_list[i];

            VkOffset2D offset_2d {};
            offset_2d.x = rhi_rect_2d_element.offset.x;
            offset_2d.y = rhi_rect_2d_element.offset.y;

            VkExtent2D extent_2d {};
            extent_2d.width  = rhi_rect_2d_element.extent.width;
            extent_2d.height = rhi_rect_2d_element.extent.height;

            vk_rect_2d_element.offset = VkOffset2D(offset_2d);
            vk_rect_2d_element.extent = VkExtent2D(extent_2d);
        };

        return fn_vk_cmd_set_scissor(static_cast<VulkanCommandBuffer*>(commandBuffer)->getResource(),
                                     firstScissor,
                                     scissorCount,
                                     vk_rect_2d_list.data());
    }

    void VulkanRHI::waitForFences()
    {
        if (fn_vk_wait_for_fences(
                m_device, 1, &m_is_frame_in_flight_fences[m_current_frame_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
        {
            LOG_ERROR("Vulkan failed to synchronize fences!");
        }
    }

    bool VulkanRHI::prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain)
    {
        // acquire next image from swapchain
        VkResult acquire_image_result =
            vkAcquireNextImageKHR(m_device,
                                  m_swapchain,
                                  UINT64_MAX,
                                  m_image_available_for_render_semaphores[m_current_frame_index],
                                  VK_NULL_HANDLE,
                                  &m_current_swapchain_image_index);

        if (VK_ERROR_OUT_OF_DATE_KHR == acquire_image_result)
        { // swapchain is not compatible with surface, should recreate swapchain
            recreateSwapchain();
            passUpdateAfterRecreateSwapchain();
            return RHI_SUCCESS;
        }
        else if (VK_SUCCESS != acquire_image_result && VK_SUBOPTIMAL_KHR != acquire_image_result)
        { // sucess || not completely compatible with surface but swapchain can present at surface
            LOG_ERROR("vkAcquireNextImageKHR failed!");
            return false;
        }

        // begin command buffer
        VkCommandBufferBeginInfo command_buffer_begin_info {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // allow resubmit command buffer even if it is already in waiting list
        command_buffer_begin_info.flags            = 0;
        command_buffer_begin_info.pInheritanceInfo = nullptr;

        if (fn_vk_begin_command_buffer(m_vk_command_buffers[m_current_frame_index], &command_buffer_begin_info) !=
            VK_SUCCESS)
        {
            LOG_ERROR("Vulkan failed to begin recording command buffer!");
            return false;
        }
        return RHI_SUCCESS;
    }

    void VulkanRHI::submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
    {
        // end command buffer
        if (fn_vk_end_command_buffer(m_vk_command_buffers[m_current_frame_index]) != VK_SUCCESS)
        {
            LOG_ERROR("Vulkan EndCommandBuffer failed!");
            return;
        }

        // submit command buffer
        // signal(semaphore[])
        VkSemaphore signal_semaphores[] = {m_image_finished_for_presentation_semaphores[m_current_frame_index]};
        // wait for color attachment output
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo         submit_info   = {};
        submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount     = 1;
        submit_info.pWaitSemaphores        = &m_image_available_for_render_semaphores[m_current_frame_index];
        submit_info.pWaitDstStageMask      = wait_stages;
        submit_info.commandBufferCount     = 1;
        submit_info.pCommandBuffers        = &m_vk_command_buffers[m_current_frame_index];
        submit_info.signalSemaphoreCount   = 1;
        submit_info.pSignalSemaphores      = signal_semaphores;

        if (fn_vk_reset_fences(m_device, 1, &m_is_frame_in_flight_fences[m_current_frame_index]) != VK_SUCCESS)
        { // reset fence state(unsignaled)
            LOG_ERROR("Vulkan ResetFences failed!");
            return;
        }

        // submit info(command buffer) to graphics queue family
        if (vkQueueSubmit(static_cast<VulkanQueue*>(m_graphics_queue)->getResource(),
                          1,
                          &submit_info,
                          // submit finished, allow another render
                          m_is_frame_in_flight_fences[m_current_frame_index]) != VK_SUCCESS)
        {
            LOG_ERROR("Vulkan QueueSubmit failed!");
            return;
        }

        // present swapchain
        VkPresentInfoKHR present_info   = {};
        present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = &m_image_finished_for_presentation_semaphores[m_current_frame_index];
        present_info.swapchainCount     = 1;
        present_info.pSwapchains        = &m_swapchain;
        present_info.pImageIndices      = &m_current_swapchain_image_index;

        VkResult present_result = vkQueuePresentKHR(m_present_queue, &present_info);
        if (VK_ERROR_OUT_OF_DATE_KHR == present_result || VK_SUBOPTIMAL_KHR == present_result)
        { // when the present result isn't best, should recreate swapchain
            recreateSwapchain();
            passUpdateAfterRecreateSwapchain();
        }
        else
        {
            if (VK_SUCCESS != present_result)
            {
                LOG_ERROR("vkQueuePresentKHR failed!");
                return;
            }
        }

        m_current_frame_index = (m_current_frame_index + 1) % m_k_max_frames_in_flight;
    }

    void VulkanRHI::destroyFramebuffer(RHIFramebuffer* framebuffer)
    {
        vkDestroyFramebuffer(m_device, static_cast<VulkanFramebuffer*>(framebuffer)->getResource(), nullptr);
    }

    // map cpu data with the memory data
    bool VulkanRHI::mapMemory(RHIDeviceMemory*  memory,
                              RHIDeviceSize     offset,
                              RHIDeviceSize     size,
                              RHIMemoryMapFlags flags,
                              void**            ppData)
    {
        if (vkMapMemory(m_device,
                        static_cast<VulkanDeviceMemory*>(memory)->getResource(),
                        offset,
                        size,
                        static_cast<VkMemoryMapFlags>(flags),
                        ppData) != VK_SUCCESS)
        {
            LOG_ERROR("vkMapMemory failed!");
            return false;
        }
        return true;
    }

    void VulkanRHI::unmapMemory(RHIDeviceMemory* memory)
    {
        vkUnmapMemory(m_device, static_cast<VulkanDeviceMemory*>(memory)->getResource());
    }
} // namespace Piccolo