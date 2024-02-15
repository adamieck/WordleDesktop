#define NOMINMAX
#include "framework.h"
#include "app_wordle.h"

#include <algorithm>

#include "game.h"
#include <windows.h>
#include <stdexcept>
#include <string>
#include <iterator>
#include <cctype>  


std::wstring const app_wordle::s_class_name{ L"WORDLE - KEYBOARD" };

bool app_wordle::register_class()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(m_instance, s_class_name.c_str(), &desc) != 0)
		return true;

	desc = {
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = window_proc_static,
		.hInstance = m_instance,
		.hIcon = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_ICON)),
		.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
		.hbrBackground = CreateSolidBrush(RGB(255,255,255)),
		.lpszMenuName = MAKEINTRESOURCE(IDR_MENU),
		.lpszClassName = s_class_name.c_str(),
		.hIconSm = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_ICON))
	};

	return RegisterClassExW(&desc) != 0;
}

HWND app_wordle::create_keywindow()
{
	return CreateWindowExW(
		0,
		s_class_name.c_str(),
		s_class_name.c_str(),
		keyboard_style,
		CW_USEDEFAULT,
		0,
		wnd_width,
		wnd_height,
		nullptr,
		h_menu,
		m_instance,
		this);

}

LRESULT app_wordle::window_proc_static(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	app_wordle* app = nullptr;
	if (message == WM_NCCREATE)
	{
		app = static_cast<app_wordle*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
	}
	else
		app = reinterpret_cast<app_wordle*>(GetWindowLongPtrW(window, GWLP_USERDATA));
	LRESULT res = app ? app->window_proc(window, message, wparam, lparam) :
		DefWindowProcW(window, message, wparam, lparam);
	if (message == WM_NCDESTROY)
		SetWindowLongPtrW(window, GWLP_USERDATA, 0);
	return res;
}

