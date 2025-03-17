#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define VEC_IMPLEMENTATION
#include "vec.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>

#define SDL_EXTRA_IMPLEMENTATION
#include "sdl_extra.h"

#define GL_EXTRA_IMPLEMENTATION
#include "gl_extra.h"

#define EDITOR_IMPLEMENTATION
#include "editor.h"

#define GLYPH_IMPLEMENTATION
#include "glyph.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 600
#define DELTA_TIME (1.0f / FPS)

#define FONT_WIDTH 128.f
#define FONT_HEIGHT 64.f
#define FONT_ROWS 7
#define FONT_COLS 18
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

typedef struct {
  SDL_Texture *spritesheet;
  SDL_Rect glyph_table[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1];
} Font;

Font font_load_from_file(SDL_Renderer *renderer, const char *filepath) {
  Font font = {0};
  SDL_Surface *font_surface = surface_from_file(filepath);
  scc(SDL_SetColorKey(font_surface, SDL_TRUE, 0xFF000000));
  font.spritesheet = scp(SDL_CreateTextureFromSurface(renderer, font_surface));
  SDL_FreeSurface(font_surface);
  for (size_t ascii = ASCII_DISPLAY_LOW; ascii <= ASCII_DISPLAY_HIGH; ++ascii) {
    const size_t idx = ascii - ASCII_DISPLAY_LOW;
    const size_t col = idx % FONT_COLS;
    const size_t row = idx / FONT_COLS;
    font.glyph_table[idx] = (SDL_Rect){
        .x = col * FONT_CHAR_WIDTH,
        .y = row * FONT_CHAR_HEIGHT,
        .w = FONT_CHAR_WIDTH,
        .h = FONT_CHAR_HEIGHT,
    };
  }
  return font;
}

void sdle_render_char(SDL_Renderer *renderer, const Font *font, char chr, Vec2f pos, float scale) {
  const SDL_Rect dst = {
      .x = (int)floorf(pos.x),
      .y = (int)floorf(pos.y),
      .w = (int)floorf(FONT_CHAR_WIDTH * scale),
      .h = (int)floorf(FONT_CHAR_HEIGHT * scale),
  };
  assert(chr >= ASCII_DISPLAY_LOW);
  assert(chr <= ASCII_DISPLAY_HIGH);
  const size_t idx = chr - ASCII_DISPLAY_LOW;
  scc(SDL_RenderCopy(renderer, font->spritesheet, &font->glyph_table[idx],
                     &dst));
}

void sdle_render_text_sized(SDL_Renderer *renderer, const Font *font,
                       const char *text, size_t text_size, Vec2f pos,
                       Uint32 color, float scale) {
  Vec2f pen = {0};
  vec2f_make(&pen, pos.x, pos.y);
  set_texture_color(font->spritesheet, color);
  for (size_t i = 0; i < text_size; ++i) {
    sdle_render_char(renderer, font, text[i], pen, scale);
    pen.x += FONT_CHAR_WIDTH * scale;
  }
}

void sdle_render_text(
    SDL_Renderer *renderer, 
    const Font *font,
    const char *text,
    Vec2f pos,
    Uint32 color,
    float scale
) {
  sdle_render_text_sized(renderer, font, text, strlen(text), pos, color, scale);
}

#define FONT_SCALE 5.f

Editor editor = {0};
Vec2f camera_pos = {0};
Vec2f camera_vel = {0};

#define UNHEX(color)                                                           \
  ((color) >> (8 * 0)) & 0xFF, ((color) >> (8 * 1)) & 0xFF,                    \
      ((color) >> (8 * 2)) & 0xFF, ((color) >> (8 * 3)) & 0xFF

void render_cursor(SDL_Renderer *renderer, const Font *font) {
  Vec2f pos = {
      .x = (int)floorf(editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE),
      .y = (int)floorf(editor.cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE),
  };
  vec2f_sub(&pos, camera_pos);
  const SDL_Rect rect = {
      .x = (int)floorf(pos.x),
      .y = (int)floorf(pos.y),
      .w = FONT_CHAR_WIDTH * FONT_SCALE,
      .h = FONT_CHAR_HEIGHT * FONT_SCALE,
  };

  scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
  scc(SDL_RenderFillRect(renderer, &rect));

  const char *c = editor_char_under_cursor(&editor);
  if (c) {
    set_texture_color(font->spritesheet, 0xFF000000);
    sdle_render_char(renderer, font, *c, pos, FONT_SCALE);
  }
}

