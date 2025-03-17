
#ifndef SDL_EXTRA_H_
#define SDL_EXTRA_H_

void scc(int code);
void *scp(void *ptr);

SDL_Surface *surface_from_file(const char *filepath);
void set_texture_color(SDL_Texture *texture, Uint32 color);

#ifdef SDL_EXTRA_IMPLEMENTATION
void scc(int code) {
  if (code < 0) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
}

void *scp(void *ptr) {
  if (ptr == NULL) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
  return ptr;
}

SDL_Surface *surface_from_file(const char *filepath) {
  int width, height, n;
  unsigned char *pixels =
      stbi_load(filepath, &width, &height, &n, STBI_rgb_alpha);
  if (pixels == NULL) {
    fprintf(stderr, "ERROR: could not load %s: %s\n", filepath,
            stbi_failure_reason());
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
  const int pitch = 4 * width;
  return scp(SDL_CreateRGBSurfaceFrom((void *)pixels, width, height, depth,
                                      pitch, rmask, gmask, bmask, amask));
}

void set_texture_color(SDL_Texture *texture, Uint32 color) {
  scc(SDL_SetTextureColorMod(texture, (color >> (8 * 0)) & 0xff,
                             (color >> (8 * 1)) & 0xff,
                             (color >> (8 * 2)) & 0xff));
  scc(SDL_SetTextureAlphaMod(texture, (color >> (8 * 3)) & 0xff));
}
#endif // SDL_EXTRA_IMPLEMENTATION
#endif // SDL_EXTRA_H_

