#include <iostream>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include "game.h"

namespace minesweeper {

struct Color {
  int r;
  int g;
  int b;

  Color(int r, int g, int b) : r(r), g(g), b(b){};
};

void SetRendererDrawColor(SDL_Renderer* renderer, Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
}

class MinesweeperUI {
 public:
  MinesweeperUI(int w, int h, int cell_size)
      : w_(w),
        h_(h),
        cell_size_(cell_size),
        game_(Minesweeper::Create(w / cell_size, h / cell_size,
                                  3 * h / cell_size)) {}

  bool Initialize() {
    window_ = SDL_CreateWindow("Minesweeper", w_, h_, 0);
    if (window_ == nullptr) {
      std::cout << "Failed to create window.";
      return false;
    }
    renderer_ = SDL_CreateRenderer(window_, nullptr, 0);
    if (renderer_ == nullptr) {
      std::cout << "Failed to create renderer.";
      return false;
    }
    DrawMineField();
    return true;
  }

  void CleanUp() {
    if (renderer_ != nullptr) {
      SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
      SDL_DestroyWindow(window_);
    }
  }

  void DrawMineField() {
    SetRendererDrawColor(renderer_, Color(0, 0, 0));
    SDL_RenderClear(renderer_);
    constexpr int cell_padding = 1;
    Minesweeper::FieldView field_view = game_.GetFieldView();
    for (int r = 0; r < field_view.size(); ++r) {
      for (int c = 0; c < field_view[r].size(); ++c) {
        SDL_FRect rect{
            .x = r * cell_size_ + cell_padding,
            .y = c * cell_size_ + cell_padding,
            .w = cell_size_ - 2 * cell_padding,
            .h = cell_size_ - 2 * cell_padding,
        };
        SetRendererDrawColor(renderer_, GetColorForCellView(field_view[r][c]));
        SDL_RenderFillRect(renderer_, &rect);
      }
    }
    SDL_RenderPresent(renderer_);
  }

  void HandleLeftClick(int x, int y) {
    if (game_.IsGameOver()) {
      return;
    }
    int r = x / cell_size_;
    int c = y / cell_size_;
    game_.Reveal(r, c);
    DrawMineField();
  }

  void HandleRightClick(int x, int y) {
    if (game_.IsGameOver()) {
      return;
    }
    int r = x / cell_size_;
    int c = y / cell_size_;
    game_.ToggleFlag(r, c);
    DrawMineField();
  }

  Color GetColorForCellView(Minesweeper::CellView cell_view) {
    switch (cell_view.type) {
      case Minesweeper::CellType::kHidden:
        // Light grey.
        return Color(200, 200, 200);
      case Minesweeper::CellType::kRevealed: {
        int neighbours = cell_view.neighbour_mines.has_value()
                             ? cell_view.neighbour_mines.value()
                             : 0;
        float coeff = 1.0 - neighbours / 8.0;
        // White but gets bluer as there are more neighbours.
        return Color(255 * coeff, 255 * coeff, 255);
      }
      case Minesweeper::CellType::kFlagged:
        // Yellow.
        return Color(255, 255, 0);
      case Minesweeper::CellType::kMined:
        // Red.
        return Color(255, 0, 0);
    }
    return Color(0, 0, 0);
  }

 private:
  int w_;
  int h_;
  int cell_size_;
  Minesweeper game_;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
};

}  // namespace minesweeper

minesweeper::MinesweeperUI* minesweeper_ui = nullptr;

int SDL_AppInit(int argc, char** argv) {
  std::cout << "AppInit" << std::endl;
  int rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  if (rc != 0) {
    return rc;
  }
  minesweeper_ui = new minesweeper::MinesweeperUI(800, 600, 15);
  if (!minesweeper_ui->Initialize()) {
    return 1;
  }
  return 0;
}

int SDL_AppIterate(void) { return 0; }

int SDL_AppEvent(const SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return 1;
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        minesweeper_ui->HandleLeftClick(event->button.x, event->button.y);
      } else if (event->button.button == SDL_BUTTON_RIGHT) {
        minesweeper_ui->HandleRightClick(event->button.x, event->button.y);
      }
    } break;
    default:
      break;
  }
  return 0;
}

void SDL_AppQuit(void) {
  std::cout << "AppQuit" << std::endl;
  delete minesweeper_ui;
}
