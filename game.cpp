#include "framework.h"
#include "game.h"

std::wstring const game::s_class_name{ L"WORDLE - PUZZLE" };

bool game::register_class()
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
		.lpszClassName = s_class_name.c_str(),
		.hIconSm = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_ICON))
	};

	return RegisterClassExW(&desc) != 0;
}

HWND game::create_window()
{
	RECT size{ 0, 0, m_board.width, m_board.height};
	AdjustWindowRectEx(&size, game_style, false, 0);
	wnd_width = size.right - size.left;
	wnd_height = size.bottom - size.top;
	return CreateWindowExW(
		0,
		s_class_name.c_str(),
		s_class_name.c_str(),
		game_style,
		CW_USEDEFAULT,
		0,
		wnd_width,
		wnd_height,
		h_parent,
		nullptr,
		m_instance,
		this);

}

LRESULT game::window_proc_static(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	game* app = nullptr;
	if (message == WM_NCCREATE)
	{
		app = static_cast<game*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
	}
	else
		app = reinterpret_cast<game*>(GetWindowLongPtrW(window, GWLP_USERDATA));
	LRESULT res = app ? app->window_proc(window, message, wparam, lparam) :
		DefWindowProcW(window, message, wparam, lparam);
	if (message == WM_NCDESTROY)
		SetWindowLongPtrW(window, GWLP_USERDATA, 0);
	return res;
}

void game::draw_tiles(HDC hdc)
{
	HPEN pen;
	HPEN old_pen;
	HBRUSH brush;
	HBRUSH old_brush;
	for (int i = 0; i < m_board.field_count; i++)
	{
		pen = CreatePen(PS_SOLID, 2, m_board.m_fields[i].color_outline);
		old_pen = (HPEN)SelectObject(hdc, pen);
		brush = CreateSolidBrush(m_board.m_fields[i].color_fill);
		old_brush = (HBRUSH)SelectObject(hdc, brush);
		RoundRect(hdc, m_board.m_fields[i].position.left, m_board.m_fields[i].position.top, m_board.m_fields[i].position.right,
			m_board.m_fields[i].position.bottom, 8, 8);
		SelectObject(hdc, old_pen);
		DeleteObject(pen);
		SelectObject(hdc, old_brush);
		DeleteObject(brush);
	}


}

void game::draw_letter(HDC hdc)
{
	TCHAR letter[2];
	letter[1] = '\0';

	SetBkMode(hdc, TRANSPARENT);
	HFONT font = CreateFont(
		- MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72), // Height
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

	for (int i = 0; i < m_board.field_count; i++)
	{
		letter[0] = m_board.m_fields[i].c;
		DrawText(hdc, letter, 1, &m_board.m_fields[i].position,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	SelectObject(hdc, old_font);
	DeleteObject(font);

}

void game::draw_overlay(HDC hdc)
{
	RECT rect = {0};
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, 128, AC_SRC_ALPHA };
	GetClientRect(h_game, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);
	HDC temp_dc = CreateCompatibleDC(hdc);
	HBITMAP old_bitmap = (HBITMAP)SelectObject(temp_dc, bitmap);
	HBRUSH brush = won ? CreateSolidBrush(RGB(0, 255, 0)) : CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH old_brush = (HBRUSH)SelectObject(temp_dc, brush);
	FillRect(temp_dc, &rect, brush);
	AlphaBlend(hdc, 0, 0, width, height, temp_dc, 0, 0, width, height, blend);
	HDC hdcMain = GetDC(h_game);
	BitBlt(hdcMain, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);
	SelectObject(temp_dc, old_brush);
	DeleteObject(brush);
	ReleaseDC(h_game, temp_dc);
	ReleaseDC(h_game, hdcMain);
}



LRESULT game::window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			HDC hdc = GetDC(h_game);
			off_DC = CreateCompatibleDC(hdc);
			ReleaseDC(h_game, hdc);
		}
			break;
		case WM_CLOSE:
			DestroyWindow(h_game);
			return 0;
		case WM_ERASEBKGND:
			return 1;
		case WM_NCHITTEST:
			return HTCAPTION;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT rc;
			GetClientRect(h_game, &rc);
			HDC hdc = BeginPaint(h_game, &ps);
			off_bitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			off_old_bitmap = (HBITMAP)SelectObject(off_DC, off_bitmap);
			// fill background
			HBRUSH h_bckgrnd_brush = CreateSolidBrush(RGB(255, 255, 255));
			FillRect(off_DC, &rc, h_bckgrnd_brush);
			DeleteObject(h_bckgrnd_brush);
			
			draw_tiles(off_DC);
			draw_letter(off_DC);
			if(won || lost) draw_overlay(off_DC);

			BitBlt(hdc, 0, 0, rc.right, rc.bottom, off_DC, 0, 0, SRCCOPY);
			SelectObject(off_DC, off_old_bitmap);
			DeleteObject(off_bitmap);
			EndPaint(h_game, &ps);
		}
			break;
		case WM_DESTROY:
		{
			if (off_old_bitmap != NULL)
				SelectObject(off_DC, off_old_bitmap);
			if (off_DC != NULL)
				DeleteDC(off_DC);
			if (off_bitmap != NULL)
				DeleteObject(off_bitmap);
		}
		break;
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

game::game(HINSTANCE instance, HWND parent, int rows) : m_instance{ instance }, h_parent{parent}
{
	m_board = board(rows);
	register_class();
	h_game = create_window();
	ShowWindow(h_game, SW_SHOW);
}