#pragma once
#include "framework.h"
#include "board.h"

class game
{
private:
	static std::wstring const s_class_name;
	HINSTANCE m_instance;
	HWND h_parent;
	DWORD game_style = WS_OVERLAPPED | WS_CAPTION;

	HDC off_DC = NULL;
	HBITMAP off_old_bitmap = NULL;
	HBITMAP off_bitmap = NULL;


	bool register_class();
	
	static LRESULT CALLBACK window_proc_static(
		HWND window,
		UINT message,
		WPARAM wparam,
		LPARAM lparam);
	LRESULT window_proc(
		HWND window,
		UINT message,
		WPARAM wparam,
		LPARAM lparam);
	HWND create_window();

	void draw_tiles(HDC);

	void draw_letter(HDC);

	void draw_overlay(HDC);

public:
	int wnd_width;
	int wnd_height;
	int current_row = 0;
	int current_column = 0;
	bool won = false;
	bool lost = false;
	std::wstring word_solution;
	HWND h_game;
	board m_board;

	game();
	game(HINSTANCE instance, HWND parent, int rows);
};