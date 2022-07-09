#pragma once

#include "Common.h"

enum class CellProperties {
	NONE                = 0b00000000,
	MOVE_DOWN           = 0b00000001,
	MOVE_DOWN_SIDE      = 0b00000010,
	MOVE_SIDE           = 0b00000100
};

inline CellProperties operator|(CellProperties a, CellProperties b) { return CellProperties(int(a) | int(b)); }
inline auto operator&(CellProperties a, CellProperties b) { return int(a) & int(b); }
 
enum class CellType {
	EMPTY,
	SAND,
	WATER,
	ROCK
};
 
struct Cell {
	CellType       Type  = CellType::EMPTY;
	CellProperties Props = CellProperties::NONE;

	Color Color; // rgba
};

struct DiscreteSandWorld
{
public:
	const size_t m_width  = 0;
	const size_t m_height = 0;
private:
	Cell* m_cells = nullptr;
	r<Texture> m_display;

public:
	DiscreteSandWorld(
		size_t width,
		size_t height
	)
		: m_width  (width)
		, m_height (height)
	{
		m_cells = new Cell[m_width * m_height];
		m_display = mkr<Texture>(m_width * m_height, Texture::uRGBA, false);
	}

	~DiscreteSandWorld() {
		delete[] m_cells;
	}

	const Cell& GetCell(size_t index)       { return m_cells[index]; }
	const Cell& GetCell(size_t x, size_t y) { return GetCell(GetIndex(x, y)); }

	size_t GetIndex(size_t x, size_t y) { return x + y * m_width; }

	bool InBounds(size_t x, size_t y) { return x < m_width && y < m_height; }
	bool IsEmpty (size_t x, size_t y) { return InBounds(x, y) && GetCell(x, y).Type == CellType::EMPTY; }

	void SetCell(
		size_t x, size_t y,
		const Cell& cell)
	{
		m_cells[GetIndex(x, y)] = cell;
	}

	std::vector<std::pair<size_t, size_t>> m_changes; // destination, source

	void MoveCell(
		size_t x,   size_t y,
		size_t xto, size_t yto)
	{
		m_changes.emplace_back(
			GetIndex(xto, yto),
			GetIndex(x,   y)
		);
	}

	void CommitCells() {
		// remove moves that have their destinations filled

		for (size_t i = 0; i < m_changes.size(); i++) {
			if (m_cells[m_changes[i].first].Type != CellType::EMPTY) {
				m_changes[i] = m_changes.back(); m_changes.pop_back();
				i--;
			}
		}

		// sort by destination

		std::sort(m_changes.begin(), m_changes.end(),
			[](auto& a, auto& b) { return a.first < b.first; }
		);

		// pick random source for each destination

		size_t iprev = 0;

		m_changes.emplace_back(-1, -1); // to catch final move

		for (size_t i = 0; i < m_changes.size() - 1; i++) {
			if (m_changes[i + 1].first != m_changes[i].first) {
				size_t rand = iprev + get_rand(int(i - iprev));

				size_t dst = m_changes[rand].first;
				size_t src = m_changes[rand].second;

				m_cells[dst] = m_cells[src];
				m_cells[src] = Cell();

				iprev = i + 1;
			}
		}

		m_changes.clear();
	}

	
	void Update()
	{
		for (size_t x = 0; x < m_width;  x++)
		for (size_t y = 0; y < m_height; y++)
		{
			const Cell& cell = GetCell(x, y);

			     if (cell.Props & CellProperties::MOVE_DOWN      && MoveDown    (x, y, cell)) {}
			else if (cell.Props & CellProperties::MOVE_DOWN_SIDE && MoveDownSide(x, y, cell)) {}
			else if (cell.Props & CellProperties::MOVE_SIDE      && MoveSide    (x, y, cell)) {}
		}

		CommitCells();

		

		// Update the sand texture
		// Draw the sand to the screen
	}

	bool MoveDown(
		size_t x, size_t y,
		const Cell& cell)
	{
		bool down = IsEmpty(x, y - 1);
		if (down) MoveCell(x, y, x, y - 1);
 
		return down;
	}

	bool MoveSide(
		size_t x, size_t y,
		const Cell& cell)
	{
		bool left  = IsEmpty(x - 1, y);
		bool right = IsEmpty(x + 1, y);

		if (left && right)
		{
			left = get_rand(1.f) > 0;
			right = !left;
		}

		     if (left)  MoveCell(x, y, x - 1, y);
		else if (right) MoveCell(x, y, x + 1, y);
 
		return left || right;
	}

	bool MoveDownSide(
		size_t x, size_t y,
		const Cell& cell)
	{
		bool downLeft  = IsEmpty(x - 1, y - 1);
		bool downRight = IsEmpty(x + 1, y - 1);

		if (downLeft && downRight)
		{
			downLeft  = get_rand(1.f) > 0;
			downRight = !downLeft;
		}

		     if (downLeft)  MoveCell(x, y, x - 1, y - 1);
		else if (downRight) MoveCell(x, y, x + 1, y - 1);
 
		return downLeft || downRight;
	}
};