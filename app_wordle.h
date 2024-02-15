#pragma once
#include "framework.h"
#include "game.h"
#include <unordered_set>
#include <fstream>

class app_wordle
{
private:
	static std::wstring const s_class_name;
	COLORREF GREEN = RGB(121, 184, 81);
	COLORREF ORANGE = RGB(243, 194, 55);
	COLORREF GRAY = RGB(164, 174, 196);
	std::wifstream file;
	HINSTANCE m_instance;
	HWND h_keyboard;
	DWORD keyboard_style = WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	HMENU h_menu;
	std::wstring letters = L"QWERTYUIOPASDFGHJKLZXCVBNM";
	int wnd_width = 630;
	int wnd_height = 250;

	UINT difficulty;
	game* games[4];
	int current_games;
	int max_rows;
	int max_columns = 5;
	board_key k_board;

	HDC key_DC = NULL;
	HBITMAP key_old_bitmap = NULL;
	HBITMAP key_bitmap = NULL;

	std::unordered_set<std::wstring> words;
	TCHAR build_guess[6];
	std::wstring word_guess;


	bool register_class();
	void init();
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
	HWND create_keywindow();
	void draw_tiles(HDC);
	void draw_letter(HDC);
	void start_game();
	void end_game();
	void read_words();
	void validate_guess(int); // window no.
	std::wstring get_solution();
	bool is_word_valid(std::wstring);

public:
	app_wordle(HINSTANCE instance);
	int run(int show_command);
	~app_wordle();
};