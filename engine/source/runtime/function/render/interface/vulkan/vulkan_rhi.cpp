#include "function/render/render_type.h"
#include "vulkan/vulkan_core.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <utility>

#include "core/base/macro.h"
#include "function/render/interface/vulkan/vulkan_rhi.h"
#include "function/render/interface/vulkan/vulkan_rhi_resource.h"
#include "function/render/interface/vulkan/vulkan_util.h"

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
    }

    // 初始化Vulkan实例：设置应用名称、版本、拓展模块等
    void VulkanRHI::createInstance()
    {
        if (m_enable_validation_layers && !checkValidationLayerSupport())
            LOG_ERROR("validation layers requested, but not available!");

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
            LOG_ERROR("Vk create instance failed!");
    }

    // 启动验证层从而在debug版本中发现可能存在的错误
    void VulkanRHI::initializeDebugMessenger()
    {
        if (m_enable_validation_layers)
        {
            VkDebugUtilsMessengerCreateInfoEXT create_info;
            populateDebugMessengerCreateInfo(create_info);
            if (createDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
                LOG_ERROR("Failed to set debug messenger!");
        }
    }

    // 链接之前的glfw，使vulkan与当前运行平台窗口系统兼容
    void VulkanRHI::createWindowSurface()
    {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
            LOG_ERROR("glfwCreateWindowSurface failed!");
    }

    // 选择物理设备，并进行一些兼容性检查（比如你的rtx2060）
    void VulkanRHI::initializePhysicalDevice()
    {
        uint32_t physical_device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);
        if (physical_device_count == 0)
            LOG_ERROR("Enumerate physical devices failed!");
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
                LOG_ERROR("Failed to find suitable physical device!");
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
            LOG_ERROR("Failed create logic device!");

        // initialize queues of this device
        VkQueue vk_graphics_queue;
        vkGetDeviceQueue(m_device, m_queue_indices.graphics_family.value(), 0, &vk_graphics_queue);
        m_graphics_queue = new VulkanQueue();
        static_cast<VulkanQueue*>(m_graphics_queue)->setResource(vk_graphics_queue); // 绑定资源

        vkGetDeviceQueue(m_device, m_queue_indices.present_family.value(), 0, &m_present_queue);
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
            LOG_ERROR("Vk create swapchain khr failed!");

        // 虽然已经显式的指明了image数量，但vulkan实际上可能创造更多，所以需要查询一次
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
        m_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data());

        // 保存交换链中的图像所选择的格式与范围到成员变量
        m_swapchain_image_format = static_cast<RHIFormat>(chosen_surface_format.format);
        m_swapchain_extent       = {chosen_extent.height, chosen_extent.width};
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
    { // TODO: implement
        // int pipeline_shader_stage_create_info_size = pCreateInfo->stageCount;
        // //
        // 要使用着色器，我们需要通过VkPipelineShaderStageCreateInfo结构体把它们分配到图形渲染管线上的某一阶段，作为管线创建过程的一部分
        // std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_info_list(
        //     pipeline_shader_stage_create_info_size);

        return RHI_SUCCESS;
    }

    bool VulkanRHI::createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo,
                                         RHIPipelineLayout*&                pPipelineLayout)
    { // TODO: implement
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
} // namespace Piccolo