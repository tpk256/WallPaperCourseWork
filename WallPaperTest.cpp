// WallPaperTest.cpp : Определяет точку входа для приложения.
//


#include "framework.h"
#include "WallPaperTest.h"
#define MAX_LOADSTRING 100

import animate;

int HEIGHT, WIDTH;

Test** animation = nullptr;


HWND FindWorkerW() {
    HWND progman = FindWindow(L"Progman", nullptr);
    // Сообщим Progman создать WorkerW
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    HWND workerw = nullptr;
    // Перебираем все окна, ищем такое, где есть SHELLDLL_DefView
    EnumWindows([](HWND top, LPARAM lparam) -> BOOL {
        HWND pView = FindWindowEx(top, nullptr, L"SHELLDLL_DefView", nullptr);
        if (pView) {
            // Нашли окно-иконок, теперь получим соседний WorkerW
            HWND* pWorker = (HWND*)lparam;
            *pWorker = FindWindowEx(nullptr, top, L"WorkerW", nullptr);
            return FALSE; // остановить EnumWindows
        }
        return TRUE;
        }, (LPARAM)&workerw);

    return workerw;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message){
       
        case WM_CREATE:{
                HDC hdcScreen = GetDC(hWnd);
                g_hMemDC = CreateCompatibleDC(hdcScreen);
                g_hBitmap = CreateCompatibleBitmap(hdcScreen, WIDTH, HEIGHT);
                SelectObject(g_hMemDC, g_hBitmap);

                ReleaseDC(hWnd, hdcScreen);

                animation = new Test* [20];

                for (int i = 0; i < 20; i++)
                {
                    animation[i] = new Test;
                }

            }
            break;

        case WM_PAINT:{
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

           
            RECT rc;
            GetClientRect(hWnd, &rc);
       
            HBRUSH brush = CreateSolidBrush(RGB(10, 10, 50));

            FillRect(g_hMemDC, &rc, brush);
            DeleteObject(brush);
            for( int i = 0; i < 20; i++)
            {
                if (!(animation[i]->is_live())){
                    delete animation[i];
                    animation[i] = new Test;
                }
                animation[i]->draw(g_hMemDC);
            }
            

            BitBlt(hdc, 0, 0, WIDTH, HEIGHT, g_hMemDC, 0, 0, SRCCOPY);
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_TIMER:
            InvalidateRect(hWnd, nullptr, FALSE);
            return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);

}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    srand(0);

   

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"MyDesktopClass";
    RegisterClass(&wc);

    HWND parent = FindWorkerW();
    if (!parent) {
        MessageBox(nullptr, L"Не удалось найти WorkerW", L"Error", MB_OK);
        return 1;
    }

    // Создаём наше окно
    RECT rc;
    GetClientRect(parent, &rc);
    HEIGHT = rc.bottom;
    WIDTH = rc.right;

    HWND hwnd = CreateWindowEx(
        NULL,               // прозрачность, если понадобится
        wc.lpszClassName,
        nullptr,
        WS_CHILD | WS_VISIBLE,
        0, 0, WIDTH, HEIGHT,
        parent,
        nullptr,
        hInst,
        nullptr
    );
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetTimer(hwnd, 1, 30, nullptr);

    // Стандартный цикл сообщений
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}





