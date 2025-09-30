$input a_position, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3, a_color0
$output v_texcoord0, v_color0

#include "bgfx_shader.sh"

void main() {
    vec2 vert_coord = a_position;
    vec2 pos = a_texcoord0;
    vec2 size = a_texcoord1;
    float rot = a_texcoord2;
    vec2 uv = a_texcoord3;

    float c = cos(rot);
    float s = sin(rot);
    mat2 r = mat2(c, -s, s, c);

    vec2 local = vert_coord * size;
    vec2 world = mul(r, local) + pos;

    gl_Position = mul(u_modelViewProj, vec4(world, 0.0, 1.0));

    v_texcoord0 = uv;
    v_color0 = a_color0;
}
