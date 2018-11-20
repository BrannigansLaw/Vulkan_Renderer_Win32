#include "physicaldeviceinfo.h"

/*!
        \class PhysicalDeviceInfo
        \brief The PhysicalDeviceInfo class is a wrapper class for a physical device, the device's features,
        it's score, along with handles to it's logical devices.

        \reentrant

        PhysicalDeviceInfo manages a physical device and stores it's supported features (such as geometry and tessellation shaders),
        memory properties and device properties (such as vendor strings). It also crudely determines a score for the device by querying
        it's limits which can be used to pick the strongest physical device to render on.
*/

PhysicalDeviceInfo::PhysicalDeviceInfo(VkPhysicalDevice * device, VkInstance *instance)
    : vulkanInstance(instance),
      physicalDevice(device),
      deviceScore(0)
{
    if (!instance)
        throw std::runtime_error("A null Vulkan instance has been passed to a PhysicalDeviceInfo!");
    if (!device)
        throw std::runtime_error("A null physical device has been passed to a PhysicalDeviceInfo!");

    //Get all device features and properties...
    vkGetPhysicalDeviceProperties(*physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(*physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &deviceMemoryProperties);
    uint32_t queuefamilypropertycount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &queuefamilypropertycount, nullptr);
    deviceQueueFamilyProperties.resize(queuefamilypropertycount);
    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &queuefamilypropertycount, &deviceQueueFamilyProperties[0]);

    //Evaluate device score by tallying limits...
    deviceScore += deviceProperties.limits.maxImageDimension1D;
    deviceScore += deviceProperties.limits.maxImageDimension2D;
    deviceScore += deviceProperties.limits.maxImageDimension3D;
    deviceScore += deviceProperties.limits.maxImageDimensionCube;
    deviceScore += deviceProperties.limits.maxImageArrayLayers;
    deviceScore += deviceProperties.limits.maxTexelBufferElements;
    deviceScore += deviceProperties.limits.maxUniformBufferRange;
    deviceScore += deviceProperties.limits.maxStorageBufferRange;
    deviceScore += deviceProperties.limits.maxPushConstantsSize;
    deviceScore += deviceProperties.limits.maxMemoryAllocationCount;
    deviceScore += deviceProperties.limits.maxSamplerAllocationCount;
    //deviceScore += deviceProperties.limits.bufferImageGranularity;
    //deviceScore += deviceProperties.limits.sparseAddressSpaceSize;
    deviceScore += deviceProperties.limits.maxBoundDescriptorSets;
    deviceScore += deviceProperties.limits.maxPerStageDescriptorSamplers;
    deviceScore += deviceProperties.limits.maxPerStageDescriptorUniformBuffers;
    deviceScore += deviceProperties.limits.maxPerStageDescriptorStorageBuffers;
    deviceScore += deviceProperties.limits.maxPerStageDescriptorSampledImages;
    deviceScore += deviceProperties.limits.maxPerStageDescriptorStorageImages;
    deviceScore += deviceProperties.limits.maxPerStageDescriptorInputAttachments;
    deviceScore += deviceProperties.limits.maxPerStageResources;
    deviceScore += deviceProperties.limits.maxDescriptorSetSamplers;
    deviceScore += deviceProperties.limits.maxDescriptorSetUniformBuffers;
    deviceScore += deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic;
    deviceScore += deviceProperties.limits.maxDescriptorSetStorageBuffers;
    deviceScore += deviceProperties.limits.maxDescriptorSetStorageBuffersDynamic;
    deviceScore += deviceProperties.limits.maxDescriptorSetSampledImages;
    deviceScore += deviceProperties.limits.maxDescriptorSetStorageImages;
    deviceScore += deviceProperties.limits.maxDescriptorSetInputAttachments;
    deviceScore += deviceProperties.limits.maxVertexInputAttributes;
    deviceScore += deviceProperties.limits.maxVertexInputBindings;
    deviceScore += deviceProperties.limits.maxVertexInputAttributeOffset;
    deviceScore += deviceProperties.limits.maxVertexInputBindingStride;
    deviceScore += deviceProperties.limits.maxVertexOutputComponents;
    deviceScore += deviceProperties.limits.maxTessellationGenerationLevel;
    deviceScore += deviceProperties.limits.maxTessellationPatchSize;
    deviceScore += deviceProperties.limits.maxTessellationControlPerVertexInputComponents;
    deviceScore += deviceProperties.limits.maxTessellationControlPerVertexOutputComponents;
    deviceScore += deviceProperties.limits.maxTessellationControlPerPatchOutputComponents;
    deviceScore += deviceProperties.limits.maxTessellationControlTotalOutputComponents;
    deviceScore += deviceProperties.limits.maxTessellationEvaluationInputComponents;
    deviceScore += deviceProperties.limits.maxTessellationEvaluationOutputComponents;
    deviceScore += deviceProperties.limits.maxGeometryShaderInvocations;
    deviceScore += deviceProperties.limits.maxGeometryInputComponents;
    deviceScore += deviceProperties.limits.maxGeometryOutputComponents;
    deviceScore += deviceProperties.limits.maxGeometryOutputVertices;
    deviceScore += deviceProperties.limits.maxGeometryTotalOutputComponents;
    deviceScore += deviceProperties.limits.maxFragmentInputComponents;
    deviceScore += deviceProperties.limits.maxFragmentOutputAttachments;
    deviceScore += deviceProperties.limits.maxFragmentDualSrcAttachments;
    deviceScore += deviceProperties.limits.maxFragmentCombinedOutputResources;
    deviceScore += deviceProperties.limits.maxComputeSharedMemorySize;
    deviceScore += deviceProperties.limits.maxComputeWorkGroupCount[0];
    deviceScore += deviceProperties.limits.maxComputeWorkGroupCount[1];
    deviceScore += deviceProperties.limits.maxComputeWorkGroupCount[2];
    deviceScore += deviceProperties.limits.maxComputeWorkGroupInvocations;
    deviceScore += deviceProperties.limits.maxComputeWorkGroupSize[0];
    deviceScore += deviceProperties.limits.maxComputeWorkGroupSize[1];
    deviceScore += deviceProperties.limits.maxComputeWorkGroupSize[2];
    deviceScore += deviceProperties.limits.subPixelPrecisionBits;
    deviceScore += deviceProperties.limits.subTexelPrecisionBits;
    deviceScore += deviceProperties.limits.mipmapPrecisionBits;
    deviceScore += deviceProperties.limits.maxDrawIndexedIndexValue;
    deviceScore += deviceProperties.limits.maxDrawIndirectCount;
    //deviceScore += deviceProperties.limits.maxSamplerLodBias;
    //deviceScore += deviceProperties.limits.maxSamplerAnisotropy;
    deviceScore += deviceProperties.limits.maxViewports;
    deviceScore += deviceProperties.limits.maxViewportDimensions[0];
    deviceScore += deviceProperties.limits.maxViewportDimensions[1];
    //deviceScore += deviceProperties.limits.viewportBoundsRange[0];
    //deviceScore += deviceProperties.limits.viewportBoundsRange[1];
    deviceScore += deviceProperties.limits.viewportSubPixelBits;
    //deviceScore += deviceProperties.limits.minMemoryMapAlignment;
    //deviceScore += deviceProperties.limits.minTexelBufferOffsetAlignment;
    //deviceScore += deviceProperties.limits.minUniformBufferOffsetAlignment;
    //deviceScore += deviceProperties.limits.minStorageBufferOffsetAlignment;
    //deviceScore += deviceProperties.limits.minTexelOffset;
    deviceScore += deviceProperties.limits.maxTexelOffset;
    //deviceScore += deviceProperties.limits.minTexelGatherOffset;
    deviceScore += deviceProperties.limits.maxTexelGatherOffset;
    //deviceScore += deviceProperties.limits.minInterpolationOffset;
    //deviceScore += deviceProperties.limits.maxInterpolationOffset;
    deviceScore += deviceProperties.limits.subPixelInterpolationOffsetBits;
    deviceScore += deviceProperties.limits.maxFramebufferWidth;
    deviceScore += deviceProperties.limits.maxFramebufferHeight;
    deviceScore += deviceProperties.limits.maxFramebufferLayers;
    //deviceScore += deviceProperties.limits.framebufferColorSampleCounts;
    //deviceScore += deviceProperties.limits.framebufferDepthSampleCounts;
    //deviceScore += deviceProperties.limits.framebufferStencilSampleCounts;
    //deviceScore += deviceProperties.limits.framebufferNoAttachmentsSampleCounts;
    deviceScore += deviceProperties.limits.maxColorAttachments;
    //deviceScore += deviceProperties.limits.sampledImageColorSampleCounts;
    //deviceScore += deviceProperties.limits.sampledImageIntegerSampleCounts;
    //deviceScore += deviceProperties.limits.sampledImageDepthSampleCounts;
    //deviceScore += deviceProperties.limits.sampledImageStencilSampleCounts;
    //deviceScore += deviceProperties.limits.storageImageSampleCounts;
    deviceScore += deviceProperties.limits.maxSampleMaskWords;
    //deviceScore += deviceProperties.limits.timestampComputeAndGraphics;
    //deviceScore += deviceProperties.limits.timestampPeriod;
    deviceScore += deviceProperties.limits.maxClipDistances;
    deviceScore += deviceProperties.limits.maxCullDistances;
    deviceScore += deviceProperties.limits.maxCombinedClipAndCullDistances;
    deviceScore += deviceProperties.limits.discreteQueuePriorities;
    //deviceScore += deviceProperties.limits.pointSizeRange[0];
    //deviceScore += deviceProperties.limits.pointSizeRange[1];
    //deviceScore += deviceProperties.limits.lineWidthRange[0];
    //deviceScore += deviceProperties.limits.lineWidthRange[1];
    //deviceScore += deviceProperties.limits.pointSizeGranularity;
    //deviceScore += deviceProperties.limits.lineWidthGranularity;
    //deviceScore += deviceProperties.limits.strictLines;
    //deviceScore += deviceProperties.limits.standardSampleLocations;
    //deviceScore += deviceProperties.limits.optimalBufferCopyOffsetAlignment;
    //deviceScore += deviceProperties.limits.optimalBufferCopyRowPitchAlignment;
    //deviceScore += deviceProperties.limits.nonCoherentAtomSize;
}

