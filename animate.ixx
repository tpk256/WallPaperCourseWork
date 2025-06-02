export module animate;


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wingdi.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cmath>
#include <iostream>


#include <shellapi.h>
#include <vector>


#define SKY_COLOR RGB(166, 202, 240)

#define WM_TRAYNOTIFY 1001
#define WM_CLOSE_WALLPAPER 1002
#define WM_SHOW_WALLPAPER 1003

#define ID_BUTTON_START 1004
#define ID_BUTTON_STOP 1005
#define ID_CMB_ANIMATION 1006

using namespace std;

LRESULT CALLBACK WndProcWALL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcManageWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

export bool is_play = true;
export std::vector<HWND> hwnd_control_panel;
export HBITMAP g_hBitmap = NULL; // Битовый образ
export HDC g_hMemDC = NULL;      // Контекст памяти
export class Animation; 
export Animation* ani = nullptr;


wchar_t text_type_wallpaper[3][20] = {
    {L"Фейверк"}, {L"Дождь"}, {L"Говно"}
};


export enum class Wallpaper : int {
    FIREWORK = 0,
    RAIN = 1,
    SHIT = 2,

};


int WIDTH_WALLPAPER, HEIGHT_WALLPAPER;
const double PI = 3.14159;


struct Point {
	double x, y;

};


class ParticleFireWork {


	// TODO: 
	// 1) ЕСЛИ ЗАШЛО ЗА РАМКИ ЭКРАНА, ТО ОБНОВИТЬ
	// 2)Использовать битовый обращ

	Point start_pos, cur_pos;
	double start_multiply, cur_multiply;	// радиус
	double direction; // Направление в радианах
	double speed; // Скорость перемещения

	int life; // жизнь в тиках

public:
	ParticleFireWork(double multiply, double x, double y, double d, double s, int l = 20) {
		this->start_pos = Point{x, y};
		this->cur_pos = Point{ x, y };

		this->cur_multiply = this->start_multiply = multiply;
	

		direction = d;
		speed = s;
		life = l;
	}

	Point get_current_pos() const { return cur_pos; }

	bool is_live() const { return life > 0; }

	void update_state(int l = 20) { 
		life = l;
		cur_pos = start_pos;
		cur_multiply = start_multiply;
	}

	void move() {
		cur_pos.x += cur_multiply * (cos(direction) * speed);
		cur_pos.y += cur_multiply * (sin(direction) * speed);
		cur_multiply /= 1.0002;

		if (cur_multiply < 10E-5)
			life = 0;
		else
			life--;
	}

	void draw(HDC hdc) const {
		LineTo(hdc,
			cur_pos.x + cur_multiply *(cos(direction) * speed),
			cur_pos.y + cur_multiply * (sin(direction) * speed));
	
	}
};


class FireWork {
	std::vector<ParticleFireWork*> particles;

public:

	~FireWork() {
		for (auto& particle : particles)
			delete particle;
	}

	FireWork() {
		int r = 1 + rand() % 8,
			x = rand() % 1800 + 200,
			y = 100 + rand() % 800,
			l = 10 + rand() % 20;
		double s = 0.5 + rand() % 3;
		for (int i = 0; i < 20; i++) {

			double d = PI * i / 10;

			particles.push_back(new ParticleFireWork(r, x, y, d, s, l));

		}

	}
	bool is_live() {
		int count = 0;
		for (auto el : particles)
			if (!(el->is_live())) count++;

		if (count == particles.size()) {

			return false;
		}
		return true;
	}
	void draw(HDC hdc) {
		POINT* lp = new POINT;
		HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 195, 11));
		SelectObject(hdc, hPen);
		for (auto particle : particles) {

			Point p = particle->get_current_pos();

			MoveToEx(hdc, p.x, p.y, lp); // выбор позиции

			if (particle->is_live()) {
				particle->draw(hdc);
				particle->move();
			}
			MoveToEx(hdc, lp->x, lp->y, NULL); // восстановление позиции

		}
        delete lp;

		DeleteObject(hPen);
	}



};




export class Animation {

	public:
		virtual void draw(HDC hdc) = 0;
		virtual Wallpaper type_animation() = 0;

};


