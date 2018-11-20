#ifndef LOGICALDEVICE_H
#define LOGICALDEVICE_H

#include "swapchain.h"
#include "src/utility.h"

class LogicalDevice final
{
    friend class PhysicalDeviceInfo;
private:
    enum Flag {
        USING_GRAPHICS_POOL = 0b00000001,
        USING_COMPUTE_POOL = 0b00000010,
        USING_NONE = 0b00000000
    };
public:
    LogicalDevice(
            VkDevice *device,
            const QueueFamilyInfo & graphicsqueue,
            const QueueFamilyInfo & computequeue,
            VkSwapchainCreateInfoKHR *swapchaincreateinfo
            );
public:
    LogicalDevice() = default;
    ~LogicalDevice() = default;
    LogicalDevice(const LogicalDevice & other) = default;
    LogicalDevice & operator=(const LogicalDevice & other) = default;
private:
    void createGraphicsCommandBuffers(VkCommandPool *commandpool,
            bool createcommandpool = true
            );
    void recreateSwapChain();
    void drawFrame();
    void cleanup() noexcept;
private:
    VkDevice *logicalDevice;
    std::vector <VkQueue> graphicsQueues;
    VkCommandPool graphicsCommandPool;
    std::vector <VkCommandBuffer> graphicsCommandBuffers;
    SwapChain swapChain;
    Flag flag;
    uint32_t graphicsQueueFamilyIndex;
    /*std::vector <VkQueue> computeQueues;
    VkCommandPool computeCommandPool;
    std::vector <VkCommandBuffer> computeCommandBuffers;*/
};

#endif // LOGICALDEVICE_H