char *argv_shift(int *argc, char ***argv) {
  char *out = **argv;
  (*argc) -= 1;
  (*argv) += 1;
  return out;
}

Vec2f window_size(SDL_Window *window) {
  int window_width, window_height;
  SDL_GetWindowSize(window, &window_width, &window_height);
  return (Vec2f){.x = (float)window_width, .y = (float)window_height};
}

void camera_project_point(SDL_Window *window, Vec2f point) {
  Vec2f cp = {0};
  Vec2f ws = window_size(window);
  vec2f_add(&cp, camera_pos);
  vec2f_div(&ws, (Vec2f){.x = 2, .y = 2});
  vec2f_add(&cp, ws);
  vec2f_sub(&cp, point);
  vec2f_sub(&camera_vel, cp);
  vec2f_mul(&camera_vel, (Vec2f){.x = DELTA_TIME, .y = DELTA_TIME});
  vec2f_mul(&camera_vel, (Vec2f){.x = 2, .y = 2});
  vec2f_add(&camera_pos, camera_vel);
}

//#define OPENGL_RENDERER


void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                     GLsizei length, const GLchar *message,
                     const void *userParam) {
  (void)source, (void)id, (void)length, (void)userParam;

  fprintf(stderr,
          "GL_CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL_ERROR **" : ""), type, severity,
          message);
}

#ifdef OPENGL_RENDERER
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    scc(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
        scp(SDL_CreateWindow("broadnic", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));

    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        int major, minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        printf("GL version: %d.%d\n", major, minor);
    }

    scp(SDL_GL_CreateContext(window));

    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "[ERROR] Could not initialize GLEW: %s\n", strerror(errno));
        exit(1);
    }

    if (!GLEW_ARB_draw_instanced) {
        fprintf(stderr, "[ERROR] `ARB_draw_instanced` is not supported! Program may not work properly.\n");
        exit(1);
    }

    if (!GLEW_ARB_instanced_arrays) {
        fprintf(stderr, "[ERROR] `ARB_instanced_arrays` is not supported! Program may not work properly.\n");
        exit(1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    } else {
        fprintf(stderr, "[WARNING] `GLEW_ARB_debug_output` is not available");
    }

    GLint time_uniform;
    GLint resolution_uniform;
    // Shader initialization
    {
        GLuint vert_shader = 0;
        if (!compile_shader_file("./shaders/font.vert", GL_VERTEX_SHADER,
                                 &vert_shader)) {
            exit(1);
        }
        GLuint frag_shader = 0;
        if (!compile_shader_file("./shaders/font.frag", GL_FRAGMENT_SHADER,
                                 &frag_shader)) {
            exit(1);
        }
        GLuint program = 0;
        if (!link_program(vert_shader, frag_shader, &program)) {
            exit(1);
        }
        glUseProgram(program);
        time_uniform = glGetUniformLocation(program, "time");
        resolution_uniform = glGetUniformLocation(program, "resolution");
        glUniform2f(resolution_uniform, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // Font Texture initialization
    {
        const char *file_path = "charmap-oldschool_white.png";
        int width, height, n;
        unsigned char *pixels =
            stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);
        if (pixels == NULL) {
            fprintf(stderr, "ERROR: could not load %s: %s\n", file_path,
                    stbi_failure_reason());
            exit(1);
        }
        glActiveTexture(GL_TEXTURE0);
        GLuint font_texture = 0;
        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, pixels);
    }

    // Buffers initialization
    {
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glyph_buffer), glyph_buffer, GL_DYNAMIC_DRAW);

        for (Glyph_Attr attr=0; attr < COUNT_GLYPH_ATTRS; ++attr) {
            glEnableVertexAttribArray(attr);
            glVertexAttribPointer(
                attr,
                glyph_attr_defs[attr].comps,
                GL_FLOAT,
                GL_FALSE,
                sizeof(Glyph),
                (void*) glyph_attr_defs[attr].offset
            );
            glVertexAttribDivisor(attr, 1);
        }
    }

    const char *text = "Hello, World";
    Vec2f text_pos = {.x=0,.y=0};
    float text_scale = 5.f;
    Vec4f text_color = {.x=0.0f,.y=1.0f,.z=0.0f,.t=1.0f};

    ogl_render_text(text, strlen(text), text_pos, text_scale, text_color);
    glyph_buffer_sync();

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    quit = true;
                } break;
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE: {
                            quit = true;
                        } break;
                    }
                } break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1f(time_uniform, (float)SDL_GetTicks()/1000.0f);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyph_buffer_count);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