export class FireWorkAnimation : Animation {
	std::vector<FireWork*> fw;
	Wallpaper type = Wallpaper::FIREWORK;


public:
	FireWorkAnimation(int count = 20) {
		for (int i = 0; i < count; i++)
			fw.push_back(new FireWork());

	}
	void draw(HDC hdc) {
		for (auto& item : fw) {

			if (!item->is_live()) {
				item->~FireWork(); // удаляем партикли
				delete item;
				item = new FireWork();
			}

			item->draw(hdc);
		}
	}
	

	
	Wallpaper type_animation() {
		return type;
	}



};

void SpriteCloud(HDC hdc, LPCWSTR Path, int x, int y, DWORD Param)
{
    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, Path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hBitmap) {
        MessageBox(NULL, L"Не удалось загрузить BMP", L"Ошибка", MB_OK);
        return;
    }


    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int pitch = ((width * 3 + 3) & ~3);  

    BYTE* pixels = new BYTE[pitch * height];

   
    HDC memdc = CreateCompatibleDC(hdc);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; 
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

  
    GetDIBits(memdc, hBitmap, 0, height, pixels, &bmi, DIB_RGB_COLORS);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            BYTE* pixel = pixels + y * pitch + x * 3;

            BYTE blue = pixel[0];
            BYTE green = pixel[1];
            BYTE red = pixel[2];

            if (red == 255 && green == 255 && blue == 255)
            {
               
                pixel[0] = 240;   // Blue
                pixel[1] = 202;   // Green
                pixel[2] = 166; // Red
            }
        }
    }


    HBITMAP newBmp = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT, pixels, &bmi, DIB_RGB_COLORS);

    SelectObject(memdc, newBmp);
    BitBlt(hdc, x, y, width, height, memdc, 0, 0, SRCCOPY);


    DeleteDC(memdc);
    DeleteObject(hBitmap);
    DeleteObject(newBmp);
    delete[] pixels;


 /*   SelectObject(memdc, bmp);
    BitBlt(hdc, x, y, width, height, memdc, 0, 0, Param);*/
}






class ParticleDrop {
    int speed;
    int start_pos_x, start_pos_y, x, y, len;

public:
    ParticleDrop(int s_x, int s_y, int speed, int len) {
        x = start_pos_x = s_x;
        y = start_pos_y = s_y;
        

        this->speed = speed;
        this->len = len;
    }

    void move(){
        if (y > HEIGHT_WALLPAPER){
            y = start_pos_y;
            speed = 15 + rand() % 30;
            len = 8 + rand() % 20;
           
        }
        else
            y += speed;
    }
    void draw(HDC hdc){
        
        MoveToEx(hdc, x, y, NULL); // выбор позиции

        LineTo(hdc, x, y + len);

    }
    
};
class Cloud {
    std::vector<ParticleDrop*> drops;
    int x, y;
public:
    int w = 626, h = 400;
    Cloud(int x, int y, int count_particle, HDC hdc) {
        this->x = x;
        this->y = y;
        SpriteCloud(hdc, L"cloud.bmp", x, y, SRCCOPY);

        int start_x = x;
        for (int i = 0; i < count_particle; i++) {
          
            drops.push_back(new ParticleDrop(start_x + rand() % (w + 1), y + h, 15 + rand() % 30, 8 + rand() % 20));
        }
        
    }

