#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

// clang-format off
const int LEVEL_WIDTH = 8; // number of rows
const int LEVEL_HEIGTH = 6; // number of columns
const int LEVEL[] = {
  0, 0, 0, 0, 0, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 1, 0, 0,
  0, 0, 1, 0, 1, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
};
// clang-format on

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
    printf("New sprite vector allocation: %zu\n", vec->capacity);
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

void move_tiles(SpriteVector *tiles) {
  for (size_t i = 0; i <= tiles->size; i++) {
    Sprite *tile = &tiles->elements[i];
    tile->dest_rect.y += 50.0f * GetFrameTime();
  }
}

void move_player(Sprite *player) {
  // Resets the player's velocity to 0 every frame. This gives snappy start
  // and stop effect.
  player->vel.x = 0.0;

  if (IsKeyDown(KEY_L)) {
    player->vel.x = 150.0;
    player->dir = SpriteDirectionRight;
  } else if (IsKeyDown(KEY_J)) {
    player->vel.x = -150.0;
    player->dir = SpriteDirectionLeft;
  }

  // NOTE: IsKeyPressed is different than IsKeyDown.
  // It is only true when the key is pressed on the exact same time.
  if (IsKeyPressed(KEY_SPACE)) {
    player->vel.y = -400;
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

void check_collisions_y(Sprite *sprite, SpriteVector *tiles) {
  Rectangle hitbox = player_hitbox(sprite);

  for (size_t i = 0; i <= tiles->size; i++) {
    Sprite tile = tiles->elements[i];

    // player's rectangle has intersected a tile's rectangle
    if (CheckCollisionRecs(hitbox, tile.dest_rect)) {
      // reverse the overlap

      if (hitbox.y > tile.dest_rect.y) {
        // moving sprite is on bottom
        sprite->dest_rect.y = tile.dest_rect.y + tile.dest_rect.height - 8.0f;
      } else {
        // moving sprite is on top
        sprite->dest_rect.y = tile.dest_rect.y - sprite->dest_rect.height;
      }
    }
  }
}

void check_collisions_x(Sprite *sprite, SpriteVector *tiles) {
  Rectangle hitbox = player_hitbox(sprite);

  for (size_t i = 0; i <= tiles->size; i++) {
    Sprite tile = tiles->elements[i];

    // player's rectangle has intersected a tile's rectangle
    if (CheckCollisionRecs(hitbox, tile.dest_rect)) {
      // reverse the overlap

      if (hitbox.x > tile.dest_rect.x) {
        // moving sprite is on bottom
        sprite->dest_rect.x = tile.dest_rect.x + tile.dest_rect.width - 8.0f;
      } else {
        // moving sprite is on top
        sprite->dest_rect.x = tile.dest_rect.x - sprite->dest_rect.width + 8.0f;
      }
    }
  }
}

void enforce_boundaries(Sprite *player) {
  // limit user fall to the ground
  if (player->dest_rect.y > GetScreenHeight() - player->dest_rect.height) {
    player->dest_rect.y = GetScreenHeight() - player->dest_rect.height;
  }

  // do not let user move horizontally out of the window
  if (player->dest_rect.x < 0) {
    player->dest_rect.x = 0;
  } else if (player->dest_rect.x > GetScreenWidth() - player->dest_rect.width) {
    player->dest_rect.x = GetScreenWidth() - player->dest_rect.width;
  }
}

int main(void) {
  // init app
  InitWindow(600, 400, "raylib - game");
  SetTargetFPS(60);

  Texture2D player_idle_texture =
      LoadTexture("assets/herochar/herochar_idle_anim_strip_4.png");

  Texture2D tiles_texture = LoadTexture("assets/tiles_bg_fg/tileset.png");

  Sprite player = (Sprite){
      .texture = player_idle_texture,
      .dest_rect =
          (Rectangle){
              .x = 10.0,
              .y = 30.0,
              .width = 32.0,
              .height = 32.0,
          },
      .dir = SpriteDirectionRight,
  };

  SpriteVector level_tiles = load_level(tiles_texture);
  for (size_t i = 0; i <= level_tiles.size; i++) {
    Sprite tile = level_tiles.elements[i];
    printf("debug tile %zu %f %f\n", i, tile.dest_rect.x, tile.dest_rect.y);
  }

  // run app
  while (!WindowShouldClose()) {
    // update section
    move_tiles(&level_tiles);
    move_player(&player);
    apply_gravity(&player);

    // after all movement updates
    apply_vel_y(&player);
    check_collisions_y(&player, &level_tiles);
    apply_vel_x(&player);
    check_collisions_x(&player, &level_tiles);

    enforce_boundaries(&player);

    // draw section
    BeginDrawing();

    // all drawing happens
    ClearBackground(SKYBLUE);

    // DrawText("Game", 220, 20, 20, DARKGRAY);

    // draw tiles
    for (size_t i = 0; i <= level_tiles.size; i++) {
      Sprite tile = level_tiles.elements[i];
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
        player.texture, (Rectangle){0, 0, 16 * player.dir, 16},  // source
        player.dest_rect,                                        // dest
        (Vector2){0, 0},                                         // origin
        0.0,                                                     // rotation
        RAYWHITE                                                 // color
    );

    EndDrawing();

    printf("frame=%f\n", GetFrameTime());
  }

  // free memory
  UnloadTexture(player_idle_texture);
  UnloadTexture(tiles_texture);

  // close app
  CloseWindow();

  return 0;
}
