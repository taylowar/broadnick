#version 330 core

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_ROWS 7
#define FONT_COLS 18
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_CHAR_WIDTH_UV (float(FONT_CHAR_WIDTH) / FONT_WIDTH)
#define FONT_CHAR_HEIGHT_UV (float(FONT_CHAR_HEIGHT) / FONT_HEIGHT)

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

uniform sampler2D font;
uniform float time;

in vec2 uv;
in float glyph_ch;
in vec4 glyph_color;

void main() {
    int ch = int(glyph_ch);
    if (!(ASCII_DISPLAY_LOW <= ch && ch <= ASCII_DISPLAY_HIGH)) {
        ch = 63;
    }
    int idx = ch - 32;
    float x = float(idx % FONT_COLS) * FONT_CHAR_WIDTH_UV;
    float y = float(idx / FONT_COLS) * FONT_CHAR_HEIGHT_UV;

    vec2 pos = vec2(x, y + FONT_CHAR_HEIGHT_UV);
    vec2 size = vec2(FONT_CHAR_WIDTH_UV, -FONT_CHAR_HEIGHT_UV);

    vec2 t = pos + size*uv;
    gl_FragColor = 
        texture(font, t) * 
        vec4(
            (sin(time)+1.0)/2.0,
            (cos(time)+1.0)/2.0,
            (sin(time)+1.0)/2.0,
            1.0
        );

}