    void draw(HDC hdc) {
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 64, 107));
        SelectObject(hdc, hPen);


        for (auto drop : drops) {
            drop->draw(hdc);
            drop->move();
        }
        DeleteObject(hPen);


    }


};
export class RainAnimation : Animation {
 /*   std::vector<FireWork*> fw;*/
    Wallpaper type = Wallpaper::RAIN;
    std::vector<Cloud*> clouds;
    HBITMAP g_Background = NULL;


public:
    RainAnimation(HWND hwnd, HDC hdc, int count = 3) {
        int start_x = 300, start_y = 100;
        g_Background = CreateCompatibleBitmap(hdc, WIDTH_WALLPAPER, HEIGHT_WALLPAPER);

        HBRUSH brush = CreateSolidBrush(SKY_COLOR);  

        RECT rc;
        rc.left = 0;
        rc.bottom = HEIGHT_WALLPAPER;
        rc.right = WIDTH_WALLPAPER;
        rc.top = 0;

     


        HBITMAP old = (HBITMAP)SelectObject(hdc, g_Background);


        FillRect(hdc, &rc, brush);

        

        for (int i = 0; i < count; i++) {   // Добавляем тучи
            Cloud* p = new Cloud(start_x, start_y, 40 + rand() % 100, hdc);
            start_x += p->w + 50 + rand() % 100;
            clouds.push_back(p);
        }
        DeleteObject(brush);
        



        SelectObject(hdc, old); // ВОЗВРАЩАЕМ СТАРЫЙ HBITMAP


    }
    void draw(HDC hdc) {
        //Достать сохраненный фон
        //Отрисовать капли

        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP frameBmp = CreateCompatibleBitmap(hdc, WIDTH_WALLPAPER, HEIGHT_WALLPAPER);
        HBITMAP old = (HBITMAP)SelectObject(memdc, frameBmp);

        // Восстанавливаем фон
        HDC bgDC = CreateCompatibleDC(hdc);
        HBITMAP oldBg = (HBITMAP)SelectObject(bgDC, g_Background);
        BitBlt(memdc, 0, 0, WIDTH_WALLPAPER, HEIGHT_WALLPAPER, bgDC, 0, 0, SRCCOPY);
        SelectObject(bgDC, oldBg);
        DeleteDC(bgDC);

        for (auto cloud : clouds) {
            cloud->draw(memdc);
        }
        // Выводим кадр на экран
        BitBlt(hdc, 0, 0, WIDTH_WALLPAPER, HEIGHT_WALLPAPER, memdc, 0, 0, SRCCOPY);
       

        // Очистка
        SelectObject(memdc, old);
        DeleteObject(frameBmp);
        DeleteDC(memdc);

       
    }
    ~RainAnimation() {
        if (g_Background) {
            DeleteObject(g_Background);
            g_Background = NULL;
        }
    }



    Wallpaper type_animation() {
        return type;
    }



};

/// <summary>
/// /////
/// </summary>
/// 
/// 
/// 
/// 
///








LRESULT CALLBACK WndProcManageWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {



    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        return 0;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case ID_CMB_ANIMATION:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                // Индекс выбранной анимации

                int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

                switch ((Wallpaper)ItemIndex)
                {

                case Wallpaper::FIREWORK:
                    if (ani->type_animation() == Wallpaper::FIREWORK) {
                        MessageBox(hWnd, L"Без изменений", L"Info", MB_OK);
                    }
                    else {
                        if (ani)
                            delete ani;
                        ani = (Animation*) new FireWorkAnimation();
                    }
                    
                    break;

                case Wallpaper::RAIN:
                    if (ani->type_animation() == Wallpaper::RAIN) {
                        MessageBox(hWnd, L"Без изменений", L"Info", MB_OK);
                    }
                    else {
                        if (ani)
                            delete ani;
                        ani = (Animation *) new RainAnimation(hWnd, g_hMemDC);
                    }
                  
                    break;

                case Wallpaper::SHIT:
                    MessageBox(hWnd, L"В разработке!", L"Info", MB_OK);
                    break;

                default:
                    break;
                }



            }

            break;

        case WM_SHOW_WALLPAPER:
            ShowWindow(hWnd, SW_SHOW);
            break;

        case WM_CLOSE_WALLPAPER:
            PostQuitMessage(0);
            break;

        case ID_BUTTON_START:
            if (HIWORD(wParam) == BN_CLICKED) {
                is_play = true;
            }

            break;

        case ID_BUTTON_STOP:
            if (HIWORD(wParam) == BN_CLICKED) {
                is_play = false;
            }

            break;




        }
        return 0;

    case WM_TRAYNOTIFY:
        switch (LOWORD(lParam)) {
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDBLCLK:
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, WM_SHOW_WALLPAPER, L"Change Wallpaper");
            AppendMenu(hMenu, MF_STRING, WM_CLOSE_WALLPAPER, L"Quit");

            SetForegroundWindow(hWnd); // обязательно!
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
            break;
        }

        return 0;

    case WM_CREATE:
        break;

    case WM_PAINT:
        break;

    }

    return DefWindowProc(hWnd, message, wParam, lParam);

}


export bool register_class_manage_window(const wchar_t* class_name, HINSTANCE hInstance) {

    static bool flag_register = false;
    if (flag_register) return true;

    WNDCLASSEX wndclass;
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProcManageWindow;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = class_name;
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&wndclass);

    flag_register = true;

    return flag_register;

}


