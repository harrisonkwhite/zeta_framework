$input a_position, a_texcoord0, a_texcoord1, a_texcoord2, a_color0
$output v_color0

#include "bgfx_shader.sh"

void main()
{
    float c = cos(a_texcoord2);
    float s = sin(a_texcoord2);
    mat2 rot = mat2(c, -s, s, c);
    vec2 local = a_position * a_texcoord1;
    vec2 world = mul(rot, local) + a_texcoord0;

    gl_Position = mul(u_modelViewProj, vec4(world, 0.0, 1.0));
    v_color0 = a_color0;
}
