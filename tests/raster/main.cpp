#include <flame/foundation/bitmap.h>

using namespace flame;

struct Renderbuffer {
    int w, h, ys;
    void* data;
};

struct Vert {
    Vec4f position;
    Vec4f texcoord;
    Vec4f color;
};

struct Varying {
    Vec4f texcoord;
    Vec4f color;
};

void vertex_shader(const Vert& in, Vec4f& gl_Position, Varying& out)
{
    out.texcoord = in.texcoord;
    out.color = in.color;
    gl_Position = { in.position[0], in.position[1], -2 * in.position[2] - 2 * in.position[3], -in.position[2] };
}

void fragment_shader(Vec4f& gl_FragCoord, const Varying& in, Vec4f& out)
{
    out = in.color;
    Vec2f wrapped = Vec2f(in.texcoord - floor(in.texcoord));
    bool brighter = (wrapped[0] < 0.5) != (wrapped[1] < 0.5);
    if (!brighter)
        (Vec3f&)out = 0.5f * (Vec3f&)out;
}

void store_color(Renderbuffer& buf, int x, int y, const Vec4f& c)
{
    // can do alpha composition here
    uint8_t* p = (uint8_t*)buf.data + buf.ys * (buf.h - y - 1) + 4 * x;
    p[0] = lrint(c[0] * 255);
    p[1] = lrint(c[1] * 255);
    p[2] = lrint(c[2] * 255);
    p[3] = lrint(c[3] * 255);
}

void draw_triangle(Renderbuffer& color_attachment, const Vec4f& viewport, const Vert* verts)
{
    Varying perVertex[3];
    Vec4f gl_Position[3];

    Vec4f aabbf = viewport;
    for (int i = 0; i < 3; ++i)
    {
        // invoke the vertex shader
        vertex_shader(verts[i], gl_Position[i], perVertex[i]);

        // convert to device coordinates by perspective division
        gl_Position[i][3] = 1 / gl_Position[i][3];
        gl_Position[i][0] *= gl_Position[i][3];
        gl_Position[i][1] *= gl_Position[i][3];
        gl_Position[i][2] *= gl_Position[i][3];

        // convert to window coordinates
        auto& pos2 = (Vec2f&)gl_Position[i];
        pos2 = mix(viewport.xy(), viewport.zw(), 0.5f * (pos2 + Vec2f(1)));
        //rect_expand(aabbf, (const Vec2f&)gl_Position[i]);
    }

    // precompute the affine transform from fragment coordinates to barycentric coordinates
    const float denom = 1 / ((gl_Position[0][0] - gl_Position[2][0]) * (gl_Position[1][1] - gl_Position[0][1]) - (gl_Position[0][0] - gl_Position[1][0]) * (gl_Position[2][1] - gl_Position[0][1]));
    const Vec3f barycentric_d0 = denom * Vec3f(gl_Position[1][1] - gl_Position[2][1], gl_Position[2][1] - gl_Position[0][1], gl_Position[0][1] - gl_Position[1][1]);
    const Vec3f barycentric_d1 = denom * Vec3f(gl_Position[2][0] - gl_Position[1][0], gl_Position[0][0] - gl_Position[2][0], gl_Position[1][0] - gl_Position[0][0]);
    const Vec3f barycentric_0 = denom * Vec3f(
        gl_Position[1][0] * gl_Position[2][1] - gl_Position[2][0] * gl_Position[1][1],
        gl_Position[2][0] * gl_Position[0][1] - gl_Position[0][0] * gl_Position[2][1],
        gl_Position[0][0] * gl_Position[1][1] - gl_Position[1][0] * gl_Position[0][1]
    );

    // loop over all pixels in the rectangle bounding the triangle
    const Vec4i aabb = Vec4i(aabbf);
    for (int y = aabb.y(); y < aabb.w(); ++y)
        for (int x = aabb.x(); x < aabb.z(); ++x)
        {
            Vec4f gl_FragCoord;
            gl_FragCoord[0] = x + 0.5;
            gl_FragCoord[1] = y + 0.5;

            // fragment barycentric coordinates in window coordinates
            const Vec3f barycentric = gl_FragCoord[0] * barycentric_d0 + gl_FragCoord[1] * barycentric_d1 + barycentric_0;

            // discard fragment outside the triangle. this doesn't handle edges correctly.
            if (barycentric[0] < 0 || barycentric[1] < 0 || barycentric[2] < 0)
                continue;

            // interpolate inverse depth linearly
            gl_FragCoord[2] = dot(barycentric, Vec3f(gl_Position[0][2], gl_Position[1][2], gl_Position[2][2]));
            gl_FragCoord[3] = dot(barycentric, Vec3f(gl_Position[0][3], gl_Position[1][3], gl_Position[2][3]));

            // clip fragments to the near/far planes (as if by GL_ZERO_TO_ONE)
            if (gl_FragCoord[2] < 0 || gl_FragCoord[2] > 1)
                continue;

            // convert to perspective correct (clip-space) barycentric
            const Vec3f perspective = 1 / gl_FragCoord[3] * barycentric * Vec3f(gl_Position[0][3], gl_Position[1][3], gl_Position[2][3]);

            // interpolate the attributes using the perspective correct barycentric
            Varying varying;
            for (int i = 0; i < sizeof(Varying) / sizeof(float); ++i)
                ((float*)&varying)[i] = dot(perspective, Vec3f(
                    ((const float*)&perVertex[0])[i],
                    ((const float*)&perVertex[1])[i],
                    ((const float*)&perVertex[2])[i]
                ));

            // invoke the fragment shader and store the result
            Vec4f color;
            fragment_shader(gl_FragCoord, varying, color);
            store_color(color_attachment, x, y, color);
        }
}

int main()
{
    Renderbuffer buffer = { 512, 512, 512 * 4 };
    buffer.data = calloc(buffer.ys, buffer.h);

    // interleaved attributes buffer
    Vert verts[] = {
        { { -1, -1, -2, 1 }, { 0, 0, 0, 1 }, { 0, 0, 1, 1 } },
        { { 1, -1, -1, 1 }, { 10, 0, 0, 1 }, { 1, 0, 0, 1 } },
        { { 0, 1, -1, 1 }, { 0, 10, 0, 1 }, { 0, 1, 0, 1 } },
    };

    Vec4f viewport = { 0, 0, (float)buffer.w, (float)buffer.h };
    draw_triangle(buffer, viewport, verts);

    auto bmp = Bitmap::create(512, 512, 4, 1, (uchar*)buffer.data);
    bmp->save(L"D:/out.png");
}
