#include <Windows.h>

int main()
{
    HWND vs_hwnd = (HWND)0x2072C;
    SetWindowLong(vs_hwnd, GWL_EXSTYLE, GetWindowLong(vs_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(vs_hwnd, 0, 255, LWA_ALPHA);
    return 0;
}
