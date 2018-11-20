#ifndef WIN32_H
#define WIN32_H

#include <Windows.h>
#include "src/utility.h"

class Win32 final
{
public:
    Win32(WindowCreateInfo &windowcreateinfo);
    bool processMessage() const noexcept;
    bool showWindow() noexcept;
    [[nodiscard]] HINSTANCE getWindows32Handle() const noexcept;
    [[nodiscard]] HWND getWindow() const noexcept;
public:
    ~Win32() = default;
    Win32(const Win32 & other) = delete;
    Win32 & operator=(const Win32 & other) = delete;
    Win32(const Win32 && other) = delete;
    Win32 & operator=(const Win32 && other) = delete;
private:
    WindowCreateInfo createInfo;
    HWND window;
    HINSTANCE handle;
    WNDCLASSEX windowsClass;
};

#endif // WIN32_H
