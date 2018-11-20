#include "vulkanrenderer.h"
#include <algorithm>

/*!
        \class VulkanRenderer
        \brief The VulkanRenderer class is the renderer.

        \reentrant

        VulkanRenderer is the surface class that acts as an abstraction layer for all
        that Vulkan does under the hood to render 3D scenes. Everything that's possible
        with this API is done through this class. It manages the Vulkan instance, the
        available physical devices, any created logical devices, the Windows surface
        used for presentation and more through it's components.

        It is via this class you will pass in all the data Vulkan needs to render such
        as required extensions, vertex data, vertex index buffers, image samplers and
        also data that tells Vulkan how you want to render such as what shader stages
        are required, what push constants to use, fixed function settings, etc...
*/

bool VulkanRenderer::windowResized = false;

VulkanRenderer::VulkanRenderer(
        WindowCreateInfo &windowcreateinfo,
        const std::vector<const char *> &enablelayers,
        const std::vector<const char *> &enableextensions
        )
    : render(true),
      currentPhysicalDeviceIndex(0),
      currentLogicalDeviceIndex(0),
      indexOfStrongestDevice(0),
      windowsClass(windowcreateinfo)
{
    //Setup application info...
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Artic Engine";
    appInfo.pEngineName = "Artic Engine";
    appInfo.pNext = nullptr;
    appInfo.applicationVersion = 1;
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    //Setup vulkan instance info...
    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    //Ensure requested layers are available...
    uint32_t numlayers = 0;
    vkEnumerateInstanceLayerProperties(&numlayers, nullptr);
    if (numlayers){
        layerProperties.resize(numlayers);
        if (vkEnumerateInstanceLayerProperties(&numlayers, &layerProperties[0]) != VK_SUCCESS)
            throw std::runtime_error("Failed to enumerate layers!");
    }
    if (!enablelayers.size()){
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }else{
        for (auto requiredlayer: enablelayers){
            auto found = false;
            for (auto layer: layerProperties){
                if (!strcmp(layer.layerName, requiredlayer)){
                    found = true;
                    break;
                }
            }
            if (!found)
                throw std::runtime_error(std::string("Required layer \"")+requiredlayer+std::string("\" is missing!"));
        }
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enablelayers.size());
        instanceCreateInfo.ppEnabledLayerNames = enablelayers.data();
    }

    //Ensure requested extensions are available...
    uint32_t numextensions = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &numextensions, nullptr);
    if (numextensions){
        extensionProperties.resize(numextensions);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &numextensions, &extensionProperties[0]) != VK_SUCCESS)
            throw std::runtime_error("Failed to enumerate extensions!");
    }
    if (!enableextensions.size()){
        instanceCreateInfo.enabledExtensionCount = 0;
        instanceCreateInfo.ppEnabledExtensionNames = nullptr;
    }else{
        for (auto requiredextension: enableextensions){
            auto found = false;
            for (auto extension: extensionProperties){
                if (!strcmp(extension.extensionName, requiredextension)){
                    found = true;
                    break;
                }
            }
            if (!found)
                throw std::runtime_error(std::string("Required extension \"")+requiredextension+std::string("\" is missing!"));
        }
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enableextensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = enableextensions.data();
    }

    //Create vulkan instance...
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan Instance!");

    //Set up validation layers...
    validationLayers.initializeValidationLayers(&vulkanInstance);

    //Create physical devices...
    uint32_t physicalDeviceCount;
    if (vkEnumeratePhysicalDevices(vulkanInstance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
        throw std::runtime_error("Failed to enumerate physical devices!");
    if (!physicalDeviceCount)
        throw std::runtime_error("No physical devices detected!");
        physicalDevices.resize(physicalDeviceCount);
    if (vkEnumeratePhysicalDevices(vulkanInstance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to enumerate physical devices!");

    //Get device info...
    auto score = 0ULL;
    for (auto i = 0U; i < physicalDeviceCount; i++){
        physicalDeviceInfos.push_back(PhysicalDeviceInfo(&physicalDevices[i], &vulkanInstance));
        if (physicalDeviceInfos.at(i).getDeviceScore() > score)
            indexOfStrongestDevice = i;
    }

    //Set up surface info...
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = windowsClass.getWindows32Handle();
    surfaceCreateInfo.hwnd = windowsClass.getWindow();

    //Create Windows surface...
    if (vkCreateWin32SurfaceKHR(vulkanInstance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

void VulkanRenderer::setWindowResized(bool resized) noexcept{
    windowResized = resized;
}

bool VulkanRenderer::wasWindowResized() const noexcept{
    return windowResized;
}

bool VulkanRenderer::keepRendering() const noexcept{
    return render;
}

void VulkanRenderer::drawFrame(){
    //Start drawing frames...
    physicalDeviceInfos[currentPhysicalDeviceIndex].draw(currentLogicalDeviceIndex);

    //Process messages...
    if (!windowsClass.processMessage()){
        render = false;
    }

    //If the window size changes we need to recreate the swapchain...
    if (windowResized)
        recreateSwapChain();
}

void VulkanRenderer::recreateSwapChain(){
    physicalDeviceInfos[currentPhysicalDeviceIndex].recreateSwapChain(currentLogicalDeviceIndex);
    //Window resize handled, revert state...
    windowResized = false;
}

void VulkanRenderer::initializeRenderLoop(int physicaldeviceindex, uint32_t logicaldeviceindex){
    //Ensure valid device indices and set indices...
    if (physicaldeviceindex == -1){
        currentPhysicalDeviceIndex = indexOfStrongestDevice;
    }else if (static_cast<uint32_t>(physicaldeviceindex) < physicalDeviceInfos.size()){
        currentPhysicalDeviceIndex = static_cast<uint32_t>(physicaldeviceindex);
    }else{
        throw std::runtime_error("Invalid physical device index!");
    }
    if (logicaldeviceindex >= physicalDeviceInfos.at(currentPhysicalDeviceIndex).getLogicalDeviceCount())
        throw std::runtime_error("Invalid logical device index!");
    currentLogicalDeviceIndex = logicaldeviceindex;

    //Show window...
    windowsClass.showWindow();
}

/*void VulkanRenderer::addLogicalDevice(VkDeviceCreateInfo * devicecreateinfo, uint32_t graphicsqueuecount, uint32_t computequeuecount, int physicaldeviceindex){
    try{
        addDevice(devicecreateinfo, physicaldeviceindex, graphicsqueuecount, computequeuecount);
    }catch (std::runtime_error error){
        throw error;
    }
}*/

void VulkanRenderer::addLogicalDevice(
        const std::array<QueueInfo, MAX_NUM_QUEUE_TYPES_ALLOWED> &queuetypes,
        const VkPhysicalDeviceFeatures & features,
        VkSwapchainCreateInfoKHR *swapchaincreateinfo,
        const std::vector<const char *> &enablelayers,
        const std::vector<const char *> &enableextensions,
        int deviceindex
        )
{
    //Make sure physicaldeviceindex is valid...
    if (deviceindex < 0 || static_cast<uint32_t>(deviceindex) >= physicalDevices.size())
        throw std::runtime_error("Invalid physicaldeviceindex device!");

    //Make sure queue flags is not empty...
    if (queuetypes.empty())
        throw std::runtime_error("No queue properties specified for the logical device!");

    //Make sure graphics and compute queues are requested separately...
    if ((queuetypes.front().flag & VK_QUEUE_GRAPHICS_BIT) == (queuetypes.back().flag & VK_QUEUE_GRAPHICS_BIT))
        throw std::runtime_error("Duplicate VK_QUEUE_GRAPHICS_BIT queue flags were passed into addLogicalDevice()!");
    if ((queuetypes.front().flag & VK_QUEUE_COMPUTE_BIT) == (queuetypes.back().flag & VK_QUEUE_COMPUTE_BIT))
        throw std::runtime_error("Duplicate VK_QUEUE_COMPUTE_BIT queue flags were passed into addLogicalDevice()!");

    //Set up the requirements for the logical device...
    VkDeviceCreateInfo devicecreateinfo;
    devicecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devicecreateinfo.pNext = nullptr;
    devicecreateinfo.flags = 0;
    if (enablelayers.empty()){
        devicecreateinfo.enabledLayerCount = 0;
        devicecreateinfo.ppEnabledLayerNames = nullptr;
    }else{
        devicecreateinfo.enabledLayerCount = static_cast<uint32_t>(enablelayers.size());
        devicecreateinfo.ppEnabledLayerNames = enablelayers.data();
    }
    if (enableextensions.empty()){
        devicecreateinfo.enabledExtensionCount = 0;
        devicecreateinfo.ppEnabledExtensionNames = nullptr;
    }else{
        devicecreateinfo.enabledExtensionCount = static_cast<uint32_t>(enableextensions.size());
        devicecreateinfo.ppEnabledExtensionNames = enableextensions.data();
    }
    devicecreateinfo.pEnabledFeatures = &features;
    devicecreateinfo.queueCreateInfoCount = static_cast<uint32_t>(queuetypes.size());

    //Set up the number and types of queues required for the logical device...
    std::vector <VkDeviceQueueCreateInfo> queueinfos(queuetypes.size());
    uint32_t index = 0;
    for (auto queueinfo : queuetypes){
        auto queuefamilyinfo = physicalDeviceInfos[static_cast<uint32_t>(deviceindex)].getQueueFamilyIndex(queueinfo.flag);
        if (!queuefamilyinfo.queueCount)
            throw std::runtime_error("Queue family unsupported!");
        VkDeviceQueueCreateInfo queuecreateinfo;
        queuecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queuecreateinfo.pNext = nullptr;
        queuecreateinfo.flags = 0;
        queuecreateinfo.queueCount = static_cast<uint32_t>(queueinfo.prioritys.size());
        queuecreateinfo.queueFamilyIndex = queuefamilyinfo.queueFamilyIndex;
        queuecreateinfo.pQueuePriorities = queueinfo.prioritys.data();
        memcpy(&queueinfos[index++], &queuecreateinfo, sizeof(VkDeviceQueueCreateInfo));
    }
    devicecreateinfo.pQueueCreateInfos = queueinfos.data();

    //If a graphics queue is requested, set up the swapchain...
    VkSwapchainCreateInfoKHR swapchaininfo;
    auto found = false;
    for (auto queue : queuetypes){
        if (queue.flag & VK_QUEUE_GRAPHICS_BIT || queue.flag & VK_QUEUE_GRAPHICS_BIT)
            found = true;
    }
    if (found){
        if (!swapchaincreateinfo){
            //Get the physical device's surface capabilities...
            VkSurfaceCapabilitiesKHR capabilities;
            if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[static_cast<uint32_t>(deviceindex)], surface, &capabilities) != VK_SUCCESS)
                throw std::runtime_error("Could not get information about the physical device's surface capabilities!");

            //Get the physical device's supported surface formats (pixel format, color space)...
            std::vector<VkSurfaceFormatKHR> formats;
            uint32_t formatcount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[static_cast<uint32_t>(deviceindex)], surface, &formatcount, nullptr);
            if (!formatcount)
                throw std::runtime_error("Could not get information about the physical device's supported formats!");
            formats.resize(formatcount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[static_cast<uint32_t>(deviceindex)], surface, &formatcount, formats.data());

            //Get the physical device's supported presentation modes...
            std::vector<VkPresentModeKHR> presentmodes;
            uint32_t presentmodecount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[static_cast<uint32_t>(deviceindex)], surface, &presentmodecount, nullptr);
            if (!presentmodecount)
                throw std::runtime_error("Could not get information about the physical device's supported presentation modes!");
            presentmodes.resize(presentmodecount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[static_cast<uint32_t>(deviceindex)], surface, &presentmodecount, presentmodes.data());

            //Get minimum image count..
            uint32_t imagecount = capabilities.minImageCount + 1;
            if (capabilities.maxImageCount > 0 && imagecount > capabilities.maxImageCount)
                imagecount = capabilities.maxImageCount;

            //Create swapchain create info based on what the physical device supports...
            auto surfaceformat = chooseSwapSurfaceFormat(formats);
            swapchaininfo = {
                swapchaininfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                swapchaininfo.pNext = nullptr,
                swapchaininfo.flags = 0,
                swapchaininfo.surface = surface,
                swapchaininfo.minImageCount = imagecount,
                swapchaininfo.imageFormat = surfaceformat.format,
                swapchaininfo.imageColorSpace = surfaceformat.colorSpace,
                /*swapchaininfo.imageExtent = */chooseSwapExtent(capabilities),
                swapchaininfo.imageArrayLayers = 1,
                swapchaininfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                swapchaininfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                swapchaininfo.queueFamilyIndexCount = 0,
                swapchaininfo.pQueueFamilyIndices = nullptr,
                swapchaininfo.preTransform = capabilities.currentTransform,
                swapchaininfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                swapchaininfo.presentMode = chooseSwapPresentMode(presentmodes),
                swapchaininfo.clipped = VK_TRUE,
                swapchaininfo.oldSwapchain = nullptr
            };
            swapchaincreateinfo = &swapchaininfo;
        }else{
            //Ensure it's requested attributes are supported by the selected physical device...
            VkSurfaceCapabilitiesKHR surfacecapabilities = {};
            if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[static_cast<uint32_t>(deviceindex)], surface, &surfacecapabilities) != VK_SUCCESS)
                throw std::runtime_error("Could not get information about the physical device's surface capabilities!");
            if (swapchaincreateinfo->minImageCount < surfacecapabilities.minImageCount || swapchaincreateinfo->minImageCount > surfacecapabilities.maxImageCount)
                throw std::runtime_error("Surface does not support the requested number of swapchain images!");
            if (swapchaincreateinfo->imageExtent.width > surfacecapabilities.maxImageExtent.width || swapchaincreateinfo->imageExtent.width < surfacecapabilities.minImageExtent.width)
                throw std::runtime_error("Surface does not support the requested swapchain image width!");
            if (swapchaincreateinfo->imageExtent.height > surfacecapabilities.maxImageExtent.height || swapchaincreateinfo->imageExtent.height < surfacecapabilities.minImageExtent.height)
                throw std::runtime_error("Surface does not support the requested swapchain image height!");
            if (swapchaincreateinfo->imageArrayLayers > surfacecapabilities.maxImageArrayLayers)
                throw std::runtime_error("Surface does not support the requested number of image array layers!");
            /*if (swapchaininfo->preTransform != (swapchaininfo->preTransform & surfacecapabilities.supportedTransforms))
                throw std::runtime_error("Surface does not support the requested transforms!");
            if (swapchaininfo->currentTransform != (swapchaininfo->currentTransform & surfacecapabilities.supportedTransforms))
                throw std::runtime_error("Surface does not support the requested transforms!");
            if (swapchaininfo->compositeAlpha != (swapchaininfo->compositeAlpha & surfacecapabilities.supportedCompositeAlpha))
                throw std::runtime_error("Surface does not support the requested composite alpha!");*/
            if (swapchaincreateinfo->imageUsage != (swapchaincreateinfo->imageUsage & surfacecapabilities.supportedUsageFlags))
                throw std::runtime_error("Surface does not support the requested image usage flags!");
        }
    }

    //Determine requested graphics and compute queue counts and add device...
    try{
        uint32_t graphicsqueuecount;
        uint32_t computequeuecount;
        (queuetypes.front().flag & VK_QUEUE_GRAPHICS_BIT) ? graphicsqueuecount = static_cast<uint32_t>(queuetypes.front().prioritys.size()): graphicsqueuecount = static_cast<uint32_t>(queuetypes.back().prioritys.size());
        (queuetypes.back().flag & VK_QUEUE_COMPUTE_BIT) ? computequeuecount = static_cast<uint32_t>(queuetypes.back().prioritys.size()): computequeuecount = static_cast<uint32_t>(queuetypes.front().prioritys.size());
        addDevice(&devicecreateinfo, deviceindex, graphicsqueuecount, computequeuecount, swapchaincreateinfo);

        //Start drawing if desired...
        if (graphicsqueuecount > 0)
            initializeRenderLoop(deviceindex, physicalDeviceInfos.at(static_cast<uint32_t>(deviceindex)).getLogicalDeviceCount() - 1);
    }catch (std::runtime_error error){
        throw error;
    }

}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableformats) const noexcept{
    if (availableformats.size() == 1 && availableformats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    for (const auto& availableformat : availableformats) {
        if (availableformat.format == VK_FORMAT_B8G8R8A8_UNORM && availableformat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableformat;
    }
    return availableformats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> & availablepresentmodes) const noexcept{
    VkPresentModeKHR bestmode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablepresentmode : availablepresentmodes){
        if (availablepresentmode == VK_PRESENT_MODE_MAILBOX_KHR){
            return availablepresentmode;
        }else if (availablepresentmode == VK_PRESENT_MODE_IMMEDIATE_KHR){
            bestmode = availablepresentmode;
        }
    }
    return bestmode;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const noexcept{
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()){
        return capabilities.currentExtent;
    }else{
        VkExtent2D actualExtent = VK_EXTENT_1080_P;
        actualExtent.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

void VulkanRenderer::addDevice(VkDeviceCreateInfo * devicecreateinfo, int deviceindex, uint32_t graphicsqueuecount, uint32_t computequeuecount, VkSwapchainCreateInfoKHR *swapchaincreateinfo){
    if (graphicsqueuecount && !swapchaincreateinfo)
        throw std::runtime_error("Graphics queues are requested but swapchain info is null!!");
    auto index = -1;
    if (deviceindex >= 0){
        if (static_cast<uint32_t>(deviceindex) < sizeof(physicalDevices)){
            index = deviceindex;
        }else{
            throw std::runtime_error("Physical device index out of range!");
        }
    }else{
        //Find the most powerful device and create the logical device on it...
        uint64_t score = 0;
        for (uint32_t i = 0; i < sizeof(physicalDevices); i++){
            uint64_t temp = physicalDeviceInfos[i].getDeviceScore();
            if (temp > score){
                score = temp;
                index = static_cast<int>(i);
            }
        }
    }
    //Create the device...
    if (index >= 0){
        try{
            physicalDeviceInfos[static_cast<uint32_t>(index)].addLogicalDevice(devicecreateinfo, surface, graphicsqueuecount, computequeuecount, swapchaincreateinfo);
        }catch (std::runtime_error error){
            throw error;
        }
    }else{
        throw std::runtime_error("This Vulkan instance has no physical devices!");
    }
}

VulkanRenderer::~VulkanRenderer()
{
    for (auto device : physicalDeviceInfos)
        device.cleanup();
    vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
    vkDestroyInstance(vulkanInstance, nullptr);
}
