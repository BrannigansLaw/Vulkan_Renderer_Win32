#ifndef GRAPHICSPIPELINE_H
#define GRAPHICSPIPELINE_H

#include "src/utility.h"

class GraphicsPipeline
{
    friend class LogicalDevice;
    friend class SwapChain;
private:
    struct Shader final
    {
        Shader(VkDevice *device, const std::string & filepath, VkShaderStageFlagBits shadertype = VK_SHADER_STAGE_ALL);
        std::string name;
        std::string path;
        VkShaderStageFlagBits stageFlag;
        VkShaderModule shader;
    };
public:
    GraphicsPipeline(VkDevice *device);
public:
    GraphicsPipeline() = default;
    ~GraphicsPipeline() = default;
    GraphicsPipeline(const GraphicsPipeline & other) = default;
    GraphicsPipeline & operator=(const GraphicsPipeline & other) = default;
private:
    void initializeFixedFunctions(VkExtent2D & swapchainextent);
    void createRenderpass(VkFormat &format);
    [[nodiscard]] VkRenderPass getRenderPass() const;
    void createGraphicsPipeline(
            VkPipelineShaderStageCreateInfo *shaderStages,
            VkPipelineVertexInputStateCreateInfo * vertexInputInfo,
            VkPipelineInputAssemblyStateCreateInfo * inputAssembly,
            VkPipelineTessellationStateCreateInfo * tessellation,
            VkPipelineViewportStateCreateInfo * viewportState,
            VkPipelineRasterizationStateCreateInfo * rasterizer,
            VkPipelineMultisampleStateCreateInfo * multisampling,
            VkPipelineDepthStencilStateCreateInfo * depthStencil,
            VkPipelineColorBlendStateCreateInfo * colorBlending,
            VkPipelineDynamicStateCreateInfo * dynamicState
            );
    void startRenderPass(
            VkFramebuffer &framebuffer,
            VkExtent2D &swapchainextent,
            VkCommandBuffer &commandbuffer,
            bool primarybuffer = true
            );
    void cleanup(bool destroyshaders = true) noexcept;
private:
    VkDevice *logicalDevice;
    std::vector <Shader> shaders;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
};

#endif // GRAPHICSPIPELINE_H
