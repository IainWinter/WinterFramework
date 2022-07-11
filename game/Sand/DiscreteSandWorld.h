#pragma once

#include "Common.h"
#include "Rendering.h"

struct SimpleSandWorld
{
	struct Cell
	{
		//float x, y, vx, vy;
		Color color;
		bool isStatic;
	};

	int width, height;
	r<Cell[]> cells_read;
	r<Cell[]> cells_write;
	r<Texture> display;

	SimpleSandWorld()
	{
		width = 0;
		height = 0;
	}

	SimpleSandWorld(int width, int height)
		: width  (width)
		, height (height)
	{
		cells_read  = r<Cell[]>(new Cell[width * height]);
		cells_write = r<Cell[]>(new Cell[width * height]);
		display = mkr<Texture>(width, height, Texture::uRGBA, false);

		memset(cells_write.get(), 0, NumberOfBytes());
		memset(cells_read .get(), 0, NumberOfBytes());
	}

	int  NumberOfBytes ()             const { return sizeof(Cell) * width * height; }
	int  Index         (int x, int y) const { return x + y * width; }
	bool Outside       (int x, int y) const { return x < 0 || x >= width || y < 0 || y >= height; }

	const Cell& Read(int x, int y)
	{
		if (Outside(x, y)) return Cell{ Color(0, 0, 0, 255), true };

		int i = Index(x, y);
		return cells_read[i];
	}

	bool Test(int x, int y)
	{
		if (Outside(x, y)) return false;
		
		int i = Index(x, y);
		return !(cells_write[i].color.as_u32 | cells_read[i].color.as_u32);
	}

	void Write(int x, int y, const Cell& c)
	{
		Write(Index(x, y), c);
	}

	void Write(int i, const Cell& c)
	{
		cells_write[i] = c;
		if (c.isStatic) cells_read[i] = c;
	}

	void Move(int ox, int oy, int x, int y)
	{
		int oi = Index(ox, oy);
		int  i = Index(x, y);
		std::swap(cells_write[i], cells_read[oi]);
	}

	void Update()
	{
		std::swap(cells_read, cells_write);

		for (int x = 0; x < width;  x++)
		for (int y = 0; y < height; y++)
		{
			Cell cell = Read(x, y);
			if (cell.color.as_u32 == 0 || cell.isStatic) continue;

			     if (MoveDown      (x, y, cell)) {}
			else if (MoveLeftRight (x, y, cell)) {}
			else
			{
				Move(x, y, x, y);
			}
		}
	}

	void DrawToTexture()
	{
		for (int i = 0; i < width * height; i++)
		{
			display->At(i) = cells_write[i].color;
		}

		display->SendToDevice();
	}

private:
	bool MoveDown(int x, int y, const Cell& c)
	{
		bool down = Test(x, y + 1);
		if (down) Move(x, y, x, y + 1);
		return down;
	}

	bool MoveLeftRight(int x, int y, const Cell& c)
	{
		int check = get_rand(2) == 1 ? -1 : 1;
		bool lr = Test(x + check, y);
		if (lr) Move(x, y, x + check, y);
		return lr;
	}
};