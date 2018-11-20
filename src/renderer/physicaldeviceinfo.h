#ifndef PHYSICALDEVICEINFO_H
#define PHYSICALDEVICEINFO_H

#include <vulkan.h>
#include <vector>
#include <string>

#include "logicaldevice.h"
#include "src/utility.h"

class PhysicalDeviceInfo final
{
    friend class VulkanRenderer;
public:
    PhysicalDeviceInfo(VkPhysicalDevice * device, VkInstance *instance);
public:
    PhysicalDeviceInfo() = default;
    ~PhysicalDeviceInfo() = default;
    PhysicalDeviceInfo(const PhysicalDeviceInfo & other) = default;
    PhysicalDeviceInfo & operator=(const PhysicalDeviceInfo & other) = default;
private:
    void addLogicalDevice(
            VkDeviceCreateInfo *devicecreateinfo,
            VkSurfaceKHR &surface,
            uint32_t graphicsqueuecount,
            uint32_t computequeuecount,
            VkSwapchainCreateInfoKHR * swapchaincreateinfo
            );
    void draw(uint32_t logicaldeviceindex);
    void recreateSwapChain(uint32_t logicaldeviceindex) noexcept;
    [[nodiscard]] constexpr uint64_t getDeviceScore() const noexcept{ return deviceScore; }
    [[nodiscard]] QueueFamilyInfo getQueueFamilyIndex(VkQueueFlags requiredflags, int indextoignore = -1) const;
    [[nodiscard]] std::string checkFeatures(const VkPhysicalDeviceFeatures *requiredfeatures) const;
    [[nodiscard]] std::string checkQueueProperties(VkQueueFlags requiredflags) const;
    [[nodiscard]] uint32_t getLogicalDeviceCount() const noexcept;
    void cleanup() noexcept;
private:
    VkInstance *vulkanInstance;
    VkPhysicalDevice *physicalDevice;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    uint64_t deviceScore;
    std::vector <VkDevice> logicalDevices;
    std::vector <LogicalDevice> logicalDeviceInfos;
    std::vector <VkQueueFamilyProperties> deviceQueueFamilyProperties;
    std::vector <VkLayerProperties> layerProperties;
    std::vector <VkExtensionProperties> extensionProperties;
};

#endif // PHYSICALDEVICEINFO_H
