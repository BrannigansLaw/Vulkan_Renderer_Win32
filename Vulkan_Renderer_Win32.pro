TARGET = Vulkan_Renderer_Win32
TEMPLATE = app

CONFIG += c++17
QMAKE_CXXFLAGS += /std:c++17

INCLUDEPATH += C:/VulkanSDK/1.1.85.0/Include/vulkan/
LIBS += "-LC:/VulkanSDK/1.1.85.0/Lib/"
LIBS += "-LC:/VulkanSDK/1.1.85.0/Lib/" -lvulkan-1
LIBS += "-LC:/Windows/WinSxS/amd64_microsoft-windows-user32_31bf3856ad364e35_10.0.17134.320_none_aef35d46c36383b6/" -luser32

SOURCES += \
        src\main.cpp \
    src/renderer/vulkanrenderer.cpp \
    src/renderer/physicaldeviceinfo.cpp \
    src/renderer/logicaldevice.cpp \
    src/renderer/swapchain.cpp \
    src/renderer/graphicspipeline.cpp \
    src/ui/win32.cpp \
    src/renderer/vulkanvalidationlayers.cpp

HEADERS += \
    src/renderer/vulkanrenderer.h \
    src/renderer/physicaldeviceinfo.h \
    src/renderer/logicaldevice.h \
    src/renderer/swapchain.h \
    src/utility.h \
    src/renderer/graphicspipeline.h \
    src/ui/win32.h \
    src/renderer/vulkanvalidationlayers.h

DISTFILES += \
    src/renderer/shaders/shader.vert \
    src/renderer/shaders/shader.frag
