#include "game.h"

#include <gtest/gtest.h>

namespace minesweeper {
namespace {

using ::testing::Eq;

TEST(Game, CreateReturns) { auto m = Minesweeper::Create(5, 5, 3); }

TEST(Game, CreateForTests) {
  auto m = Minesweeper::CreateForTests({
      {false, false, false, true},  //
      {false, true, false, true},   //
      {false, false, true, true},   //
      {false, false, false, true},  //
  });
}

TEST(Game, Reveal) {
  auto m = Minesweeper::CreateForTests({
      {false, true, false, false},   //
      {true, true, false, false},    //
      {false, false, false, false},  //
      {false, false, false, false},  //
  });

  EXPECT_FALSE(m.Reveal(0, 0));

  Minesweeper::FieldView field_view = m.GetFieldView();
  EXPECT_EQ(field_view[0][0].type, Minesweeper::CellType::kRevealed);
  EXPECT_EQ(field_view[0][0].neighbour_mines, 3);
  EXPECT_EQ(field_view[0][1].type, Minesweeper::CellType::kHidden);
  EXPECT_EQ(field_view[1][0].type, Minesweeper::CellType::kHidden);
  EXPECT_EQ(field_view[1][1].type, Minesweeper::CellType::kHidden);
}

}  // namespace
}  // namespace minesweeper