export HWND create_button_start(HWND hParent, HINSTANCE hInstance) {

    HWND button_start = CreateWindowEx(
        NULL,
        L"BUTTON",
        L"Старт",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        40, 20, 90, 26,
        hParent,
        (HMENU)ID_BUTTON_START,
        hInstance,
        nullptr
    );



    return button_start;
}


export HWND create_button_stop(HWND hParent, HINSTANCE hInstance) {

    HWND button_stop = CreateWindowEx(
        NULL,
        L"BUTTON",
        L"Стоп",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        150, 20, 90, 26,
        hParent,
        (HMENU)ID_BUTTON_STOP,
        hInstance,
        nullptr
    );



    return button_stop;
}


export HWND create_cmb_animate(HWND hParent, HINSTANCE hInstance) {

    HWND cmb = CreateWindowEx(
        NULL,
        L"COMBOBOX",
        NULL,
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        40, 70, 180, 150,
        hParent,
        (HMENU)ID_CMB_ANIMATION,
        hInstance,
        nullptr
    );
    for (size_t i = 0; i < 3; i++) {

        SendMessage(cmb, CB_ADDSTRING, 0, (LPARAM)text_type_wallpaper[i]);
    }


    SendMessage(cmb, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
    return cmb;
}





export HWND create_tray_notify(HWND hParent, HINSTANCE hInstance) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hParent;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYNOTIFY;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    wcscpy_s(nid.szTip, L"Wallpaper");

    Shell_NotifyIcon(NIM_ADD, &nid);
    return (HWND)-1;
}



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


export HWND create_wallpaper(HWND hParent, HINSTANCE hInstance) {
    srand(0);



    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProcWALL;
    wc.hInstance = hInstance;

    wc.lpszClassName = L"wallpaper_win";
    RegisterClass(&wc);

    HWND parent = FindWorkerW();
    if (!parent) {
        MessageBox(nullptr, L"Не удалось найти WorkerW", L"Error", MB_OK);
        PostQuitMessage(-1);
    }

    // Создаём наше окно
    RECT rc;
    GetClientRect(parent, &rc);
    HEIGHT_WALLPAPER = rc.bottom;
    WIDTH_WALLPAPER = rc.right;

    HWND hwnd = CreateWindowEx(
        NULL,
        wc.lpszClassName,
        nullptr,
        WS_CHILD | WS_VISIBLE,
        0, 0, WIDTH_WALLPAPER, HEIGHT_WALLPAPER,
        parent,
        nullptr,
        hInstance,
        nullptr
    );
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetTimer(hwnd, 1, 30, nullptr);

    return hwnd;


}






LRESULT CALLBACK WndProcWALL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_CREATE: {
        HDC hdcScreen = GetDC(hWnd);
        g_hMemDC = CreateCompatibleDC(hdcScreen);
        g_hBitmap = CreateCompatibleBitmap(hdcScreen, WIDTH_WALLPAPER, HEIGHT_WALLPAPER);
        SelectObject(g_hMemDC, g_hBitmap);

        ReleaseDC(hWnd, hdcScreen);

        // По умолчанию у нас анимация будет устанавливаться фейрверка
        if (!ani) {
            ani = (Animation*) new FireWorkAnimation();
        }

    }
                  break;

    case WM_PAINT: {


        if (!is_play || !ani)
            break;



        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (ani->type_animation() == Wallpaper::FIREWORK) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            HBRUSH brush = CreateSolidBrush(RGB(10, 10, 50));   //по дефолту для фейрверка
            
            FillRect(g_hMemDC, &rc, brush);

            DeleteObject(brush);

            ani->draw(g_hMemDC);

            BitBlt(hdc, 0, 0, WIDTH_WALLPAPER, HEIGHT_WALLPAPER, g_hMemDC, 0, 0, SRCCOPY);
            
        }
        else if (ani->type_animation() == Wallpaper::RAIN) {

            ani->draw(g_hMemDC);

            BitBlt(hdc, 0, 0, WIDTH_WALLPAPER, HEIGHT_WALLPAPER, g_hMemDC, 0, 0, SRCCOPY);
        }

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_TIMER:
        InvalidateRect(hWnd, nullptr, FALSE);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);

}


