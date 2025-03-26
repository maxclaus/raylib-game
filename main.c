#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

const int FPS_FRAMELIMIT = 60;  // 60 frames-per-second
const int WIN_WIDTH = 600;
const int WIN_HEIGHT = 400;
const int LEVEL_WIDTH = 18;   // number of rows
const int LEVEL_HEIGTH = 13;  // number of columns

// TODO: add option to play with "random" levels
// clang-format off
const int LEVEL[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
// clang-format on

typedef enum GameStatus {
  GameStatusBeginning = 1,
  GameStatusRunning = 2,
  GameStatusGameOver = 3,
} GameStatus;

typedef enum SpriteDirection {
  SpriteDirectionLeft = -1,
  SpriteDirectionRight = 1,
} SpriteDirection;

typedef struct Sprite {
  Texture2D texture;
  // x, y, width, and height of the sprite.
  Rectangle dest_rect;
  Vector2 vel;
  SpriteDirection dir;
} Sprite;

typedef struct Player {
  Sprite sprite;
  // whether this player is on top of a tile (not jumping or falling)
  bool ground;
} Player;

static const int SPRITE_VECTOR_DEFAULT_CAPACITY = 4;

typedef struct SpriteVector {
  Sprite *elements;  // address to the first element of this vector
  size_t size;       // number of elements
  size_t capacity;   // current supported capacity
} SpriteVector;

SpriteVector make_sprite_vector(void) {
  SpriteVector vec;

  vec.elements = malloc(SPRITE_VECTOR_DEFAULT_CAPACITY * sizeof(Sprite));
  vec.size = 0;
  vec.capacity = SPRITE_VECTOR_DEFAULT_CAPACITY;

  return vec;
}

void sprite_vector_dtr(SpriteVector *vec) {
  free(vec->elements);
  free(vec);
}

void sprite_vector_push_back(SpriteVector *vec, const Sprite sprite) {
  // check if vector is full
  if (vec->size > 0 && vec->size % vec->capacity == 0) {
    // increase in steps of default capacity
    vec->capacity = ((vec->size / SPRITE_VECTOR_DEFAULT_CAPACITY) + 1) *
                    SPRITE_VECTOR_DEFAULT_CAPACITY;
    vec->elements = realloc(vec->elements, vec->capacity * sizeof(Sprite));
  }
  vec->elements[vec->size++] = sprite;
}

SpriteVector load_level(Texture2D temp_texture) {
  SpriteVector sprites = make_sprite_vector();

  for (size_t i = 0; i < LEVEL_HEIGTH * LEVEL_WIDTH; i++) {
    // Converting an index to a coordinate
    // [_, _, _]
    // [_, _, _]
    // [_, o, _]
    // In this example, if index is at 'o', then its value is 7.
    // The width of level is 3, so the x component would be 7 % 3  = 1.
    // The y component would be 7 / 3 = 2.

    // Add sprite to list
    if (LEVEL[i] > 0) {
      size_t x = i % LEVEL_WIDTH;
      size_t y = i / LEVEL_WIDTH;
      float size = 32.0f;

      Sprite sprite = {
          .texture = temp_texture,
          .vel = (Vector2){.x = 0, .y = 0},
          .dest_rect = (Rectangle){
              .x = x * size,
              .y = y * size,
              .width = size,
              .height = size,
          }};
      sprite_vector_push_back(&sprites, sprite);
    }
  }

  return sprites;
}

// move tiles down the screen and update current level.
void move_tiles(SpriteVector *tiles, int *last_tile, int *level) {
  int curr_tile = -1;

  // TODO: skip tiles out of screen
  // TODO: gradually increase velocity to make the game harder over time
  for (size_t i = 0; i <= tiles->size; i++) {
    Sprite *tile = &tiles->elements[i];
    tile->vel.y += 10.0 * GetFrameTime();
    tile->dest_rect.y += tile->vel.y * GetFrameTime();

    // last tile currently at the bottom of the screen
    if (tile->dest_rect.y > WIN_HEIGHT) {
      curr_tile = i;
    }

    // ignore tiles out of screen
    if ((tile->dest_rect.y - tile->dest_rect.height) > WIN_HEIGHT) {
      break;
    }
  }

  if (*last_tile != curr_tile) {
    *last_tile = curr_tile;
    *level += 1;
  }
}

void move_player(Player *player) {
  // Resets the player's velocity to 0 every frame. This gives snappy start
  // and stop effect.
  player->sprite.vel.x = 0.0;

  if (IsKeyDown(KEY_L)) {
    player->sprite.vel.x = 150.0;
    player->sprite.dir = SpriteDirectionRight;
  } else if (IsKeyDown(KEY_J)) {
    player->sprite.vel.x = -150.0;
    player->sprite.dir = SpriteDirectionLeft;
  }

  // allow jumping only when the player is on top of a tile
  // NOTE: IsKeyPressed is different than IsKeyDown.
  // It is only true when the key is pressed on the exact same time.
  if (player->ground && IsKeyPressed(KEY_SPACE)) {
    player->sprite.vel.y = -400;
  }
}

// Creates a small rectangle out of destination rectangle.
// e.g. [px, py, 16.0, 24.0]
Rectangle player_hitbox(const Sprite *player) {
  return (Rectangle){
      .x = player->dest_rect.x + 8.0f,
      .y = player->dest_rect.y + 8.0f,
      .width = 16.0f,
      .height = 24.0f,
  };
}

void apply_gravity(Sprite *sprite) {
  sprite->vel.y += 32.0;
  if (sprite->vel.y > 600.0) {
    sprite->vel.y = 600;
  }
}

void apply_vel_x(Sprite *sprite) {
  sprite->dest_rect.x += sprite->vel.x * GetFrameTime();
}

void apply_vel_y(Sprite *sprite) {
  sprite->dest_rect.y += sprite->vel.y * GetFrameTime();
}

void check_collisions_y(Player *player, SpriteVector *tiles) {
  Rectangle hitbox = player_hitbox(&player->sprite);
  player->ground = false;

  for (size_t i = 0; i <= tiles->size; i++) {
    Sprite tile = tiles->elements[i];

    // player's rectangle has intersected a tile's rectangle
    if (CheckCollisionRecs(hitbox, tile.dest_rect)) {
      // reverse the overlap

      if (hitbox.y > tile.dest_rect.y) {
        // moving player is on bottom of a tile
        player->sprite.dest_rect.y =
            tile.dest_rect.y + tile.dest_rect.height - 8.0f;
      } else {
        // moving player is on top of a tile
        player->ground = true;
        player->sprite.dest_rect.y =
            tile.dest_rect.y - player->sprite.dest_rect.height;
      }
    }
  }
}

void check_collisions_x(Player *player, SpriteVector *tiles) {
  Rectangle hitbox = player_hitbox(&player->sprite);

  for (size_t i = 0; i <= tiles->size; i++) {
    Sprite tile = tiles->elements[i];

    // player's rectangle has intersected a tile's rectangle
    if (CheckCollisionRecs(hitbox, tile.dest_rect)) {
      // reverse the overlap

      if (hitbox.x > tile.dest_rect.x) {
        // moving sprite is on bottom
        player->sprite.dest_rect.x =
            tile.dest_rect.x + tile.dest_rect.width - 8.0f;
      } else {
        // moving sprite is on top
        player->sprite.dest_rect.x =
            tile.dest_rect.x - player->sprite.dest_rect.width + 8.0f;
      }
    }
  }
}

bool enforce_boundaries(Player *player) {
  // limit user fall to the ground
  if (player->sprite.dest_rect.y >
      GetScreenHeight() - player->sprite.dest_rect.height) {
    return false;
  }

  // do not let user move horizontally out of the window
  if (player->sprite.dest_rect.x < 0) {
    player->sprite.dest_rect.x = 0;
  } else if (
      player->sprite.dest_rect.x >
      GetScreenWidth() - player->sprite.dest_rect.width) {
    player->sprite.dest_rect.x =
        GetScreenWidth() - player->sprite.dest_rect.width;
  }

  return true;
}

char *level_status(int level) {
  char *buf = (char *)malloc(sizeof(char) * 17);
  sprintf(buf, "Got to level: %d", level);
  return buf;
}

typedef struct GameContext {
  GameStatus status;
  Player player;
  SpriteVector level_tiles;
  Texture2D tiles_texture;
  int level;
  int last_tile;
  // pause game in development to make it simpler to debug the UI
  bool paused;
  bool ready;
} GameContext;

GameContext make_game_context(
    Texture2D player_idle_texture, Texture2D tiles_texture) {
  Player player = (Player){
      .sprite =
          (Sprite){
              .texture = player_idle_texture,
              .dest_rect =
                  (Rectangle){
                      .x = 30.0,
                      .y = 30.0,
                      .width = 32.0,
                      .height = 32.0,
                  },
              .dir = SpriteDirectionRight,
          },
      .ground = false,
  };

  return (GameContext){.status = GameStatusBeginning,
                       .player = player,
                       .level_tiles = load_level(tiles_texture),
                       .tiles_texture = tiles_texture,
                       .last_tile = 0,
                       .level = 0,
                       .ready = false};
}

void reset_game_context(GameContext *ctx) {
  ctx->status = GameStatusRunning;
  ctx->player.sprite.dest_rect.x = 30.0;
  ctx->player.sprite.dest_rect.y = 30.0;
  ctx->player.sprite.vel.x = 0.0;
  ctx->player.sprite.vel.y = 0.0;
  ctx->level_tiles = load_level(ctx->tiles_texture);
  ctx->level = 0;
  ctx->last_tile = -1;
  ctx->ready = false;
}

// Draw text horizontally centter (using default font)
void DrawTextHorizontallyCenter(
    const char *text, int posY, int fontSize, Color color) {
  int posX = (WIN_WIDTH / 2) - (MeasureText(text, fontSize) / 2);
  DrawText(text, posX, posY, fontSize, color);
}

void UpdateDrawFrame(GameContext *ctx) {
#if defined(DEBUG)
  if (IsKeyPressed(KEY_P)) {
    ctx->paused = !ctx->paused;
  }
#endif

  if (!ctx->paused && (ctx->status == GameStatusBeginning ||
                       ctx->status == GameStatusGameOver)) {
    // restart game
    if (IsKeyPressed(KEY_ENTER)) {
      reset_game_context(ctx);
    }
  }

  if (!ctx->paused && ctx->status == GameStatusRunning) {
    // update section
    if (ctx->ready) {
      move_tiles(&ctx->level_tiles, &ctx->last_tile, &ctx->level);
      move_player(&ctx->player);
    }
    apply_gravity(&ctx->player.sprite);

    // after all movement updates
    apply_vel_y(&ctx->player.sprite);
    check_collisions_y(&ctx->player, &ctx->level_tiles);
    apply_vel_x(&ctx->player.sprite);
    check_collisions_x(&ctx->player, &ctx->level_tiles);

    if (!ctx->ready && ctx->player.ground) {
      // game is ready for user to start playing.
      ctx->ready = true;
    }

    if (!enforce_boundaries(&ctx->player)) {
      ctx->status = GameStatusGameOver;
    }
  }

  // draw section
  // TODO: skip re-draw if there were no changes,
  // for instance, the initial and game over screens.
  BeginDrawing();

  // all drawing happens
  ClearBackground(SKYBLUE);

  if (ctx->status == GameStatusBeginning || ctx->status == GameStatusGameOver) {
    // TODO: since the window isn't resizable we could hardcode pos_x for game
    // over text
    const char *title =
        ctx->status == GameStatusBeginning ? "Falling World" : "Game Over";
    DrawTextHorizontallyCenter(title, 20, 80, BLACK);

    if (ctx->status == GameStatusGameOver) {
      DrawTextHorizontallyCenter(level_status(ctx->level), 100, 30, BLACK);
    }

    DrawText("Press <Enter> to start, then:", 100, 160, 20, BLACK);
    DrawText("  - Press <j> to move left", 100, 200, 20, BLACK);
    DrawText("  - Press <l> to move right", 100, 240, 20, BLACK);
    DrawText("  - Press <Space> to jump", 100, 280, 20, BLACK);
    DrawText("Press <Esc> to exit", 100, 320, 20, BLACK);
  } else if (ctx->status == GameStatusRunning) {
    // draw level count info
    DrawText(level_status(ctx->level), 500, 20, 18, BLACK);

    // draw tiles
    for (size_t i = 0; i <= ctx->level_tiles.size; i++) {
      Sprite tile = ctx->level_tiles.elements[i];
      DrawTexturePro(
          tile.texture, (Rectangle){0, 0, 16, 16},  // source
          tile.dest_rect,                           // dest
          (Vector2){0, 0},                          // origin
          0.0,                                      // rotation
          RAYWHITE                                  // color
      );
    }

    // draw player
    DrawTexturePro(
        ctx->player.sprite.texture,
        (Rectangle){0, 0, 16 * ctx->player.sprite.dir, 16},  // source
        ctx->player.sprite.dest_rect,                        // dest
        (Vector2){0, 0},                                     // origin
        0.0,                                                 // rotation
        RAYWHITE                                             // color
    );
  }

  EndDrawing();
}

void ESUpdateDrawFrame(void *arg) { UpdateDrawFrame((GameContext *)arg); }

int main(void) {
  // init app
  InitWindow(WIN_WIDTH, WIN_HEIGHT, "Raylib - Game");

  Texture2D player_idle_texture =
      LoadTexture("assets/herochar/herochar_idle_anim_strip_4.png");

  Texture2D tiles_texture = LoadTexture("assets/tiles_bg_fg/tileset.png");

  GameContext ctx = make_game_context(player_idle_texture, tiles_texture);

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop_arg(ESUpdateDrawFrame, &ctx, FPS_FRAMELIMIT, 1);
#else
  SetTargetFPS(FPS_FRAMELIMIT);

  // Main game loop
  while (!WindowShouldClose()) {
    UpdateDrawFrame(&ctx);
  }
#endif

  // free memory
  UnloadTexture(player_idle_texture);
  UnloadTexture(tiles_texture);

  // close app
  CloseWindow();

  return 0;
}