LRESULT app_wordle::window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	TCHAR c;
	switch (message)
	{
		case WM_COMMAND:
		{
			int wm_ID = LOWORD(wparam);
			switch (wm_ID)
			{
				case ID_MENU_EASY:
				{
					CheckMenuItem(h_menu, difficulty, MF_UNCHECKED);
					difficulty = ID_MENU_EASY;
					CheckMenuItem(h_menu, difficulty, MF_CHECKED);
					WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"1", L".\\Wordle.ini");
					end_game();
					start_game();
				}
					break;
				case ID_MENU_MEDIUM:
				{
					CheckMenuItem(h_menu, difficulty, MF_UNCHECKED);
					difficulty = ID_MENU_MEDIUM;
					CheckMenuItem(h_menu, difficulty, MF_CHECKED);
					WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"2", L".\\Wordle.ini");
					end_game();
					start_game();
				}
					break;
				case ID_MENU_HARD:
				{
					CheckMenuItem(h_menu, difficulty, MF_UNCHECKED);
					difficulty = ID_MENU_HARD;
					CheckMenuItem(h_menu, difficulty, MF_CHECKED);
					WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"4", L".\\Wordle.ini");
					end_game();
					start_game();
				}
					break;

			}
		}
		break;

		case WM_CHAR:
		{
			c = (TCHAR)wparam;
			if (c == '\b')
			{
				for (int i = 0; i < current_games; i++)
				{
					if (games[i]->won || games[i]->lost) continue;
					if (games[i]->current_column > 0) games[i]->current_column--;
					int cr = games[i]->current_row;
					int cc = games[i]->current_column;
					games[i]->m_board.m_fields[cr * max_columns + cc].c = L' ';
					InvalidateRect(games[i]->h_game, NULL, TRUE);
				}
			}
			else if (c == '\r')
			{
				for (int i = 0; i < current_games; i++)
				{
					if (games[i]->won || games[i]->lost) continue;
					if (games[i]->current_column == max_columns)
					{
						
						// obs³uga dobrych wyrazow
						if (games[i]->won == false)
						{
							for (int k = 0; k < max_columns; k++)
							{
								build_guess[k] = games[i]->m_board.m_fields[games[i]->current_row * max_columns + k].c;
							}
							word_guess.assign(build_guess);
							if(is_word_valid(word_guess))
								validate_guess(i);
							else
							{
								break;
							}
						}
						//break;
						
						if (games[i]->current_row == max_rows - 1) break;

						games[i]->current_row++;
						games[i]->current_column = 0;	
					}
				}
			}
			else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			{
				if (c >= 'a' && c <= 'z')
					c = c - 32;

				for (int i = 0; i < current_games; i++)
				{
					if (games[i]->won || games[i]->lost) continue;
					if (games[i]->current_column < max_columns)
					{
						int cr = games[i]->current_row;
						int cc = games[i]->current_column++;
						games[i]->m_board.m_fields[cr * max_columns + cc].c = c;
						InvalidateRect(games[i]->h_game, NULL, TRUE);
					}
				}
			}

		}
			break;

		case WM_CLOSE:
			DestroyWindow(h_keyboard);
			return 0;
		case WM_DESTROY:
			if (window == h_keyboard)
				PostQuitMessage(EXIT_SUCCESS);
			return 0;
		case WM_CREATE:
		{
			HDC hdc = GetDC(h_keyboard);
			key_DC = CreateCompatibleDC(hdc);
			ReleaseDC(h_keyboard, hdc);
		}
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT rc;
			GetClientRect(h_keyboard, &rc);
			HDC hdc = BeginPaint(h_keyboard, &ps);
			key_bitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			key_old_bitmap = (HBITMAP)SelectObject(key_DC, key_bitmap);
			HBRUSH h_bckgrnd_brush = CreateSolidBrush(RGB(255, 255, 255));
			FillRect(key_DC, &rc, h_bckgrnd_brush);
			DeleteObject(h_bckgrnd_brush);

			draw_tiles(key_DC);
			draw_letter(key_DC);

			BitBlt(hdc, 0, 0, rc.right, rc.bottom, key_DC, 0, 0, SRCCOPY);
			SelectObject(key_DC, key_old_bitmap);
			DeleteObject(key_bitmap);
			EndPaint(h_keyboard, &ps);
		}
		break;
	}
	return DefWindowProcW(window, message, wparam, lparam);
}
void app_wordle::draw_tiles(HDC hdc)
{
	HPEN pen;
	HPEN old_pen;
	HBRUSH brush;
	HBRUSH old_brush;
	for (int i = 0; i < 26; i++)
	{
		pen = CreatePen(PS_SOLID, 2, RGB(222, 225, 233));
		old_pen = (HPEN)SelectObject(hdc, pen);
		brush = CreateSolidBrush(RGB(251, 252, 255));
		old_brush = (HBRUSH)SelectObject(hdc, brush);
		RoundRect(hdc, k_board.k_fields[i].position.left, k_board.k_fields[i].position.top, k_board.k_fields[i].position.right,
			k_board.k_fields[i].position.bottom, 8, 8);

		if (k_board.k_fields[i].show)
		{
			switch (difficulty)
			{
				case ID_MENU_EASY:
				{
					pen = CreatePen(PS_SOLID, 2, k_board.k_fields[i].color_outline);
					old_pen = (HPEN)SelectObject(hdc, pen);
					brush = CreateSolidBrush(k_board.k_fields[i].color_fill);
					old_brush = (HBRUSH)SelectObject(hdc, brush);
					RoundRect(hdc, k_board.k_fields[i].position.left, k_board.k_fields[i].position.top, k_board.k_fields[i].position.right,
						k_board.k_fields[i].position.bottom, 3, 3);
				}
				break;
				case ID_MENU_MEDIUM:
				{
					for (int k = 0; k < 2; k++)
					{
						pen = CreatePen(PS_SOLID, 2, k_board.k_fields[i].medium_field[k].color_outline);
						old_pen = (HPEN)SelectObject(hdc, pen);
						brush = CreateSolidBrush(k_board.k_fields[i].medium_field[k].color_fill);
						old_brush = (HBRUSH)SelectObject(hdc, brush);
						RoundRect(hdc, k_board.k_fields[i].medium_field[k].position.left, k_board.k_fields[i].medium_field[k].position.top, 
							k_board.k_fields[i].medium_field[k].position.right,	k_board.k_fields[i].medium_field[k].position.bottom, 3, 3);
					}
				}
				break;
				case ID_MENU_HARD:
				{
					for (int k = 0; k < 4; k++)
					{
						pen = CreatePen(PS_SOLID, 2, k_board.k_fields[i].hard_field[k].color_outline);
						old_pen = (HPEN)SelectObject(hdc, pen);
						brush = CreateSolidBrush(k_board.k_fields[i].hard_field[k].color_fill);
						old_brush = (HBRUSH)SelectObject(hdc, brush);
						RoundRect(hdc, k_board.k_fields[i].hard_field[k].position.left, k_board.k_fields[i].hard_field[k].position.top,
							k_board.k_fields[i].hard_field[k].position.right, k_board.k_fields[i].hard_field[k].position.bottom, 3, 3);
					}
				}
				break;
			}
		}

		SelectObject(hdc, old_pen);
		DeleteObject(pen);
		SelectObject(hdc, old_brush);
		DeleteObject(brush);
	}


}
void app_wordle::draw_letter(HDC hdc)
{
	TCHAR letter[2];
	letter[1] = '\0';

	SetBkMode(hdc, TRANSPARENT);
	HFONT font = CreateFont(
		-MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72), // Height
		0, // Width
		0, // Escapement
		0, // Orientation
		FW_BOLD, // Weight
		false, // Italic
		FALSE, // Underline
		0, // StrikeOut
		EASTEUROPE_CHARSET, // CharSet
		OUT_DEFAULT_PRECIS, // OutPrecision
		CLIP_DEFAULT_PRECIS, // ClipPrecision
		DEFAULT_QUALITY, // Quality
		DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
		L"Helvetica"); // Facename
	HFONT old_font = (HFONT)SelectObject(hdc, font);

	for (int i = 0; i < k_board.field_count; i++)
	{
		letter[0] = k_board.k_fields[i].c;
		DrawText(hdc, letter, 1, &k_board.k_fields[i].position,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	SelectObject(hdc, old_font);
	DeleteObject(font);

}
app_wordle::app_wordle(HINSTANCE instance) : m_instance{ instance }
{
	register_class();
	h_menu = LoadMenu(m_instance, MAKEINTRESOURCE(IDR_MENU));

	current_games = GetPrivateProfileInt(L"WORDLE", L"DIFFICULTY", 1, L".\\Wordle.ini");
	switch (current_games)
	{
		case 1:
		{
			difficulty = ID_MENU_EASY;
			CheckMenuItem(h_menu, difficulty, MF_CHECKED);
		}
			break;
		case 2:
		{
			difficulty = ID_MENU_MEDIUM;
			CheckMenuItem(h_menu, difficulty, MF_CHECKED);
		}
			break;
		case 4:
		{
			difficulty = ID_MENU_HARD;
			CheckMenuItem(h_menu, difficulty, MF_CHECKED);
		}
			break;
		default:
		{
			difficulty = ID_MENU_EASY;
			CheckMenuItem(h_menu, difficulty, MF_CHECKED);
		}
			break;
	}
	h_keyboard = create_keywindow();
}

void app_wordle::validate_guess(int i)
{
	bool match = true;
	for (int k = 0; k < max_columns; k++)
	{
		int idx = letters.find(word_guess[k]);
		auto& f = games[i]->m_board.m_fields[games[i]->current_row * max_columns + k];
		if (games[i]->word_solution[k] == word_guess[k])
		{
			f.color_fill = GREEN;
			f.color_outline = GREEN;
			
			if (difficulty == ID_MENU_EASY)
			{
				k_board.k_fields[idx].color_fill = GREEN;
				k_board.k_fields[idx].color_outline = GREEN;
			}
			else if (difficulty == ID_MENU_MEDIUM)
			{
				k_board.k_fields[idx].medium_field[i].color_fill = GREEN;
				k_board.k_fields[idx].medium_field[i].color_outline = GREEN;
			}
			else if (difficulty == ID_MENU_HARD)
			{
				k_board.k_fields[idx].hard_field[i].color_fill = GREEN;
				k_board.k_fields[idx].hard_field[i].color_outline = GREEN;
			}
			
		}
		else if (games[i]->word_solution.find(word_guess[k]) != std::wstring::npos)
		{
			f.color_fill = ORANGE;
			f.color_outline = ORANGE;

			if (difficulty == ID_MENU_EASY)
			{
				k_board.k_fields[idx].color_fill = ORANGE;
				k_board.k_fields[idx].color_outline = ORANGE;
			}
			else if (difficulty == ID_MENU_MEDIUM)
			{
				k_board.k_fields[idx].medium_field[i].color_fill = ORANGE;
				k_board.k_fields[idx].medium_field[i].color_outline = ORANGE;
			}
			else if (difficulty == ID_MENU_HARD)
			{
				k_board.k_fields[idx].hard_field[i].color_fill = ORANGE;
				k_board.k_fields[idx].hard_field[i].color_outline = ORANGE;
			}
		}
		else
		{
			f.color_fill = GRAY;
			f.color_outline = GRAY;

			if (difficulty == ID_MENU_EASY)
			{
				k_board.k_fields[idx].color_fill = GRAY;
				k_board.k_fields[idx].color_outline = GRAY;
			}
			else if (difficulty == ID_MENU_MEDIUM)
			{
				k_board.k_fields[idx].medium_field[i].color_fill = GRAY;
				k_board.k_fields[idx].medium_field[i].color_outline = GRAY;
			}
			else if (difficulty == ID_MENU_HARD)
			{
				k_board.k_fields[idx].hard_field[i].color_fill = GRAY;
				k_board.k_fields[idx].hard_field[i].color_outline = GRAY;
			}
		}
		k_board.k_fields[idx].show = true;

		
		if (games[i]->word_solution[k] != word_guess[k]) match = false;
	}
	games[i]->won = match;
	if (!match && games[i]->current_row == max_rows - 1) games[i]->lost = true;
	InvalidateRect(h_keyboard, NULL, TRUE);
	InvalidateRect(games[i]->h_game, NULL, TRUE);
}
void app_wordle::read_words()
{
	file.open("Wordle.txt", std::wifstream::in);

	if (!file) exit(EXIT_FAILURE);
	std::wstring word;
	while (std::getline(file, word))
	{
		if (word.length() == 5)
		{
			std::transform(word.begin(), word.end(), word.begin(), towupper);
			words.insert(word);
		}
	}
	file.close();
}

bool app_wordle::is_word_valid(std::wstring word)
{
	return words.contains(word);
}

std::wstring app_wordle::get_solution()
{
	int idx = std::rand() % words.size();
	auto it = std::next(words.begin(), idx);
	return *it;
}

void app_wordle::init()
{
	// Prepare solution
	build_guess[max_columns] = '\0';
	read_words();


	// Prepare keyboard
	int xScreen = (GetSystemMetrics(SM_CXSCREEN));
	int yScreen = (GetSystemMetrics(SM_CYSCREEN));
	SetWindowPos(h_keyboard, HWND_TOPMOST, (xScreen - wnd_width) / 2, yScreen * 2 / 3, 0, 0, SWP_NOSIZE);
	SetWindowLong(h_keyboard, GWL_EXSTYLE, GetWindowLong(h_keyboard, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(h_keyboard, RGB(255, 255, 255), (255 * 50) / 100, LWA_ALPHA);
	ShowWindow(h_keyboard, SW_SHOW);
}

void app_wordle::end_game()
{
	for (int i = 0; i < current_games; i++)
	{
		DestroyWindow(games[i]->h_game);
		delete games[i];
	}
}

void app_wordle::start_game()
{
	int xScreen = (GetSystemMetrics(SM_CXSCREEN));
	int yScreen = (GetSystemMetrics(SM_CYSCREEN));
	switch (difficulty)
	{
		case ID_MENU_EASY:
		{
			current_games = 1;
			max_rows = 6;
			for (int i = 0; i < current_games; i++)
			{ 
				games[i] = new game(m_instance, h_keyboard, max_rows);
				games[i]->word_solution = get_solution();
			}
			MoveWindow(games[0]->h_game, (xScreen - games[0]->wnd_width) / 2, (yScreen - games[0]->wnd_height) / 2, games[0]->wnd_width, games[0]->wnd_height, true);
		}
			break;
		case ID_MENU_MEDIUM:
		{
			current_games = 2;
			max_rows = 8;
			for (int i = 0; i < current_games; i++)
			{
				games[i] = new game(m_instance, h_keyboard, max_rows);
				games[i]->word_solution = get_solution();
			}
			MoveWindow(games[0]->h_game, xScreen / 3 - games[0]->wnd_width / 2, (yScreen - games[0]->wnd_height) / 2, games[0]->wnd_width, games[0]->wnd_height, true);
			MoveWindow(games[1]->h_game, 2 * xScreen / 3 - games[1]->wnd_width / 2, (yScreen - games[1]->wnd_height) / 2, games[1]->wnd_width, games[1]->wnd_height, true);
		}
			break;
		case ID_MENU_HARD:
		{
			current_games = 4;
			max_rows = 10;
			for (int i = 0; i < current_games; i++)
			{
				games[i] = new game(m_instance, h_keyboard, max_rows);
				games[i]->word_solution = get_solution();
			}
			MoveWindow(games[0]->h_game, (xScreen / 2 - games[0]->wnd_width) / 2, (yScreen / 2 - games[0]->wnd_height) / 2, games[0]->wnd_width, games[0]->wnd_height, true);
			MoveWindow(games[1]->h_game, (3 * xScreen / 2 - games[1]->wnd_width) / 2, (yScreen / 2 - games[1]->wnd_height) / 2, games[1]->wnd_width, games[1]->wnd_height, true);
			MoveWindow(games[2]->h_game, (xScreen / 2 - games[2]->wnd_width) / 2, (3 * yScreen / 2 - games[2]->wnd_height) / 2, games[2]->wnd_width, games[2]->wnd_height, true);
			MoveWindow(games[3]->h_game, (3 * xScreen / 2 - games[3]->wnd_width) / 2, (3 *yScreen / 2 - games[3]->wnd_height) / 2, games[3]->wnd_width, games[3]->wnd_height, true);
		}
			break;
	}

	for (int i = 0; i < 26; i++)
		k_board.k_fields[i].show = false;

	InvalidateRect(h_keyboard, NULL, TRUE);
}
int app_wordle::run(int show_command)
{

	init();
	start_game();
	

	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1)
			return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return EXIT_SUCCESS;

}

app_wordle::~app_wordle()
{
	end_game();
}


int WINAPI wWinMain(HINSTANCE instance,
	HINSTANCE /*prevInstance*/,
	LPWSTR /*command_line*/,
	int show_command)
{
	srand(time(nullptr));
	app_wordle app{ instance };
	return app.run(show_command);
}