#pragma once
#include "framework.h"
#include <array>

struct field
{
public:
	RECT position;
	TCHAR c = L' ';
	COLORREF color_fill = RGB(251, 252, 255);
	COLORREF color_outline = RGB(222, 225, 233);
};


struct key_field : field
{
public:
	field medium_field[2];
	field hard_field[4];
	bool show = false;
};

class board
{
public:
	int rows = 6;
	int columns = 5;
	int margin = 6;
	int field_size = 55;
	int field_count = rows * columns;
	int width = columns * (field_size + margin) + margin;
	int height = rows * (field_size + margin) + margin;
	board() {}
	board(int);
	field *m_fields = nullptr;
};

class board_key
{
public:
	int field_count = 26;
	int field_size = 55;
	int margin = 6;
	key_field* k_fields = nullptr;
	LPCWSTR letters = L"QWERTYUIOPASDFGHJKLZXCVBNM";
	board_key();
};