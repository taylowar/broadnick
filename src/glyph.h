
#ifndef GLYPH_H_
#define GLYPH_H_

#define FONT_WIDTH 128.f
#define FONT_HEIGHT 64.f
#define FONT_ROWS 7
#define FONT_COLS 18
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)

// TODO: this header file depends on "vec.h"

typedef struct {
    Vec2f pos;   
    float scale;
    float ch;
    Vec4f color;
} Glyph;

typedef enum {
    GLYPH_ATTR_POS = 0,
    GLYPH_ATTR_SCALE,
    GLYPH_ATTR_CH,
    GLYPH_ATTR_COLOR,
    COUNT_GLYPH_ATTRS,
} Glyph_Attr;

typedef struct {
    size_t offset;
    size_t comps;
} Glyph_Attr_Def;

static const Glyph_Attr_Def glyph_attr_defs[COUNT_GLYPH_ATTRS] = {
    [GLYPH_ATTR_POS]   = {
        .offset = offsetof(Glyph, pos),
        .comps = 2,
    },
    [GLYPH_ATTR_SCALE] = {
        .offset = offsetof(Glyph, scale),
        .comps = 1,
    },
    [GLYPH_ATTR_CH]    = {
        .offset = offsetof(Glyph, ch),
        .comps = 1,
    },
    [GLYPH_ATTR_COLOR] = {
        .offset = offsetof(Glyph, color),
        .comps = 4,
    },
};

static_assert(COUNT_GLYPH_ATTRS == 4, "The amout of glyph vertex attributes have changed");

#define GLYPH_BUFFER_CAP 1024

Glyph glyph_buffer[GLYPH_BUFFER_CAP];
size_t glyph_buffer_count = 0;

void glyph_buffer_push(Glyph glyph)
{
    assert(glyph_buffer_count < GLYPH_BUFFER_CAP && "Glyph buffer overflow");
    glyph_buffer[glyph_buffer_count++] = glyph;
}

void glyph_buffer_sync(void)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, glyph_buffer_count * sizeof(Glyph), glyph_buffer);
}

void ogl_render_text(const char *text, size_t text_size, Vec2f pos, float scale, Vec4f color)
{
    Vec2f char_size = {0};
    for (size_t i=0; i < text_size; ++i) {
        vec2f_make(&char_size, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT);
        Vec2f new_pos = vec2f_clone(pos);
        Vec2f scalar = {0};
        vec2f_make(&scalar, (float)i*scale, 0);
        vec2f_mul(&char_size, scalar);
        vec2f_add(&new_pos, char_size);
        Glyph glyph = {
            .pos = new_pos,
            .scale = scale,
            .ch = (float)text[i],
            .color = color,
        };
        glyph_buffer_push(glyph);
    }
}

#ifdef GLYPH_IMPLEMENTATION
#endif // GLYPH_IMPLEMENTATION
#endif // GLYPH_H_
