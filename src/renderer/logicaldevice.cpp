#include "logicaldevice.h"

LogicalDevice::LogicalDevice(
        VkDevice *device,
        const QueueFamilyInfo & graphicsqueue,
        const QueueFamilyInfo & computequeue,
        VkSwapchainCreateInfoKHR *swapchaincreateinfo
        )
    : logicalDevice(device),
      swapChain(device),
      flag(USING_NONE),
      graphicsQueueFamilyIndex(graphicsqueue.queueFamilyIndex)
{
    if (!device)
        throw std::runtime_error("Null device passed to LogicalDevice!");
    if (graphicsqueue.queueCount && !swapchaincreateinfo)
        throw std::runtime_error("Graphics queues are requested but swapchain info is null!!");

    //Set flag to note what queues are gong to be used...
    if (graphicsqueue.queueCount)
        flag = USING_GRAPHICS_POOL;
    if (computequeue.queueCount)
        flag = (Flag)(flag | USING_COMPUTE_POOL);

    //Obtain handles to all available graphics and compute queues...
    auto getqueues = [&](std::vector <VkQueue> & queues, const QueueFamilyInfo & queueinfo){
        queues.resize(queueinfo.queueCount);
        for (uint32_t i = 0; i < queueinfo.queueCount; i++)
            vkGetDeviceQueue(*device, queueinfo.queueFamilyIndex, i, &queues[i]);
    };
    getqueues(graphicsQueues, graphicsqueue);
    //getqueues(computeQueues, computequeue);

    //Create swapchain and retreive swapchain images...
    if (swapchaincreateinfo)
        swapChain.initializeSwapChain(swapchaincreateinfo);

    //Initialise command pools and retreive buffers...
    if (flag & USING_GRAPHICS_POOL){
        createGraphicsCommandBuffers(&graphicsCommandPool);
    }
}

void LogicalDevice::createGraphicsCommandBuffers(VkCommandPool *commandpool, bool createcommandpool){
    if (createcommandpool){
        //Create a command pool that will reuse command buffers...
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(*logicalDevice, &poolInfo, nullptr, commandpool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool!");
    }

    //Create one primary command buffer for each swapchain image...
    graphicsCommandBuffers.resize(swapChain.getSwapChainFramebuffersCount());
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *commandpool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(graphicsCommandBuffers.size());
    if (vkAllocateCommandBuffers(*logicalDevice, &allocInfo, graphicsCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");

    //Record command buffers...
    for (auto & buffer: graphicsCommandBuffers){
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer!");
    }

    //For each framebuffer, bind a command buffer to it and start renderpass...
    swapChain.startRenderPass(graphicsCommandBuffers);
}
void LogicalDevice::recreateSwapChain(){
    //May need to destroy frame buffers first...
    vkFreeCommandBuffers(
                *logicalDevice,
                graphicsCommandPool,
                static_cast<uint32_t>(graphicsCommandBuffers.size()),
                graphicsCommandBuffers.data()
                );
    swapChain.recreateSwapChain();
    createGraphicsCommandBuffers(&graphicsCommandPool, false);
}

void LogicalDevice::drawFrame(){
    //Start drawing...
    auto result = swapChain.draw(graphicsCommandBuffers, graphicsQueues);

    //Swapchain is out of date or is suboptimal, try once more...
    if (result != VK_SUCCESS){
        recreateSwapChain();
        swapChain.draw(graphicsCommandBuffers, graphicsQueues);
    }
}

void LogicalDevice::cleanup() noexcept{
    swapChain.cleanup();
    if (flag & USING_GRAPHICS_POOL)
        vkDestroyCommandPool(*logicalDevice, graphicsCommandPool, nullptr);
    /*if (flag & USING_COMPUTE_POOL)
        vkDestroyCommandPool(*logicalDevice, computeCommandPool, nullptr);*/
}


