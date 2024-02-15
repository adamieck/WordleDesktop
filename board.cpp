#include "framework.h"
#include "board.h"

board::board(int rows): rows{rows}
{
	field_count = rows * columns;
	width = columns * (field_size + margin) + margin;
	height = rows * (field_size + margin) + margin;

	m_fields = new field[field_count];

	for (LONG row = 0; row < rows; ++row)
		for (LONG column = 0; column < columns; ++column)
		{
			auto & f = m_fields[row * columns + column];
			f.position.top =
				row * (field_size + margin) + margin;
			f.position.left =
				column * (field_size + margin) + margin;
			f.position.bottom = f.position.top + field_size;
			f.position.right = f.position.left + field_size;
		}
}

//board::~board()
//{
//	delete[] m_fields;
//}

board_key::board_key()
{
	k_fields = new key_field[field_count];

	for (int i = 0; i < 10; i++)
	{
		auto& f = k_fields[i];
		f.position.top = margin;
		f.position.left = i * (field_size + margin) + margin;
		f.position.bottom = f.position.top + field_size;
		f.position.right = f.position.left + field_size;
		f.c = letters[i];
	}
	for (int i = 10; i < 19; i++)
	{
		auto& f = k_fields[i];
		f.position.top = 2 * margin + field_size;
		f.position.left = (i - 10) * (field_size + margin) + (margin + field_size / 2);
		f.position.bottom = f.position.top + field_size;
		f.position.right = f.position.left + field_size;
		f.c = letters[i];
	}
	for (int i = 19; i < field_count; i++)
	{
		auto& f = k_fields[i];
		f.position.top = 2 * (margin + field_size) + margin;
		f.position.left = (i - 19) * (field_size + margin) + (2 * margin + field_size * 3 / 2);
		f.position.bottom = f.position.top + field_size;
		f.position.right = f.position.left + field_size;
		f.c = letters[i];
	}

	for (int i = 0; i < field_count; i++)
	{
		auto& f = k_fields[i];
		f.medium_field[0].position.top = f.position.top;
		f.medium_field[0].position.left = f.position.left;
		f.medium_field[0].position.bottom = f.position.top + field_size;
		f.medium_field[0].position.right = f.position.left + field_size / 2;

		f.medium_field[1].position.top = f.position.top;
		f.medium_field[1].position.left = f.position.left + field_size / 2;
		f.medium_field[1].position.bottom = f.position.top + field_size;
		f.medium_field[1].position.right = f.position.left + field_size;

		for(int row = 0; row < 2; row++)
			for (int col = 0 ; col < 2; col++)
			{
				auto& h = f.hard_field[row * 2 + col];
				h.position.top = f.position.top + row * field_size / 2;
				h.position.left = f.position.left + col * field_size / 2;
				h.position.bottom = h.position.top + field_size / 2;
				h.position.right = h.position.left + field_size / 2;
			}
	}
}