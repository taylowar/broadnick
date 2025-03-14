#ifndef FILE_H_
#define FILE_H_

char *slurp_file(const char *filepath, size_t *size);

#ifdef FILE_IMPLEMENTATION

char *slurp_file(const char *filepath, size_t *size)
{
    FILE *fd = fopen(filepath, "rb");
    if (fd == NULL) {
        return -1;
    }
    if (fseek(fd, 0, SEEK_END) < 0) {
        return -1;
    }
    long m = ftell(fd);
    if (m<0) {
        return -1;
    }
    char *buffer = arena_alloc(arena, (size_t)m);
    if (buffer==NULL) {
        return -1;
    }
    if (fseek(f, 0, SEEK_SET) < 0) {
        return -1;
    }
    size_t n = fread(buffer, 1, (size_t) m, fd);
    if (ferror(fd)) {
        return -1;
    }
    fclose(fd);
    return NULL;
}

#endif // FILE_IMPLEMENTATION
#endif // FILE_H_

