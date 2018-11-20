#ifndef UTILITY_H
#define UTILITY_H

#include <vulkan.h>
#include <Windows.h>
#include <vulkan_win32.h>
#include <vector>
#include <string>
#include <array>
#include <fstream>
#include <mutex>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

#define MAX_NUM_PHYSICAL_DEVICES_SUPPORTED 2
#define MAX_NUM_LOGICAL_DEVICES_PER_PHYSICAL_DEVICE_ALLOWED 2
#define MAX_NUM_QUEUE_TYPES_ALLOWED 2
#define MIN_SWAPCHAIN_IMAGE_COUNT 2
#define MAX_NUM_SHADERS_PER_GRAPHICS_PIPELINE_ALLOWED 8
#define VK_EXTENT_1080_P {1920, 1080}
#define VK_EXTENT_1440_P {2560, 1440}
#define VK_EXTENT_4K {3840, 2160}
#define VK_EXTENT_TRUE_4K {4096, 2160}
#define PATH_TO_SHADERS_DIRECTORY_WINDOWS "src\\renderer\\shaders"
#define PATH_TO_SHADERS_DIRECTORY_LINUX "src/renderer/shaders"
#define VERTEX_SHADER_SUBSTRING "vert."
#define FRAGMENT_SHADER_SUBSTRING "frag."
#define TESSELLATION_CONTROL_SHADER_SUBSTRING "tesc."
#define TESSELLATION_EVALUATION_SHADER_SUBSTRING "tese."
#define GEOMETRY_SHADER_SUBSTRING "geom."
#define COMPUTE_SHADER_SUBSTRING "comp."
#define PATH_TO_LOG_DIRECTORY_WINDOWS "logs\\debug.txt"
#define PATH_TO_LOG_DIRECTORY_LINUX "logs/debug.txt"

class WindowCreateInfo final
{
public:
    WindowCreateInfo(
            HINSTANCE instance,
            HINSTANCE previnstance,
            LPSTR lpcmdline,
            int showcmd
            )
        : hInstance(instance),
          hPrevInstance(previnstance),
          lpCmdLine(lpcmdline),
          nShowCmd(showcmd)
    {
        //
    }
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpCmdLine;
    int nShowCmd;
};

class QueueFamilyInfo final
{
public:
    QueueFamilyInfo(const uint32_t family, const uint32_t count)
        : queueFamilyIndex(family),
          queueCount(count)
    {
        //
    }
public:
    const uint32_t queueFamilyIndex;
    const uint32_t queueCount;
};

class QueueInfo final
{
public:
    QueueInfo(const VkQueueFlags flg, const std::vector <float> & queuepriority)
        : flag(flg),
          prioritys(queuepriority)
    {
        //
    }
public:
    const VkQueueFlags flag;
    const std::vector <float> prioritys;
};

class ShaderCreateInfo final
{
public:
    ShaderCreateInfo(const std::string & shadername, const std::string & entrypoint, VkShaderStageFlagBits shaderstage)
        : shaderName(shadername),
          entryPoint(entrypoint),
          shaderStage(shaderstage)
    {
        //
    }
public:
    const std::string shaderName;
    const std::string entryPoint;
    const VkShaderStageFlagBits shaderStage;
};

namespace {

class LogFile final
{
private:
    static std::ofstream logFile;
    static std::mutex mutex;
public:
    LogFile(){
        //Generate path to log file...
        auto currentpath = fs::current_path().u8string();
        auto index = currentpath.find_last_of('\\') + 1;
        std::string logpath = PATH_TO_LOG_DIRECTORY_WINDOWS;
        if (index == (std::numeric_limits<size_t>::max)()){ //Not Windows...
            index = currentpath.find_last_of('/') + 1;
            logpath = PATH_TO_LOG_DIRECTORY_LINUX;
            if (index == (std::numeric_limits<size_t>::max)())
                throw std::runtime_error("LogFile: Invalid directory path!");
        }
        currentpath.resize(currentpath.size() + 1 + logpath.size() - sizeof("build"));
        auto j = 0U;
        while (index < currentpath.size())
            currentpath[index++] = logpath[j++];
        currentpath[index] = '\0';
        logFile.open(currentpath, std::ios::out | std::ios::trunc);
    }

    ~LogFile(){
        logFile << "\n\nApplication closing... Bye..\n";
        logFile.close();
    }

    static void writeToLog(const char * message){
        std::lock_guard <std::mutex> guard(mutex);
        //logFile.open(QDir::currentPath().toStdString()+"/DebugLog.txt", std::ios::out | std::ios::trunc);
        logFile << message << "\n";
    }

    static void writeToLog(const std::string & message){
        std::lock_guard <std::mutex> guard(mutex);
        //logFile.open(QDir::currentPath().toStdString()+"/DebugLog.txt", std::ios::out | std::ios::trunc);
        logFile << message << "\n";
    }

    static void writeToLog(const std::vector <std::string> & messages){
        std::lock_guard <std::mutex> guard(mutex);
        //logFile.open(QDir::currentPath().toStdString()+"/DebugLog.txt", std::ios::out | std::ios::trunc);
        for (auto i = 0U; i < messages.size(); i++){
            logFile << messages.at(i) << "\n";
        }
    }
};

std::ofstream LogFile::logFile;
std::mutex LogFile::mutex;

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open file!");
    auto filesize = file.tellg();
    std::vector<char> buffer(static_cast<size_t>(filesize));
    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();
    return buffer;
}

}

#endif // UTILITY_H
