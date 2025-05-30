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

#include <vector>

export HBITMAP g_hBitmap = NULL; // Битовый образ
export HDC g_hMemDC = NULL;      // Контекст памяти


const double PI = 3.14159;


struct Point {
	double x, y;

};


class ParticleStar {


	// TODO: 
	// 1) ЕСЛИ ЗАШЛО ЗА РАМКИ ЭКРАНА, ТО ОБНОВИТЬ
	// 2)Использовать битовый обращ

	Point start_pos, cur_pos;
	double start_multiply, cur_multiply;	// радиус
	double direction; // Направление в радианах
	double speed; // Скорость перемещения

	int life; // жизнь в тиках

public:
	ParticleStar(double multiply, double x, double y, double d, double s, int l = 20) {
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

enum class Animation {
	Blood,
	Rain
};






export class Test {

	std::vector<ParticleStar*> test;

public:
	Test() {
		int r = 1 + rand() % 8,
			x = rand() % 1800 + 200, 
			y = 100 + rand() % 800,
			l = 10 + rand()% 20;
		double s = 0.5 + rand() % 3;
		for (int i = 0; i < 20; i++) {
			
			double d = PI * i / 10;

			test.push_back(new ParticleStar(r, x, y, d, s, l));
			
		}

	}

	bool is_live() {
		int count = 0;
		for (auto el : test) 
			if (!(el->is_live())) count++;

		if (count == test.size()) {
			for (auto el : test)
				delete el;	// Удаляем партикли

			return false;
		}
		return true;
	}

	void draw(HDC hdc) {
		POINT * lp = new POINT;
		HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 195, 11));
		SelectObject(hdc, hPen);
		for (auto particle : test) {
			
			Point p = particle->get_current_pos();	

			MoveToEx(hdc, p.x, p.y, lp); // выбор позиции

			if (particle->is_live()) {
				particle->draw(hdc);
				particle->move();
			}
			MoveToEx(hdc, lp->x, lp->y, NULL); // восстановление позиции
			
		}

		DeleteObject(hPen);
	}
};

