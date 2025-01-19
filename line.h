#ifndef LINE_H_
#define LINE_H_

#include <assert.h>
#include <stddef.h>
#include <string.h>

#define LINE_INIT_CAPACITY 245

typedef struct {
    size_t capacity;
    size_t size;
    char *es;
} Line;

static void line_extend(Line *line, size_t n)
{
    size_t new_capacity = line->capacity;
    assert(new_capacity >= line->size);
    while (new_capacity - line->size < n) {
        if (line->capacity == 0) {
            new_capacity = LINE_INIT_CAPACITY;
        } else {
            new_capacity = new_capacity*2;
        }
    }
    if (new_capacity != line->capacity) {
        line->es = realloc(line->es, line->capacity);
        line->capacity = new_capacity;
    }
}

void line_insert_text_before(Line *line, const char *text, size_t col);
void line_backspace(Line *line, size_t col);
void line_delete(Line *line, size_t col);

#ifdef LINE_IMPLEMENTATION

void line_insert_text_before(Line *line, const char *text, size_t col)
{
    size_t text_size = strlen(text);
    printf("size = %zu, cap = %zu\n", line->size, line->capacity);
    line_extend(line, text_size);
    memmove(
        line->es+col+text_size,
        line->es+col,
        line->size-col
    );
    memcpy(line->es+col, text, text_size);
    line->size += text_size;
}

void line_backspace(Line *line, size_t col)
{
    if (line->size > 0 && col > 0) {
        memcpy(line->es+col-1, line->es+col, line->size-col);
        line->size-=1;
    }
}

void line_delete(Line *line, size_t col)
{
    if (col < line->size && line->size > 0) {
        memcpy(line->es+col, line->es+col+1, line->size-col);
        line->size-=1;
    }
}

#endif // LINE_IMPLEMENTATION

#endif // LINE_H_

