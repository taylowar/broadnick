#ifndef VEC_H_

typedef struct {
    float x,y;
} Vec2f;

void  vec2f_make(Vec2f *v, float x, float y);
void  vec2f_make_single(Vec2f *v, float xy);
Vec2f vec2f_clone(Vec2f v);

void vec2f_add(Vec2f *dst, Vec2f v);
void vec2f_sub(Vec2f *dst, Vec2f v);
void vec2f_mul(Vec2f *dst, Vec2f v);
void vec2f_div(Vec2f *dst, Vec2f v);

#ifdef VEC_IMPLEMENTATION

void vec2f_make(Vec2f *v, float x, float y)
{
    v->x = x;
    v->y = y;
}

void vec2f_make_single(Vec2f *v, float xy)
{
    v->x = xy;
    v->y = xy;
}

Vec2f vec2f_clone(Vec2f v)
{
    Vec2f nv = {0};
    nv.x=v.x;
    nv.y=v.y;
    return nv;
}

void vec2f_add(Vec2f *dst, Vec2f v)
{
    dst->x += v.x;
    dst->y += v.y;
}

void vec2f_sub(Vec2f *dst, Vec2f v)
{
    dst->x -= v.x;
    dst->y -= v.y;
}
void vec2f_mul(Vec2f *dst, Vec2f v)
{
    dst->x *= v.x;
    dst->y *= v.y;
}
void vec2f_div(Vec2f *dst, Vec2f v)
{
    dst->x /= v.x;
    dst->y /= v.y;
}
#endif // VEC_IMPLEMENTATION

#endif // VEC_H_

