#ifndef VULKANVALIDATIONLAYERS_H
#define VULKANVALIDATIONLAYERS_H

#include <vulkan.h>

static PFN_vkDebugReportCallbackEXT debugCallback();

class VulkanValidationLayers
{
public:
    VulkanValidationLayers();
    ~VulkanValidationLayers();
public:
    void initializeValidationLayers(VkInstance *vulkaninstance);
private:
    VkResult createDebugReportCallbackEXT(
            VkInstance instance,
            const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugReportCallbackEXT* pCallback
            );
    void destroyDebugReportCallbackEXT(
            VkInstance instance,
            VkDebugReportCallbackEXT callback,
            const VkAllocationCallbacks* pAllocator
            );
private:
    VkInstance *vulkanInstance;
    VkDebugReportCallbackEXT callback;
};

#endif // VULKANVALIDATIONLAYERS_H
