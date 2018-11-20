#include "vulkanvalidationlayers.h"
//#include <QtDebug>
#include "src/utility.h"

//Validation layer functions...
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT,
        VkDebugReportObjectTypeEXT,
        uint64_t,
        size_t,
        int32_t,
        const char*,
        const char* msg,
        void*
        )
{
    //qDebug(msg);
    return VK_FALSE;
}

VkResult VulkanValidationLayers::createDebugReportCallbackEXT(
        VkInstance instance,
        const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugReportCallbackEXT* pCallback
        )
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT")
                );
    if (func){
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }else{
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanValidationLayers::destroyDebugReportCallbackEXT(
        VkInstance instance,
        VkDebugReportCallbackEXT callback,
        const VkAllocationCallbacks* pAllocator
        )
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT")
                );
    if (func)
        func(instance, callback, pAllocator);
}

VulkanValidationLayers::VulkanValidationLayers()
    : vulkanInstance(nullptr)
{
    //
}

void VulkanValidationLayers::initializeValidationLayers(VkInstance *vulkaninstance){
    if (!vulkaninstance)
        throw std::runtime_error("VulkanValidationLayers: null VkInstance pointer passed!");
    vulkanInstance = vulkaninstance;
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;
    if (createDebugReportCallbackEXT(*vulkanInstance, &createInfo, nullptr, &callback) != VK_SUCCESS)
        throw std::runtime_error("Failed to set up debug callback!");
}

VulkanValidationLayers::~VulkanValidationLayers()
{
    if (vulkanInstance)
        destroyDebugReportCallbackEXT(*vulkanInstance, callback, nullptr);
}

