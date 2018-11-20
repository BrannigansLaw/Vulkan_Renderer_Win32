#include "swapchain.h"

SwapChain::SwapChain(VkDevice *device)
    : logicalDevice(device),
      swapChain(nullptr),
      graphicsPipeline(device),
      initialised(false)
{
    swapChainCreateInfo = {};
    if (!device)
        throw std::runtime_error("Null device passed to SwapChain!");
}

void SwapChain::startRenderPass(std::vector<VkCommandBuffer> &graphicsCommandBuffers) noexcept{
    for (auto i = 0U; i < graphicsCommandBuffers.size(); i++)
        graphicsPipeline.startRenderPass(swapChainFramebuffers.at(i), swapChainExtent, graphicsCommandBuffers[i]);
}

void SwapChain::initializeSwapChain(VkSwapchainCreateInfoKHR *swapchaincreateinfo){
    if (!swapchaincreateinfo)
        throw std::runtime_error("Swap chain create info is null!");

    //Reuse old swapchain if one already exists...
    swapchaincreateinfo->oldSwapchain = swapChain;

    //Copy the create info...THIS MAY BREAK WITH SWAPCHAIN RECREATION!!!
    swapChainCreateInfo = *swapchaincreateinfo;

    //Create swapchain...
    if (vkCreateSwapchainKHR(*logicalDevice, swapchaincreateinfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");

    //Get swapchain images...
    auto imageCount = 0U;
    vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &imageCount, swapChainImages.data());

    //Store image format, extent and set initialised (for use in the destructor)...
    swapChainImageFormat = swapchaincreateinfo->imageFormat;
    swapChainExtent = swapchaincreateinfo->imageExtent;
    initialised = true;

    //Create swapchain image views...
    swapChainImageViews.resize(swapChainImages.size());
    for (auto i = 0U; i < swapChainImages.size(); i++){
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(*logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }

    //Create renderpass now since we'll need it to create framebuffers...
    graphicsPipeline.createRenderpass(swapChainImageFormat);

    //Create framebufffers for all swapchain image views...
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (auto i = 0U; i < swapChainImageViews.size(); i++){
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = graphicsPipeline.getRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(*logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }

    //Set up the graphics pipeline...
    graphicsPipeline.initializeFixedFunctions(swapChainExtent);
}

void SwapChain::recreateSwapChain(){
    cleanup(false);
    graphicsPipeline.cleanup(false);
    initializeSwapChain(&swapChainCreateInfo);
}

void SwapChain::cleanup(bool destroyswapchain) noexcept{
    if (initialised){
        //Sometimes we want to reuse the swapchain...
        if (destroyswapchain)
            vkDestroySwapchainKHR(*logicalDevice, swapChain, nullptr);

        //Nothing reusable here...
        for (auto view : swapChainImageViews)
            vkDestroyImageView(*logicalDevice, view, nullptr);
        for (auto buffer : swapChainFramebuffers)
            vkDestroyFramebuffer(*logicalDevice, buffer, nullptr);
        initialised = false;
    }
}

VkFramebuffer SwapChain::getSwapChainFramebuffer(size_t index) const{
    if (index >= swapChainFramebuffers.size())
        throw  std::runtime_error("Invalid swapChainFramebuffers index!");
    return swapChainFramebuffers[index];
}

VkResult SwapChain::draw(std::vector<VkCommandBuffer> &graphicsCommandBuffers, std::vector<VkQueue> &graphicsqueues){
    //Create semaphores for synchronizing swap chain events (get swapchain image, execute commands on it, return it)...
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(*logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(*logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS){
        throw std::runtime_error("Failed to create semaphores!");
    }

    //Aquire image from swapchain...
    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(
                *logicalDevice,
                swapChain,
                (std::numeric_limits<uint64_t>::max)(),
                imageAvailableSemaphore,
                nullptr,
                &imageIndex
                );

    if (result == VK_SUCCESS){
        //TO DO: Need separate semaphores? command buffers? fix this...
        auto numqueues = static_cast<uint32_t>(graphicsqueues.size());
        for (auto & graphicsqueue : graphicsqueues){
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &graphicsCommandBuffers[imageIndex];
            VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;
            if (vkQueueSubmit(graphicsqueue, numqueues, &submitInfo, nullptr) != VK_SUCCESS)
                throw std::runtime_error("Failed to submit draw command buffer!");

            //Get the resulting image and present it...
            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = {swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;
            presentInfo.pResults = nullptr;
            if (vkQueuePresentKHR(graphicsqueue, &presentInfo) != VK_SUCCESS)
                throw std::runtime_error("Presentation failed!");

            //Do this when using validation layers to prevent memory leaks...
            vkQueueWaitIdle(graphicsqueue);
        }

        //Destroy semaphores...
        vkDestroySemaphore(*logicalDevice, renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(*logicalDevice, imageAvailableSemaphore, nullptr);
    }else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
        //Just return the result and the calling code will recreate the swapchain...
    }else{
        throw std::runtime_error("Failed to aquire image from swapchain!");
    }

    return result;
}

/*GraphicsPipeline SwapChain::getGraphicPipeline() const{
    return graphicsPipeline;
}*/

size_t SwapChain::getSwapChainFramebuffersCount() const noexcept{
    return swapChainFramebuffers.size();
}

