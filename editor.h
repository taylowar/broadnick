#ifndef EDITOR_H_
#define EDITOR_H_

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define LINE_INIT_CAPACITY 1024

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

void line_insert_text_before(Line *line, const char *text, size_t *col);
void line_backspace(Line *line, size_t *col);
void line_delete(Line *line, size_t *col);

#define EDITOR_INIT_CAPACITY 128

typedef struct {
    size_t capacity;
    size_t size;
    Line *lines;
    size_t cursor_row;
    size_t cursor_col;
} Editor;

void editor_push_new_line(Editor *editor);
void editor_insert_new_line(Editor *editor);
void editor_insert_text_before_cursor(Editor *editor, const char *text);
void editor_backspace(Editor *editor);
void editor_delete(Editor *editor);
void editor_save_to_file(const Editor *editor, const char *filepath);

const char *editor_char_under_cursor(Editor *editor);

#ifdef EDITOR_IMPLEMENTATION

void line_insert_text_before(Line *line, const char *text, size_t *col)
{
    if (*col > line->size) {
        (*col) = line->size;
    }
    size_t text_size = strlen(text);
    line_extend(line, text_size);
    memmove(
        line->es+(*col)+text_size,
        line->es+(*col),
        line->size-(*col)
    );
    memcpy(line->es+(*col), text, text_size);
    line->size += text_size;
    *col += text_size;
}

void line_backspace(Line *line, size_t *col)
{
    if ((*col) > line->size) {
        (*col) = line->size;
    }
    if (line->size > 0 && (*col) > 0) {
        memcpy(line->es+(*col)-1, line->es+(*col), line->size-(*col));
        line->size-=1;
        *col-=1;
    }
}

void line_delete(Line *line, size_t *col)
{
    if ((*col) > line->size) {
        (*col) = line->size;
    }
    if ((*col) < line->size && line->size > 0) {
        memcpy(line->es+(*col), line->es+(*col)+1, line->size-(*col));
        line->size-=1;
    }
}

// EDITOR //

void editor_grow(Editor *editor, size_t n)
{
    size_t new_capacity = editor->capacity;
    assert(new_capacity >= editor->size);
    while (new_capacity - editor->size < n) {
        if (editor->capacity == 0) {
            new_capacity = EDITOR_INIT_CAPACITY;
        } else {
            new_capacity = new_capacity*2;
        }
    }
    if (new_capacity != editor->capacity) {
        editor->lines = realloc(editor->lines, editor->capacity*sizeof(editor->lines[0]));
        editor->capacity = new_capacity;
    }
}

void editor_push_new_line(Editor *editor)
{
    editor_grow(editor, 1);
    memset(&editor->lines[editor->size], 0, sizeof(editor->lines[0]));
    editor->size+=1;
}

void editor_insert_new_line(Editor *editor)
{
    if (editor->cursor_row > editor->size) {
        editor->cursor_row = editor->size;
    }
    editor_grow(editor, 1);
    const size_t line_size = sizeof(editor->lines[0]);
    memmove(
        editor->lines + (editor->cursor_row+1),
        editor->lines + (editor->cursor_row),
        (editor->size - editor->cursor_row) * line_size
    );
    memset(&editor->lines[editor->cursor_row+1], 0, line_size);
    editor->cursor_row += 1;
    editor->cursor_col = 0;
    editor->size += 1;
}

void editor_insert_text_before_cursor(Editor *editor, const char *text)
{
    if (editor->cursor_row >= editor->size) {
        if (editor->size > 0) {
            editor->cursor_row = editor->size-1; 
        } else {
            editor_push_new_line(editor);
        }
    }
    line_insert_text_before(&editor->lines[editor->cursor_row], text, &editor->cursor_col);
}

void editor_backspace(Editor *editor)
{
    if (editor->cursor_row >= editor->size) {
        if (editor->size > 0) {
            editor->cursor_row = editor->size-1; 
        } else {
            editor_push_new_line(editor);
        }
    }
    line_backspace(&editor->lines[editor->cursor_row], &editor->cursor_col);
}

void editor_delete(Editor *editor)
{
    if (editor->cursor_row >= editor->size) {
        if (editor->size > 0) {
            editor->cursor_row = editor->size-1; 
        } else {
            editor_push_new_line(editor);
        }
    }
    line_delete(&editor->lines[editor->cursor_row], &editor->cursor_col);
}

const char *editor_char_under_cursor(Editor *editor)
{
    if (editor->cursor_row < editor->size) {
        if (editor->cursor_col < editor->lines[editor->cursor_row].size) {
            return &editor->lines[editor->cursor_row].es[editor->cursor_col];
        }
    }
    return NULL;
}

void editor_save_to_file(const Editor *editor, const char *filepath)
{
    FILE *fd = fopen(filepath, "w");
    if (fd == NULL) {
        fprintf(stderr, "ERROR: unable to open %s: %s\n", filepath, strerror(errno));
        exit(1);
    }
    for (size_t row=0;row<editor->size;++row) {
        fwrite(editor->lines[row].es, 1, editor->lines[row].size, fd);
        fputc('\n', fd);
    }
    fclose(fd);
}

#endif // EDITOR_IMPLEMENTATIO

#endif // EDITOR_H_

