
#include <SDL2/SDL_stdinc.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define VEC_IMPLEMENTATION
#include "vec.h"

#define EDITOR_IMPLEMENTATION
#include "editor.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>

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

void scc(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

void *scp(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

SDL_Surface* surface_from_file(const char *filepath)
{
    int width,height,n;
    unsigned char *pixels = stbi_load(filepath, &width, &height, &n, STBI_rgb_alpha);
    if (pixels == NULL) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", filepath, stbi_failure_reason());
        exit(1);
    }
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const Uint32 int shift = (req_format == STBI_rgb) ? 8 : 0;
    const Uint32 rmask = 0xff000000 >> shift;
    const Uint32 gmask = 0x00ff0000 >> shift;
    const Uint32 bmask = 0x0000ff00 >> shift;
    const Uint32 amask = 0x000000ff >> shift;
#else // little endian
    const Uint32 rmask = 0xff000000;
    const Uint32 gmask = 0x00ff0000;
    const Uint32 bmask = 0x0000ff00;
    const Uint32 amask = 0x000000ff;
#endif
    const int depth = 32;
    const int pitch = 4*width;
    return scp(
        SDL_CreateRGBSurfaceFrom((void*)pixels, width, height, depth, pitch, rmask, gmask, bmask, amask)
    );
}

void set_texture_color(SDL_Texture *texture, Uint32 color)
{
    scc(SDL_SetTextureColorMod(
        texture, 
        (color>>(8*0))&0xff,
        (color>>(8*1))&0xff,
        (color>>(8*2))&0xff
    ));
    scc(SDL_SetTextureAlphaMod(texture, (color>>(8*3))&0xff));
}

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

typedef struct {
    SDL_Texture *spritesheet;
    SDL_Rect glyph_table[ASCII_DISPLAY_HIGH-ASCII_DISPLAY_LOW+1];
} Font;

Font font_load_from_file(SDL_Renderer *renderer, const char *filepath)
{
    Font font = {0};
    SDL_Surface *font_surface = surface_from_file(filepath);
    scc(SDL_SetColorKey(font_surface, SDL_TRUE, 0xFF000000));
    font.spritesheet = scp(SDL_CreateTextureFromSurface(renderer, font_surface));
    SDL_FreeSurface(font_surface);
    for (size_t ascii=ASCII_DISPLAY_LOW;ascii<=ASCII_DISPLAY_HIGH;++ascii) {
        const size_t idx = ascii-ASCII_DISPLAY_LOW;
        const size_t col = idx % FONT_COLS;
        const size_t row = idx / FONT_COLS;
        font.glyph_table[idx] = (SDL_Rect) {
            .x = col*FONT_CHAR_WIDTH,
            .y = row*FONT_CHAR_HEIGHT,
            .w = FONT_CHAR_WIDTH,
            .h = FONT_CHAR_HEIGHT,
        };
    }
    return font;
}

void render_char(SDL_Renderer *renderer, const Font *font, char chr, Vec2f pos, float scale)
{
    const SDL_Rect dst = {
        .x = (int) floorf(pos.x),
        .y = (int) floorf(pos.y),
        .w = (int) floorf(FONT_CHAR_WIDTH * scale),
        .h = (int) floorf(FONT_CHAR_HEIGHT * scale),
    };
    assert(chr >= ASCII_DISPLAY_LOW);
    assert(chr <= ASCII_DISPLAY_HIGH);
    const size_t idx = chr-ASCII_DISPLAY_LOW;
    scc(SDL_RenderCopy(renderer, font->spritesheet, &font->glyph_table[idx], &dst));
}

void render_text_sized(SDL_Renderer *renderer, const Font *font, const char *text, size_t text_size, Vec2f pos, Uint32 color, float scale)
{
    Vec2f pen = {0};
    vec2f_make(&pen, pos.x, pos.y);
    set_texture_color(font->spritesheet, color);
    for (size_t i=0;i<text_size;++i) {
        render_char(renderer, font, text[i], pen, scale);
        pen.x += FONT_CHAR_WIDTH*scale;
    }
}

void render_text(SDL_Renderer *renderer, const Font *font, const char *text, Vec2f pos, Uint32 color, float scale)
{
    render_text_sized(renderer, font, text, strlen(text), pos, color, scale);
}

#define FONT_SCALE 5.f

Editor editor = {0};
Vec2f camera_pos = {0};
Vec2f camera_vel = {0};

#define UNHEX(color) \
        ((color)>>(8*0))&0xFF, \
        ((color)>>(8*1))&0xFF, \
        ((color)>>(8*2))&0xFF, \
        ((color)>>(8*3))&0xFF

void render_cursor(SDL_Renderer *renderer, const Font *font)
{
    Vec2f pos = {
        .x = (int) floorf(editor.cursor_col * FONT_CHAR_WIDTH*FONT_SCALE),
        .y = (int) floorf(editor.cursor_row * FONT_CHAR_HEIGHT*FONT_SCALE),
    };
    vec2f_sub(&pos, camera_pos);
    const SDL_Rect rect = {
        .x = (int)floorf(pos.x),
        .y = (int)floorf(pos.y),
        .w = FONT_CHAR_WIDTH*FONT_SCALE,
        .h = FONT_CHAR_HEIGHT*FONT_SCALE,
    };

    scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
    scc(SDL_RenderFillRect(renderer, &rect));

    const char *c = editor_char_under_cursor(&editor);
    if (c) {
        set_texture_color(font->spritesheet, 0xFF000000);
        render_char(renderer, font, *c, pos, FONT_SCALE);
    }
}

char *argv_shift(int *argc, char ***argv)
{
    char *out = **argv;
    (*argc) -= 1;
    (*argv) += 1;
    return out;
}

Vec2f window_size(SDL_Window *window)
{
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    return (Vec2f){.x=(float)window_width, .y=(float)window_height};
}

void camera_project_point(SDL_Window *window, Vec2f point)
{
    Vec2f cp = {0};
    Vec2f ws = window_size(window);
    vec2f_add(&cp, camera_pos);
    vec2f_div(&ws, (Vec2f){.x=2,.y=2});
    vec2f_add(&cp, ws);
    vec2f_sub(&cp, point);
    vec2f_sub(&camera_vel, cp);
    vec2f_mul(&camera_vel, (Vec2f){.x=DELTA_TIME,.y=DELTA_TIME});
    vec2f_mul(&camera_vel, (Vec2f){.x=2,.y=2});
    vec2f_add(&camera_pos, camera_vel);
}

int main(int argc, char **argv)
{
    argv_shift(&argc, &argv);
    char *loaded_file_path = NULL; 

    if (argc>0) {
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

    SDL_Window *window = scp(SDL_CreateWindow("broadnic", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Font font = font_load_from_file(renderer, "./charmap-oldschool_white.png");

    bool quit = false;
    while (!quit) {
        const Uint32 start = SDL_GetTicks();
        SDL_Event evt = {0};
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_QUIT: {
                    quit=true;
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
                            quit=true;
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

        for (size_t row=0;row<editor.size;++row) {
            const Line *line = editor.lines + row;
            Vec2f line_pos = {0};
            vec2f_make(&line_pos, 0, row*FONT_CHAR_HEIGHT*FONT_SCALE);
            vec2f_sub(&line_pos, camera_pos);
            render_text_sized(renderer, &font, line->es, line->size, line_pos, 0xFFFFFFFF, FONT_SCALE);
        }

        const Vec2f cursor_pos = {
            .x = (int) floorf(editor.cursor_col * FONT_CHAR_WIDTH*FONT_SCALE),
            .y = (int) floorf(editor.cursor_row * FONT_CHAR_HEIGHT*FONT_SCALE),
        };
        camera_project_point(window, cursor_pos);

        render_cursor(renderer, &font);

        SDL_RenderPresent(renderer);
        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000/FPS; 
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms-duration);
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#define SV_IMPLEMENTATION
#include "sv.h"