void PhysicalDeviceInfo::addLogicalDevice(VkDeviceCreateInfo * devicecreateinfo, VkSurfaceKHR & surface, uint32_t graphicsqueuecount, uint32_t computequeuecount, VkSwapchainCreateInfoKHR *swapchaincreateinfo){
    if (!devicecreateinfo)
        throw std::runtime_error("VkDeviceCreateInfo is nullptr!");
    if (graphicsqueuecount && !swapchaincreateinfo)
        throw std::runtime_error("Graphics queues are requested but swapchain info is null!!");

    //Ensure required features are supported by the physical device...
    auto missingfeatures = checkFeatures(devicecreateinfo->pEnabledFeatures);
    if (missingfeatures != "")
        throw std::runtime_error(missingfeatures);

    //Create logical device and store it's creation info...
    logicalDevices.resize(logicalDevices.size() + 1);
    if (vkCreateDevice(*physicalDevice, devicecreateinfo, nullptr, &logicalDevices.back()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    //Use different queue families for graphics and compute...
    auto graphicsqueueinfo = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    int graphicsindex;
    graphicsqueueinfo.queueCount ? graphicsindex = static_cast<int>(graphicsqueueinfo.queueFamilyIndex) : graphicsindex = -1;
    auto computequeueinfo = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, graphicsindex);

    //If graphics queues are requested check for presentation support...
    if (graphicsqueuecount > 0){
        VkBool32 presentsupport = false;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(*physicalDevice, graphicsqueueinfo.queueFamilyIndex, surface, &presentsupport) != VK_SUCCESS)
            throw std::runtime_error("The physical device does not support presentation!");
    }

    //Set requested number of queues and initialise device info...
    logicalDeviceInfos.push_back(
                LogicalDevice(
                    &logicalDevices.back(),
                    QueueFamilyInfo(graphicsqueueinfo.queueFamilyIndex, graphicsqueuecount),
                    QueueFamilyInfo(computequeueinfo.queueFamilyIndex, computequeuecount),
                    swapchaincreateinfo
                    )
                );
}

