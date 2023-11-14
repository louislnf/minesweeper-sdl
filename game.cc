#include "game.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <utility>
#include <vector>

namespace minesweeper {

namespace {

bool CoordinatesAreInTable(int row, int col, int max_rows, int max_cols) {
  return row >= 0 && row < max_rows && col >= 0 && col < max_cols;
}

std::vector<std::pair<int, int>> ListNeighbours(int row, int col, int max_rows,
                                                int max_cols,
                                                bool with_corners) {
  std::vector<std::pair<int, int>> neighbours{
      {row - 1, col},
      {row, col - 1},
      {row, col + 1},
      {row + 1, col},
  };
  if (with_corners) {
    neighbours.push_back(std::make_pair<int, int>(row - 1, col - 1));
    neighbours.push_back(std::make_pair<int, int>(row - 1, col + 1));
    neighbours.push_back(std::make_pair<int, int>(row + 1, col - 1));
    neighbours.push_back(std::make_pair<int, int>(row + 1, col + 1));
  }
  std::erase_if(neighbours, [&](const std::pair<int, int>& coord) {
    return !CoordinatesAreInTable(coord.first, coord.second, max_rows,
                                  max_cols);
  });
  return neighbours;
}

int RandomInt(int min, int max) {
  static bool seeded;
  if (!seeded) {
    std::srand(std::time(nullptr));
    seeded = true;
  }
  return min + (std::rand() % (max - min + 1));
}

}  // namespace

Minesweeper Minesweeper::Create(int rows, int cols, int mines) {
  Minesweeper::Cells cells;
  cells.reserve(rows);
  for (int row = 0; row < rows; ++row) {
    cells.push_back(std::vector<Cell>(cols));
  }
  while (mines > 0) {
    int row = RandomInt(0, rows - 1);
    int col = RandomInt(0, cols - 1);
    if (cells[row][col].mined) {
      continue;
    }
    cells[row][col].mined = true;
    --mines;
  }
  Minesweeper m(std::move(cells));
  m.PopulateNeighbourMines();
  return m;
}

Minesweeper Minesweeper::CreateForTests(std::vector<std::vector<bool>> mines) {
  Minesweeper::Cells cells;
  cells.reserve(mines.size());
  for (int row = 0; row < mines.size(); ++row) {
    cells.push_back(std::vector<Cell>(mines[row].size()));
    for (int col = 0; col < mines[row].size(); ++col) {
      if (mines[row][col]) {
        cells[row][col] = Cell{.mined = true};
      }
    }
  }
  Minesweeper m(std::move(cells));
  m.PopulateNeighbourMines();
  return m;
}

void Minesweeper::GameOver() {
  game_over_ = true;
  // Reveal all mines.
  for (std::vector<Cell>& row : cells_) {
    for (Cell& cell : row) {
      if (cell.mined) {
        cell.revealed = true;
      }
    }
  }
}

void Minesweeper::Reveal(int row, int col) {
  if (IsGameOver()) {
    return;
  }
  cells_[row][col].revealed = true;
  if (cells_[row][col].mined) {
    GameOver();
    return;
  }
  if (cells_[row][col].neighbour_mines > 0) {
    return;
  }
  RevealRec(row, col, true);
}

void Minesweeper::RevealRec(int row, int col, bool is_origin = false) {
  Cell& cell = cells_[row][col];
  if (!is_origin && (cell.revealed || cell.mined || cell.flagged)) {
    return;
  }
  cell.revealed = true;
  if (cell.neighbour_mines > 0) {
    return;
  }
  std::vector<std::pair<int, int>> unrevealed_neighbours = ListNeighbours(
      row, col, cells_.size(), cells_[0].size(), /*with_corners=*/false);
  std::erase_if(unrevealed_neighbours,
                [this](const std::pair<int, int>& coord) {
                  return cells_[coord.first][coord.second].revealed;
                });
  for (const std::pair<int, int>& neighbour : unrevealed_neighbours) {
    RevealRec(neighbour.first, neighbour.second);
  }
}

void Minesweeper::ToggleFlag(int row, int col) {
  if (IsGameOver()) {
    return;
  }
  if (cells_[row][col].revealed) {
    return;
  }
  cells_[row][col].flagged = !cells_[row][col].flagged;
}

bool Minesweeper::IsGameOver() const { return game_over_; }

std::pair<int, int> Minesweeper::GetFieldSize() const {
  return std::make_pair<int, int>(cells_.size(), cells_[0].size());
}

Minesweeper::FieldView Minesweeper::GetFieldView() const {
  const auto [rows, cols] = GetFieldSize();
  FieldView field;
  field.reserve(rows);
  for (const std::vector<Cell>& cells_row : cells_) {
    field.push_back({});
    field.back().reserve(cells_row.size());
    for (const Cell& cell : cells_row) {
      field.back().push_back(GetCellView(cell));
    }
  }
  return field;
}

void Minesweeper::PopulateNeighbourMines() {
  for (int r = 0; r < cells_.size(); ++r) {
    for (int c = 0; c < cells_[r].size(); ++c) {
      cells_[r][c].neighbour_mines = CountNeighbourMines(r, c);
    }
  }
}

int Minesweeper::CountNeighbourMines(int row, int col) {
  const int max_rows = cells_.size();
  const int max_cols = cells_.front().size();
  const std::vector<std::pair<int, int>> neighbours =
      ListNeighbours(row, col, max_rows, max_cols, /*with_corners=*/true);
  return std::count_if(neighbours.begin(), neighbours.end(),
                       [this](const std::pair<int, int>& coord) {
                         return cells_[coord.first][coord.second].mined;
                       });
}

Minesweeper::CellView Minesweeper::GetCellView(const Cell& cell) const {
  CellView cell_view;
  if (cell.revealed) {
    if (cell.mined) {
      cell_view.type = CellType::kMined;
    } else {
      cell_view.type = CellType::kRevealed;
      if (cell.neighbour_mines > 0) {
        cell_view.neighbour_mines = cell.neighbour_mines;
      }
    }
  } else if (cell.flagged) {
    cell_view.type = CellType::kFlagged;
  }
  return cell_view;
}

}  // namespace minesweeper