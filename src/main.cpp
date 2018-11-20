#include "src/renderer/vulkanrenderer.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <array>
#include "utility.h"

int WINAPI WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nShowCmd
        )
{
    static LogFile logfile;
    WindowCreateInfo createinfo(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    VulkanRenderer renderer(createinfo);
    std::array<QueueInfo, 2> flags = {
        QueueInfo(VK_QUEUE_GRAPHICS_BIT, std::vector<float> {1.0}),
        QueueInfo(VK_QUEUE_COMPUTE_BIT, std::vector<float> {1.0})
    };
    VkPhysicalDeviceFeatures features {};
    features.multiDrawIndirect = VK_TRUE;
    features.geometryShader = VK_TRUE;
    features.tessellationShader = VK_TRUE;
    renderer.addLogicalDevice(flags, features);
    uint16_t index = 0;
    std::array <uint64_t, (std::numeric_limits<uint8_t>::max)()> frametimes;
    while (renderer.keepRendering()){
        auto t1 = std::chrono::system_clock::now();
        renderer.drawFrame();
        auto t2 = std::chrono::system_clock::now();
        auto int_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()/100;
        //LogFile::writeToLog(std::string("Frametime (rounded): ") + std::to_string(int_us) + std::string(" us"));
        frametimes[index++] = int_us;
        if (index == frametimes.size()){
            uint64_t avg = 0;
            for (auto i : frametimes)
                avg = avg + i;
            avg = avg/frametimes.size();
            LogFile::writeToLog(std::string("\nFPS (rounded): ") + std::to_string(int_us) + std::string("\n"));
            index = 0;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
