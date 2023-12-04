#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

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
    font_ = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 30);
    if (font_ == nullptr) {
      std::cout << "Failed to load font" << std::endl;
      return false;
    }
    window_ = SDL_CreateWindow("Minesweeper", w_, h_, 0);
    if (window_ == nullptr) {
      std::cout << "Failed to create window." << std::endl;
      return false;
    }
    renderer_ = SDL_CreateRenderer(window_, nullptr, 0);
    if (renderer_ == nullptr) {
      std::cout << "Failed to create renderer." << std::endl;
      return false;
    }
    if (!InitializeNumberTextures()) {
      std::cout << "Failed to initilaze number textures." << std::endl;
      return false;
    }
    DrawMineField();
    return true;
  }

  bool InitializeNumberTextures() {
    std::cout << "Initializing number textures." << std::endl;
    // Maximum number of neighbours is 8.
    std::vector<std::string> texts{"1", "2", "3", "4", "5", "6", "7", "8"};
    SDL_Color white{.r = 255, .g = 255, .b = 255, .a = SDL_ALPHA_OPAQUE};
    for (int i = 0; i < texts.size(); ++i) {
      std::cout << "Texture #" << i << std::endl;
      SDL_Surface* surface =
          TTF_RenderText_Solid(font_, texts[i].c_str(), white);
      if (surface == nullptr) {
        return false;
      }
      SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
      if (texture == nullptr) {
        return false;
      }
      SDL_DestroySurface(surface);
      number_textures_.emplace(i + 1, texture);
    }
    return true;
  }

  void CleanUp() {
    if (font_ != nullptr) {
      TTF_CloseFont(font_);
    }
    if (renderer_ != nullptr) {
      SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
      SDL_DestroyWindow(window_);
    }
  }

  void DrawMineField() {
    std::cout << "Drawing mine field" << std::endl;
    SetRendererDrawColor(renderer_, Color(0, 0, 0));
    SDL_RenderClear(renderer_);
    constexpr float cell_padding = 1;
    Minesweeper::FieldView field_view = game_.GetFieldView();
    for (int r = 0; r < field_view.size(); ++r) {
      for (int c = 0; c < field_view[r].size(); ++c) {
        SDL_FRect rect{
            .x = r * cell_size_ + cell_padding,
            .y = c * cell_size_ + cell_padding,
            .w = cell_size_ - 2 * cell_padding,
            .h = cell_size_ - 2 * cell_padding,
        };
        const Minesweeper::CellView cell_view = field_view[r][c];
        if (cell_view.type == Minesweeper::CellType::kRevealed &&
            cell_view.neighbour_mines.has_value() &&
            cell_view.neighbour_mines.value() > 0) {
          auto it = number_textures_.find(cell_view.neighbour_mines.value());
          SDL_RenderTexture(renderer_, it->second, nullptr, &rect);
        } else {
          SetRendererDrawColor(renderer_,
                               GetColorForCellView(field_view[r][c]));
          SDL_RenderFillRect(renderer_, &rect);
        }
      }
    }
    SDL_RenderPresent(renderer_);
  }

  std::optional<std::pair<int, int>> RowAndColFromXY(int x, int y) {
    int r = x / cell_size_;
    int c = y / cell_size_;
    std::pair<int, int> field_size = game_.GetFieldSize();
    if (r < 0 || r >= field_size.first || c < 0 || c >= field_size.second) {
      return std::nullopt;
    }
    std::pair<int, int> result = std::make_pair(r, c);
    return result;
  }

  void HandleLeftClick(int x, int y) {
    if (game_.IsGameOver()) {
      return;
    }
    auto row_col = RowAndColFromXY(x, y);
    if (!row_col.has_value()) {
      return;
    }
    game_.Reveal(row_col.value().first, row_col.value().second);
    DrawMineField();
  }

  void HandleRightClick(int x, int y) {
    if (game_.IsGameOver()) {
      return;
    }
    auto row_col = RowAndColFromXY(x, y);
    if (!row_col.has_value()) {
      return;
    }
    game_.ToggleFlag(row_col.value().first, row_col.value().second);
    DrawMineField();
  }

  Color GetColorForCellView(const Minesweeper::CellView& cell_view) {
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
  TTF_Font* font_ = nullptr;
  std::unordered_map<int, SDL_Texture*> number_textures_;
};

}  // namespace minesweeper

minesweeper::MinesweeperUI* minesweeper_ui = nullptr;

int SDL_AppInit(int argc, char** argv) {
  std::cout << "AppInit" << std::endl;
  int rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  if (rc != 0) {
    std::cout << "Failed to initialize SDL" << std::endl;
    return rc;
  }
  rc = TTF_Init();
  if (rc != 0) {
    std::cout << "Failed to initialize TTF" << std::endl;
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