void PhysicalDeviceInfo::recreateSwapChain(uint32_t logicaldeviceindex) noexcept{
    //Wait for the device to finish it's pending tasks before recreating the swapchain...
    vkDeviceWaitIdle(logicalDevices[logicaldeviceindex]);
    logicalDeviceInfos[logicaldeviceindex].recreateSwapChain();
}

void PhysicalDeviceInfo::draw(uint32_t logicaldeviceindex){
    //Draw a frame using the speficifed device...
    logicalDeviceInfos[logicaldeviceindex].drawFrame();
}

std::string PhysicalDeviceInfo::checkQueueProperties(VkQueueFlags requiredflags) const{
    std::string missingqueueproperties;
    VkQueueFlags supportedflags = 0;
    auto checkflags = [&](VkQueueFlags required, const std::string & flagname){
        if ((requiredflags & required) && !(supportedflags & required))
            missingqueueproperties.append(std::string("Physical device does not support ")+flagname+std::string("!\n"));
    };
    for (const auto & queueproperties : deviceQueueFamilyProperties)
        supportedflags = supportedflags | (requiredflags & queueproperties.queueFlags);
    checkflags(VK_QUEUE_GRAPHICS_BIT, std::string("VK_QUEUE_GRAPHICS_BIT"));
    checkflags(VK_QUEUE_COMPUTE_BIT, std::string("VK_QUEUE_COMPUTE_BIT"));
    checkflags(VK_QUEUE_TRANSFER_BIT, std::string("VK_QUEUE_TRANSFER_BIT"));
    checkflags(VK_QUEUE_SPARSE_BINDING_BIT, std::string("VK_QUEUE_SPARSE_BINDING_BIT"));
    checkflags(VK_QUEUE_PROTECTED_BIT, std::string("VK_QUEUE_PROTECTED_BIT"));
    return missingqueueproperties;
}

