#include "random.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <iostream>
#include <vector>

static SDL_Renderer* renderer{nullptr};
static SDL_Window* window{nullptr};

namespace BoardSettings {
int rows{15};
int cols{15};

} // namespace BoardSettings

namespace WindowSetting {
int height{480};
int width{640};

} // namespace WindowSetting

namespace moveSettings {
float speed{32};

}
SDL_FRect makeFood();

struct Food {
  SDL_FRect food = makeFood();
};
SDL_FRect makeFood() {
  int randomX{Random::get(20, 620)};
  int randomY{Random::get(20, 460)};

  return SDL_FRect{static_cast<float>(randomX), static_cast<float>(randomY), 20, 20};
}

void renderFood(const Food& food) {
  SDL_SetRenderDrawColor(renderer, 250, 50, 50, 255);
  SDL_RenderFillRect(renderer, &food.food);
}
namespace DefaultRect {
SDL_FRect defaultRect{0.f, 0.f, 32.f, 32.f};

} // namespace DefaultRect

class Snake {
public:
  Snake() {
    m_snake.push_back(DefaultRect::defaultRect);
    m_head = m_snake[0];
    m_length++;

    SDL_Log("Contructed!\n");
  }
  ~Snake() = default;
  void setRect(float x, float y);

  void moveUp();
  void moveDown();
  void moveRight();
  void moveLeft();
  void move();
  void setDirection(SDL_Keycode k);

  bool handleCollisionFood(Food& f);
  bool handleCollisionEdge();
  bool handleCollisionSelf();

  void addSnakeBody();

  void renderSnake();

  int getLength() { return m_length; }

  SDL_FRect operator[](int val) { return m_snake[val]; }

private:
  int m_length{};
  std::vector<SDL_FRect> m_snake{};
  SDL_FRect m_head;
  SDL_Keycode m_direction;
};
bool Snake::handleCollisionSelf() {
  bool collided{false};
  for (size_t i = 1; i < m_snake.size(); ++i) {
    if (m_snake[0].x < m_snake[i].x + m_snake[i].w &&
        m_snake[0].x + m_snake[0].w > m_snake[i].x &&
        m_snake[0].y < m_snake[i].y + m_snake[i].h &&
        m_snake[0].y + m_snake[0].h > m_snake[i].y) {
      collided = true;
    }
  }

  return collided;
}

bool Snake::handleCollisionEdge() {
  return m_snake[0].x > 640 || m_snake[0].x < 0 ||
         m_snake[0].y < 0 || m_snake[0].y > 480;
}
void Snake::addSnakeBody() {
  SDL_FRect tail{m_snake.back()};
  SDL_FRect newRect{SDL_FRect(tail.x, tail.y, 32.f, 32.f)};
  m_length++;

  m_snake.push_back(newRect);
}

void Snake::renderSnake() {
  for (size_t i = 0; i < m_snake.size(); i++) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderFillRect(renderer, &m_snake[i]);
  }
}

void Snake::move() {

  for (int i = m_snake.size() - 1; i > 0; i--) {
    m_snake[i] = m_snake[i - 1];
  }
  switch (m_direction) {
  case SDLK_UP:
    m_snake[0].y -= 32.f;
    break;
  case SDLK_DOWN:
    m_snake[0].y += 32.f;
    break;
  case SDLK_LEFT:
    m_snake[0].x -= 32.f;
    break;
  case SDLK_RIGHT:
    m_snake[0].x += 32.f;
    break;
  }
}
bool Snake::handleCollisionFood(Food& f) {
  return m_snake[0].x < f.food.x + f.food.w &&
         m_snake[0].x + m_snake[0].w > f.food.x &&
         m_snake[0].y < f.food.y + f.food.h &&
         m_snake[0].y + m_snake[0].h > f.food.y;
}

void Snake::setDirection(SDL_Keycode k) {
  if (m_direction == SDLK_UP && k == SDLK_DOWN)
    return;
  if (m_direction == SDLK_DOWN && k == SDLK_UP)
    return;
  if (m_direction == SDLK_RIGHT && k == SDLK_LEFT)
    return;
  if (m_direction == SDLK_LEFT && k == SDLK_RIGHT)
    return;

  else {
    m_direction = k;
  }
}
void close() {
  SDL_DestroyRenderer(renderer);
  renderer = nullptr;

  SDL_DestroyWindow(window);
  window = nullptr;
}

bool init() {
  bool success{true};
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to init sub systems %s", SDL_GetError());
    success = false;
  }

  else if (!SDL_CreateWindowAndRenderer("Snake", WindowSetting::width, WindowSetting::height, 0, &window, &renderer)) {
    SDL_Log("Failed to create window %s", SDL_GetError());
    success = false;
  }
  return success;
}

int main() {

  Snake snake{};
  snake.addSnakeBody();

  Food food{};

  Uint64 lastTime = SDL_GetTicks();
  Uint64 accumulator = 0;
  Uint64 INTERVAL_MS = 250;

  if (!init()) {
    return 1;
  } else {

    bool quit{false};
    SDL_Event e{};
    SDL_zero(e);

    SDL_Keycode eventk{};

    while (quit == false) {
      if (snake.getLength() > 2 && snake.handleCollisionSelf()) {
        std::cerr << "QUIT: self\n";
        quit = true;
      }
      Uint64 currentTime = SDL_GetTicks();
      Uint64 deltaTime = currentTime - lastTime;
      lastTime = currentTime;
      accumulator += deltaTime;

      while (SDL_PollEvent(&e)) {

        if (e.type == SDL_EVENT_QUIT) {
          quit = true;
        } else if (e.key.key == SDLK_UP || e.key.key == SDLK_W) {
          eventk = SDLK_UP;
        } else if (e.key.key == SDLK_DOWN || e.key.key == SDLK_S) {
          eventk = SDLK_DOWN;
        } else if (e.key.key == SDLK_LEFT || e.key.key == SDLK_A) {
          eventk = SDLK_LEFT;
        } else if (e.key.key == SDLK_RIGHT || e.key.key == SDLK_D) {
          eventk = SDLK_RIGHT;
        }
      }

      snake.setDirection(eventk);
      if (snake.handleCollisionFood(food)) {
        snake.addSnakeBody();
        int randomX{Random::get(20, 620)};
        int randomY{Random::get(20, 460)};
        food.food.x = static_cast<float>(randomX);
        food.food.y = static_cast<float>(randomY);
        std::cout << "Food eaten length now: " << snake.getLength() << '\n';
        renderFood(food);
      }

      while (accumulator >= INTERVAL_MS) {
        if (snake.handleCollisionEdge()) {
          quit = true;
        }
        snake.move();
        accumulator -= INTERVAL_MS;
      }

      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);
      snake.renderSnake();
      renderFood(food);
      SDL_RenderPresent(renderer);
    }
  }
  close();
  return 0;
}
