#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "src/utility.h"
#include "physicaldeviceinfo.h"
#include "src/ui/win32.h"
#include "vulkanvalidationlayers.h"

class VulkanRenderer final
{
public:
    VulkanRenderer(WindowCreateInfo &windowcreateinfo,
            const std::vector<const char *> &enablelayers = std::vector<const char *> {
                "VK_LAYER_LUNARG_standard_validation"
                }
            ,
            const std::vector<const char *> &enableextensions = std::vector<const char *> {
                VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                "VK_KHR_win32_surface",
                "VK_KHR_surface"
                }
            );
    ~VulkanRenderer();
public:
    VulkanRenderer(const VulkanRenderer & other) = delete;
    VulkanRenderer & operator=(const VulkanRenderer & other) = delete;
    VulkanRenderer(const VulkanRenderer && other) = delete;
    VulkanRenderer & operator=(const VulkanRenderer && other) = delete;
public:
    static void setWindowResized(bool resized) noexcept;
    [[nodiscard]] bool wasWindowResized() const noexcept;
    [[nodiscard]] bool keepRendering() const noexcept;
    void drawFrame();
    void addLogicalDevice(
            const std::array<QueueInfo, MAX_NUM_QUEUE_TYPES_ALLOWED> & queuetypes,
            const VkPhysicalDeviceFeatures & features,
            VkSwapchainCreateInfoKHR * swapchaincreateinfo = nullptr,
            const std::vector<const char *> &enablelayers = std::vector<const char *> {/*"VK_LAYER_LUNARG_standard_validation"*/},
            const std::vector<const char *> &enableextensions = std::vector<const char *> {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
            int deviceindex = 0
            );
    //void addLogicalDevice(VkDeviceCreateInfo *devicecreateinfo, uint32_t graphicsqueuecount, uint32_t computequeuecount, int physicaldeviceindex = -1);
private:
    void recreateSwapChain();
    void initializeRenderLoop(int physicaldeviceindex = -1, uint32_t logicaldeviceindex = 0);
    [[nodiscard]] VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> & availableformats) const noexcept;
    [[nodiscard]] VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> & availablepresentmodes) const noexcept;
    [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const noexcept;
    void addDevice(
            VkDeviceCreateInfo *devicecreateinfo,
            int physicaldeviceindex,
            uint32_t graphicsqueuecount,
            uint32_t computequeuecount,
            VkSwapchainCreateInfoKHR * swapchaincreateinfo
            );
private:
    static bool windowResized;
    bool render;
    uint32_t currentPhysicalDeviceIndex;
    uint32_t currentLogicalDeviceIndex;
    VkInstance vulkanInstance;
    uint32_t indexOfStrongestDevice;
    std::vector <VkPhysicalDevice> physicalDevices;
    std::vector <PhysicalDeviceInfo> physicalDeviceInfos;
    VkSurfaceKHR surface;
    Win32 windowsClass;
    VulkanValidationLayers validationLayers;
    std::vector <VkLayerProperties> layerProperties;
    std::vector <VkExtensionProperties> extensionProperties;
};

#endif // VULKANRENDERER_H
