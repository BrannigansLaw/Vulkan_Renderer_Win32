#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vulkan.h>
#include <vector>
#include <string>

#include "graphicspipeline.h"

class SwapChain final
{
    friend class LogicalDevice;
public:
    SwapChain(VkDevice *device);
public:
    SwapChain() = default;
    ~SwapChain() = default;
    SwapChain(const SwapChain & other) = default;
    SwapChain & operator=(const SwapChain & other) = default;
private:
    void startRenderPass(std::vector<VkCommandBuffer> &graphicsCommandBuffers) noexcept;
    void initializeSwapChain(VkSwapchainCreateInfoKHR *swapchaincreateinfo);
    void recreateSwapChain();
    void cleanup(bool destroyswapchain = true) noexcept;
    [[nodiscard]] VkFramebuffer getSwapChainFramebuffer(size_t index) const;
    VkResult draw(std::vector<VkCommandBuffer> &graphicsCommandBuffers, std::vector<VkQueue> &graphicsqueues);
    //GraphicsPipeline getGraphicPipeline() const;
    [[nodiscard]] size_t getSwapChainFramebuffersCount() const noexcept;
private:
    VkDevice *logicalDevice;
    VkSwapchainCreateInfoKHR swapChainCreateInfo;
    VkSwapchainKHR swapChain;
    std::vector <VkImage> swapChainImages;
    std::vector <VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    GraphicsPipeline graphicsPipeline;
    bool initialised;
};

#endif // SWAPCHAIN_H
