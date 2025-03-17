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

typedef struct {
    float x,y,z,t;
} Vec4f;

void  vec4f_make(Vec4f *v, float x, float y, float z, float t);
void  vec4f_make_single(Vec4f *v, float xyzt);
Vec4f vec4f_clone(Vec4f v);

void vec4f_add(Vec4f *dst, Vec4f v);
void vec4f_sub(Vec4f *dst, Vec4f v);
void vec4f_mul(Vec4f *dst, Vec4f v);
void vec4f_div(Vec4f *dst, Vec4f v);

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

void vec4f_make(Vec4f *v, float x, float y, float z, float t)
{
    v->x = x;
    v->y = y;
    v->z = z;
    v->t = t;
}

void vec4f_make_single(Vec4f *v, float xyzt)
{
    v->x = xyzt;
    v->y = xyzt;
    v->z = xyzt;
    v->t = xyzt;
}

Vec4f vec4f_clone(Vec4f v)
{
    Vec4f nv = {0};
    nv.x=v.x;
    nv.y=v.y;
    nv.z=v.z;
    nv.t=v.t;
    return nv;
}

void vec4f_add(Vec4f *dst, Vec4f v)
{
    dst->x += v.x;
    dst->y += v.y;
    dst->z += v.z;
    dst->t += v.t;
}

void vec4f_sub(Vec4f *dst, Vec4f v)
{
    dst->x -= v.x;
    dst->y -= v.y;
    dst->z -= v.z;
    dst->t -= v.t;
}
void vec4f_mul(Vec4f *dst, Vec4f v)
{
    dst->x *= v.x;
    dst->y *= v.y;
    dst->z *= v.z;
    dst->t *= v.t;

}
void vec4f_div(Vec4f *dst, Vec4f v)
{
    
    dst->x /= v.x;
    dst->y /= v.y;
    dst->z /= v.z;
    dst->t /= v.t;
}

#endif // VEC_IMPLEMENTATION

#endif // VEC_H_

