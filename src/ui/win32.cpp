#include "win32.h"
#include "src/utility.h"

//The Windows procedure...
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZING:
        //VulkanRenderer::setWindowResized(true);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

Win32::Win32(WindowCreateInfo &windowcreateinfo)
    : createInfo(windowcreateinfo)
{
    //Get window handle and register the Window Class...
    handle = windowcreateinfo.hInstance;
    windowsClass.cbSize = sizeof(WNDCLASSEX);
    windowsClass.style = 0;
    windowsClass.lpfnWndProc = WndProc;
    windowsClass.cbClsExtra = 0;
    windowsClass.cbWndExtra = 0;
    windowsClass.hInstance = nullptr;
    windowsClass.hIcon = nullptr;
    windowsClass.hCursor = nullptr;
    windowsClass.hbrBackground = nullptr;
    windowsClass.lpszMenuName = nullptr;
    windowsClass.lpszClassName = L"Null";
    windowsClass.hIconSm = nullptr;
    if (!RegisterClassEx(&windowsClass))
        throw std::runtime_error("Failed to register Windows class!");

    //Create the window...
    window = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"Null",
                L"Artic Engine",
                WS_OVERLAPPEDWINDOW,
                static_cast<int>(0x80000000),
                static_cast<int>(0x80000000),
                1980,
                1020,
                nullptr,
                nullptr,
                handle,
                nullptr
                );
}

bool Win32::processMessage()  const noexcept{
    auto messaged = false;
    //Process messages...
    if (MSG message; GetMessage(&message, nullptr, 0, 0)){    //Use PeekMessage to not block user input???
        TranslateMessage(&message);
        DispatchMessage(&message);
        messaged = true;
    }
    return messaged;
}

bool Win32::showWindow() noexcept{
    auto result = true;
    if (!ShowWindow(window, createInfo.nShowCmd))
        result = false;
    createInfo.nShowCmd = SW_SHOW;
    SetForegroundWindow(window);
    SetFocus(window);
    return result;
}

HINSTANCE Win32::getWindows32Handle() const noexcept{
    return handle;
}

HWND Win32::getWindow() const noexcept{
    return window;
}
