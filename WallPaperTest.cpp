



#include "framework.h"

import animate;


int HEIGHT, WIDTH;


using namespace std;





int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {

    wchar_t class_name[] = L"manage_window";
    WIDTH = 300;
    HEIGHT = 256;
    if (register_class_manage_window(class_name, hInst)) {}
   

    HWND hwnd = CreateWindowEx(
        NULL,
        class_name,
        L"WallPaper",
        WS_VISIBLE | WS_BORDER | WS_SYSMENU,
        1920 / 2, 1080 / 2, WIDTH, HEIGHT,
        NULL,
        nullptr,
        hInst,
        nullptr
    );

    hwnd_control_panel.push_back(create_button_start(hwnd, hInst));
    hwnd_control_panel.push_back(create_button_stop(hwnd, hInst));
    hwnd_control_panel.push_back(create_cmb_animate(hwnd, hInst));
    hwnd_control_panel.push_back(create_tray_notify(hwnd, hInst)); 

    hwnd_control_panel.push_back(create_wallpaper(hwnd, hInst));


    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}