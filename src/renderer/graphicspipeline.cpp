#include "graphicspipeline.h"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

GraphicsPipeline::Shader::Shader(VkDevice *device, const std::string & filepath, VkShaderStageFlagBits shadertype)
{
    if (!device)
        throw std::runtime_error("Null device was passed to Shader!");
    if (filepath == "")
        throw std::runtime_error("Empty string filename was passed to Shader!");

    //Get name of file (strip path)...
    auto index = filepath.find_last_of('\\');
    if (index == (std::numeric_limits<size_t>::max)()) //Not Windows...
        index = filepath.find_last_of('/');
    if (index != (std::numeric_limits<size_t>::max)()){
        name.resize(filepath.size() - 1 - index++);
        auto j = 0U;
        while (index < filepath.size())
            name[j++] = filepath[index++];
        name[j] = '\0';
    }else{
        name = filepath;
    }
    path = filepath;

    //Set or figure out shader type by checking it's name...
    if (shadertype == VK_SHADER_STAGE_ALL){
        if (name.find(VERTEX_SHADER_SUBSTRING) != std::string::npos){
            stageFlag = VK_SHADER_STAGE_VERTEX_BIT;
        }else if (name.find(FRAGMENT_SHADER_SUBSTRING) != std::string::npos){
            stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
        }else if (name.find(TESSELLATION_CONTROL_SHADER_SUBSTRING) != std::string::npos){
            stageFlag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }else if (name.find(TESSELLATION_EVALUATION_SHADER_SUBSTRING) != std::string::npos){
            stageFlag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }else if (name.find(GEOMETRY_SHADER_SUBSTRING) != std::string::npos){
            stageFlag = VK_SHADER_STAGE_GEOMETRY_BIT;
        }else if (name.find(COMPUTE_SHADER_SUBSTRING) != std::string::npos){
            stageFlag = VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }else{
        stageFlag = shadertype;
    }

    //Load shader...
    auto code = readFile(filepath);
    if (code.empty())
        throw std::runtime_error("Empty shader found!");

    //Create shader...
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(*device, &createInfo, nullptr, &shader) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
}

GraphicsPipeline::GraphicsPipeline(VkDevice *device)
    : logicalDevice(device)
{
    if (!device)
        throw std::runtime_error("Null device was passed to Shader!");

    //Generate path to shaders directory...
    auto currentpath = fs::current_path().u8string();
    auto index = currentpath.find_last_of('\\') + 1;
    std::string shaderpath = PATH_TO_SHADERS_DIRECTORY_WINDOWS;
    if (index == (std::numeric_limits<size_t>::max)()){ //Not Windows...
        index = currentpath.find_last_of('/') + 1;
        shaderpath = PATH_TO_SHADERS_DIRECTORY_LINUX;
        if (index == (std::numeric_limits<size_t>::max)())
            throw std::runtime_error("GraphicsPipeline: Invalid directory path!");
    }
    currentpath.resize(currentpath.size() + 1 + shaderpath.size() - sizeof("build"));
    auto j = 0U;
    while (index < currentpath.size())
        currentpath[index++] = shaderpath[j++];
    currentpath[index] = '\0';

    //Search shaders directory for shaders to load...
    std::vector <std::string> shadernames;
    for (auto & shader : fs::directory_iterator(currentpath)){
        auto name = shader.path().filename().generic_u8string();
        if (name.find(".spv") != std::string::npos)
            shadernames.push_back(shader.path().generic_u8string());
    }

    //Create shaders...
    for (const auto & name : shadernames)
        shaders.push_back(Shader(logicalDevice, name));
}

void GraphicsPipeline::createRenderpass(VkFormat & format){
    //Create renderpass...
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(*logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass!");
}

void GraphicsPipeline::initializeFixedFunctions(VkExtent2D &swapchainextent){
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainextent.width);
    viewport.height = static_cast<float>(swapchainextent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchainextent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(*logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (auto & shader : shaders){
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.pName = "main";
        shaderStageInfo.stage = shader.stageFlag;
        shaderStageInfo.module = shader.shader;
        shaderStageInfo.pSpecializationInfo = nullptr;
        shaderStages.push_back(shaderStageInfo);
    }

    createGraphicsPipeline(
                shaderStages.data(),
                &vertexInputInfo,
                &inputAssembly,
                nullptr,
                &viewportState,
                &rasterizer,
                &multisampling,
                nullptr,
                &colorBlending,
                &dynamicState
                );
}

void GraphicsPipeline::createGraphicsPipeline(
        VkPipelineShaderStageCreateInfo * shaderStages,
        VkPipelineVertexInputStateCreateInfo * vertexInputInfo,
        VkPipelineInputAssemblyStateCreateInfo * inputAssembly,
        VkPipelineTessellationStateCreateInfo * tessellation,
        VkPipelineViewportStateCreateInfo * viewportState,
        VkPipelineRasterizationStateCreateInfo * rasterizer,
        VkPipelineMultisampleStateCreateInfo * multisampling,
        VkPipelineDepthStencilStateCreateInfo * depthStencil,
        VkPipelineColorBlendStateCreateInfo * colorBlending,
        VkPipelineDynamicStateCreateInfo * dynamicState
        )
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = vertexInputInfo;
    pipelineInfo.pInputAssemblyState = inputAssembly;
    pipelineInfo.pTessellationState = tessellation;
    pipelineInfo.pViewportState = viewportState;
    pipelineInfo.pRasterizationState = rasterizer;
    pipelineInfo.pMultisampleState = multisampling;
    pipelineInfo.pDepthStencilState = depthStencil;
    pipelineInfo.pColorBlendState = colorBlending;
    pipelineInfo.pDynamicState = dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;
    pipelineInfo.basePipelineIndex = -1;
    if (vkCreateGraphicsPipelines(*logicalDevice, nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline!");
}

void GraphicsPipeline::cleanup(bool destroyshaders) noexcept{
    if (destroyshaders){
        for (const auto & shader : shaders)
            vkDestroyShaderModule(*logicalDevice, shader.shader, nullptr);
        shaders.clear();
    }
    vkDestroyPipeline(*logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(*logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(*logicalDevice, renderPass, nullptr);
}

VkRenderPass GraphicsPipeline::getRenderPass() const{
    return renderPass;
}

void GraphicsPipeline::startRenderPass(
        VkFramebuffer & framebuffer,
        VkExtent2D & swapchainextent,
        VkCommandBuffer & commandbuffer,
        bool primarybuffer
        )
{
    //Set up the renderpass create info using the framebuffer associated with the swapchain image we are sampling from...
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainextent;

    //Set the default color attachment to black...
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    primarybuffer ? vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE) :
                    vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    //Bind the command buffer to the graphics pipeline and execute the commands in it...
    vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdDraw(commandbuffer, 3, 1, 0, 0);   //TO DO: passed index buffer, use direct or indirect draw?

    //End the render pass and finish recording the command buffer...
    vkCmdEndRenderPass(commandbuffer);
    if (vkEndCommandBuffer(commandbuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer!");
}