QueueFamilyInfo PhysicalDeviceInfo::getQueueFamilyIndex(VkQueueFlags requiredflags, int indextoignore) const{
    uint32_t index = 0;
    for (const auto & queueproperties : deviceQueueFamilyProperties){
        if ((requiredflags == (requiredflags & queueproperties.queueFlags)) && static_cast<uint32_t>(indextoignore) != index)
            return QueueFamilyInfo(index, queueproperties.queueCount);
        index++;
    }
    return QueueFamilyInfo(0, 0);
}

std::string PhysicalDeviceInfo::checkFeatures(const VkPhysicalDeviceFeatures * requiredfeatures) const{
    std::string missingfeatures;
    auto checkfeature = [&](VkBool32 supported, VkBool32 required, const std::string & featurename){
        if (!supported && required)
            missingfeatures.append(std::string("Physical device does not support ")+featurename+std::string("!\n"));
    };
    checkfeature(deviceFeatures.robustBufferAccess, requiredfeatures->robustBufferAccess, std::string("robustBufferAccess"));
    checkfeature(deviceFeatures.fullDrawIndexUint32, requiredfeatures->fullDrawIndexUint32, std::string("fullDrawIndexUint32"));
    checkfeature(deviceFeatures.imageCubeArray, requiredfeatures->imageCubeArray, std::string("imageCubeArray"));
    checkfeature(deviceFeatures.independentBlend, requiredfeatures->independentBlend, std::string("independentBlend"));
    checkfeature(deviceFeatures.geometryShader, requiredfeatures->geometryShader, std::string("geometryShader"));
    checkfeature(deviceFeatures.tessellationShader, requiredfeatures->tessellationShader, std::string("tessellationShader"));
    checkfeature(deviceFeatures.sampleRateShading, requiredfeatures->sampleRateShading, std::string("sampleRateShading"));
    checkfeature(deviceFeatures.dualSrcBlend, requiredfeatures->dualSrcBlend, std::string("dualSrcBlend"));
    checkfeature(deviceFeatures.logicOp, requiredfeatures->logicOp, std::string("logicOp"));
    checkfeature(deviceFeatures.multiDrawIndirect, requiredfeatures->multiDrawIndirect, std::string("multiDrawIndirect"));
    checkfeature(deviceFeatures.drawIndirectFirstInstance, requiredfeatures->drawIndirectFirstInstance, std::string("drawIndirectFirstInstance"));
    checkfeature(deviceFeatures.depthClamp, requiredfeatures->depthClamp, std::string("depthClamp"));
    checkfeature(deviceFeatures.depthBiasClamp, requiredfeatures->depthBiasClamp, std::string("depthBiasClamp"));
    checkfeature(deviceFeatures.fillModeNonSolid, requiredfeatures->fillModeNonSolid, std::string("fillModeNonSolid"));
    checkfeature(deviceFeatures.depthBounds, requiredfeatures->depthBounds, std::string("depthBounds"));
    checkfeature(deviceFeatures.wideLines, requiredfeatures->wideLines, std::string("wideLines"));
    checkfeature(deviceFeatures.largePoints, requiredfeatures->largePoints, std::string("largePoints"));
    checkfeature(deviceFeatures.alphaToOne, requiredfeatures->alphaToOne, std::string("alphaToOne"));
    checkfeature(deviceFeatures.multiViewport, requiredfeatures->multiViewport, std::string("multiViewport"));
    checkfeature(deviceFeatures.samplerAnisotropy, requiredfeatures->samplerAnisotropy, std::string("samplerAnisotropy"));
    checkfeature(deviceFeatures.textureCompressionETC2, requiredfeatures->textureCompressionETC2, std::string("textureCompressionETC2"));
    checkfeature(deviceFeatures.textureCompressionASTC_LDR, requiredfeatures->textureCompressionASTC_LDR, std::string("textureCompressionASTC_LDR"));
    checkfeature(deviceFeatures.textureCompressionBC, requiredfeatures->textureCompressionBC, std::string("textureCompressionBC"));
    checkfeature(deviceFeatures.occlusionQueryPrecise, requiredfeatures->occlusionQueryPrecise, std::string("occlusionQueryPrecise"));
    checkfeature(deviceFeatures.pipelineStatisticsQuery, requiredfeatures->pipelineStatisticsQuery, std::string("pipelineStatisticsQuery"));
    checkfeature(deviceFeatures.vertexPipelineStoresAndAtomics, requiredfeatures->vertexPipelineStoresAndAtomics, std::string("vertexPipelineStoresAndAtomics"));
    checkfeature(deviceFeatures.fragmentStoresAndAtomics, requiredfeatures->fragmentStoresAndAtomics, std::string("fragmentStoresAndAtomics"));
    checkfeature(deviceFeatures.shaderTessellationAndGeometryPointSize, requiredfeatures->shaderTessellationAndGeometryPointSize, std::string("shaderTessellationAndGeometryPointSize"));
    checkfeature(deviceFeatures.shaderImageGatherExtended, requiredfeatures->shaderImageGatherExtended, std::string("shaderImageGatherExtended"));
    checkfeature(deviceFeatures.shaderStorageImageExtendedFormats, requiredfeatures->shaderStorageImageExtendedFormats, std::string("shaderStorageImageExtendedFormats"));
    checkfeature(deviceFeatures.shaderStorageImageMultisample, requiredfeatures->shaderStorageImageMultisample, std::string("shaderStorageImageMultisample"));
    checkfeature(deviceFeatures.shaderStorageImageReadWithoutFormat, requiredfeatures->shaderStorageImageReadWithoutFormat, std::string("shaderStorageImageReadWithoutFormat"));
    checkfeature(deviceFeatures.shaderStorageImageWriteWithoutFormat, requiredfeatures->shaderStorageImageWriteWithoutFormat, std::string("shaderStorageImageWriteWithoutFormat"));
    checkfeature(deviceFeatures.shaderUniformBufferArrayDynamicIndexing, requiredfeatures->shaderUniformBufferArrayDynamicIndexing, std::string("shaderUniformBufferArrayDynamicIndexing"));
    checkfeature(deviceFeatures.shaderSampledImageArrayDynamicIndexing, requiredfeatures->shaderSampledImageArrayDynamicIndexing, std::string("shaderSampledImageArrayDynamicIndexing"));
    checkfeature(deviceFeatures.shaderStorageBufferArrayDynamicIndexing, requiredfeatures->shaderStorageBufferArrayDynamicIndexing, std::string("shaderStorageBufferArrayDynamicIndexing"));
    checkfeature(deviceFeatures.shaderStorageImageArrayDynamicIndexing, requiredfeatures->shaderStorageImageArrayDynamicIndexing, std::string("shaderStorageImageArrayDynamicIndexing"));
    checkfeature(deviceFeatures.shaderClipDistance, requiredfeatures->shaderClipDistance, std::string("shaderClipDistance"));
    checkfeature(deviceFeatures.shaderCullDistance, requiredfeatures->shaderCullDistance, std::string("shaderCullDistance"));
    checkfeature(deviceFeatures.shaderFloat64, requiredfeatures->shaderFloat64, std::string("shaderFloat64"));
    checkfeature(deviceFeatures.shaderInt64, requiredfeatures->shaderInt64, std::string("shaderInt64"));
    checkfeature(deviceFeatures.shaderInt16, requiredfeatures->shaderInt16, std::string("shaderInt16"));
    checkfeature(deviceFeatures.shaderResourceResidency, requiredfeatures->shaderResourceResidency, std::string("shaderResourceResidency"));
    checkfeature(deviceFeatures.shaderResourceMinLod, requiredfeatures->shaderResourceMinLod, std::string("shaderResourceMinLod"));
    checkfeature(deviceFeatures.sparseBinding, requiredfeatures->sparseBinding, std::string("sparseBinding"));
    checkfeature(deviceFeatures.sparseResidencyBuffer, requiredfeatures->sparseResidencyBuffer, std::string("sparseResidencyBuffer"));
    checkfeature(deviceFeatures.sparseResidencyImage2D, requiredfeatures->sparseResidencyImage2D, std::string("sparseResidencyImage2D"));
    checkfeature(deviceFeatures.sparseResidencyImage3D, requiredfeatures->sparseResidencyImage3D, std::string("sparseResidencyImage3D"));
    checkfeature(deviceFeatures.sparseResidency2Samples, requiredfeatures->sparseResidency2Samples, std::string("sparseResidency2Samples"));
    checkfeature(deviceFeatures.sparseResidency4Samples, requiredfeatures->sparseResidency4Samples, std::string("sparseResidency4Samples"));
    checkfeature(deviceFeatures.sparseResidency8Samples, requiredfeatures->sparseResidency8Samples, std::string("sparseResidency8Samples"));
    checkfeature(deviceFeatures.sparseResidency16Samples, requiredfeatures->sparseResidency16Samples, std::string("sparseResidency16Samples"));
    checkfeature(deviceFeatures.sparseResidencyAliased, requiredfeatures->sparseResidencyAliased, std::string("sparseResidencyAliased"));
    checkfeature(deviceFeatures.variableMultisampleRate, requiredfeatures->variableMultisampleRate, std::string("variableMultisampleRate"));
    checkfeature(deviceFeatures.inheritedQueries, requiredfeatures->inheritedQueries, std::string("inheritedQueries"));
    return missingfeatures;
}

void PhysicalDeviceInfo::cleanup() noexcept{
    //Cleanup logical devices...
    for (auto device : logicalDeviceInfos)
        device.cleanup();

    //Try to finish any current tasks...
    auto result = VK_RESULT_RANGE_SIZE;
    for (auto device : logicalDevices){
        result = vkDeviceWaitIdle(device);
        if (result != VK_SUCCESS)
            //TO DO: log("VkLogicalDevice failed to complete it's tasks when being destructed!");
        vkDestroyDevice(device, nullptr);
    }
}

uint32_t PhysicalDeviceInfo::getLogicalDeviceCount() const noexcept{
    return static_cast<uint32_t>(logicalDevices.size());
}
