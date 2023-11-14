#ifndef INCLUDED_MINESWEEPER_GAME_H_
#define INCLUDED_MINESWEEPER_GAME_H_

#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace minesweeper {

class Minesweeper {
 public:
  enum class CellType { kHidden = 0, kRevealed, kFlagged, kMined };
  struct CellView {
    CellType type = CellType::kHidden;
    std::optional<int> neighbour_mines;
  };
  using FieldView = std::vector<std::vector<CellView>>;

  static Minesweeper Create(int rows, int cols, int mines);

  static Minesweeper CreateForTests(std::vector<std::vector<bool>> mines);

  // Reveals the field from the cell in position (row,col).
  void Reveal(int row, int col);

  // Toggles the flag on the cell in position (row, col).
  // Returns true if the cell was flaggable.
  void ToggleFlag(int row, int col);

  // Returns the size of the mine field (rows, cols).
  std::pair<int, int> GetFieldSize() const;

  // Returns a view of the current mine field.
  FieldView GetFieldView() const;

  // Returns whether the game is over.
  bool IsGameOver() const;

 private:
  struct Cell {
    int neighbour_mines = 0;
    bool mined = false;
    bool flagged = false;
    bool revealed = false;
  };
  using Cells = std::vector<std::vector<Cell>>;

  explicit Minesweeper(Cells cells) : cells_(std::move(cells)){};

  // Game over.
  void GameOver();

  // Populate the neighbour_mines attribute of each Cell in the cells_
  // attribute.
  void PopulateNeighbourMines();

  // Count the number of mines adjacent to the cell in position (row, col).
  int CountNeighbourMines(int row, int col);

  // Reveals the neighbour cells  from the cell in position (row,col).
  // If is_origin is set to true, will skip the (row,col) cell and reveal the
  // neighbours.
  void RevealRec(int row, int col, bool is_origin);

  // Returns the CellView object for a given Cell.
  CellView GetCellView(const Cell& cell) const;

  Cells cells_;
  bool game_over_ = false;
};

}  // namespace minesweeper

#endif