#else
int main(int argc, char **argv) {
  argv_shift(&argc, &argv);
  char *loaded_file_path = NULL;

  if (argc > 0) {
    loaded_file_path = argv_shift(&argc, &argv);
    printf("`%s` loaded\n", loaded_file_path);
  }

  if (loaded_file_path) {
    FILE *file = fopen(loaded_file_path, "r");
    if (file != NULL) {
      editor_load_from_file(&editor, file);
      fclose(file);
    }
  }

  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window = scp(SDL_CreateWindow(
      "broadnic", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE));
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  Font font = font_load_from_file(renderer, "./charmap-oldschool_white.png");

  bool quit = false;
  while (!quit) {
    const Uint32 start = SDL_GetTicks();
    SDL_Event evt = {0};
    while (SDL_PollEvent(&evt)) {
      switch (evt.type) {
      case SDL_QUIT: {
        quit = true;
      } break;
      case SDL_KEYDOWN: {
        switch (evt.key.keysym.sym) {
        case SDLK_TAB: {
          editor_insert_text_before_cursor(&editor, "  ");
        } break;
        case SDLK_BACKSPACE: {
          editor_backspace(&editor);
        } break;
        case SDLK_F2: {
          if (loaded_file_path) {
            editor_save_to_file(&editor, loaded_file_path);
          }
        } break;
        case SDLK_DELETE: {
          editor_delete(&editor);
        } break;
        case SDLK_UP: {
          if (editor.cursor_row > 0) {
            editor.cursor_row -= 1;
          }
        } break;
        case SDLK_DOWN: {
          if (editor.cursor_row < editor.size) {
            editor.cursor_row += 1;
          }
        } break;
        case SDLK_LEFT: {
          if (editor.cursor_col > 0) {
            editor.cursor_col -= 1;
          }
        } break;
        case SDLK_RIGHT: {
          if (editor.cursor_col < 80) {
            editor.cursor_col += 1;
          }
        } break;
        case SDLK_RETURN: {
          editor_insert_new_line(&editor);
        } break;
        case SDLK_ESCAPE: {
          quit = true;
        } break;
        }
      } break;
      case SDL_TEXTINPUT: {
        editor_insert_text_before_cursor(&editor, evt.text.text);
      } break;
      }
    }

    scc(SDL_SetRenderDrawColor(renderer, 8, 8, 8, 255));
    scc(SDL_RenderClear(renderer));

    for (size_t row = 0; row < editor.size; ++row) {
      const Line *line = editor.lines + row;
      Vec2f line_pos = {0};
      vec2f_make(&line_pos, 0, row * FONT_CHAR_HEIGHT * FONT_SCALE);
      vec2f_sub(&line_pos, camera_pos);
      sdle_render_text_sized(renderer, &font, line->es, line->size, line_pos,
                        0xFFFFFFFF, FONT_SCALE);
    }

    const Vec2f cursor_pos = {
        .x = (int)floorf(editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE),
        .y = (int)floorf(editor.cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE),
    };
    camera_project_point(window, cursor_pos);

    render_cursor(renderer, &font);

    SDL_RenderPresent(renderer);
    const Uint32 duration = SDL_GetTicks() - start;
    const Uint32 delta_time_ms = 1000 / FPS;
    if (duration < delta_time_ms) {
      SDL_Delay(delta_time_ms - duration);
    }
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
#endif // OPENGL_RENDERER

#define SV_IMPLEMENTATION
#include "sv.h"
