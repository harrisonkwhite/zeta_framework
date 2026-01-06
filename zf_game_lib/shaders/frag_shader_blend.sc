$input v_color0, v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(u_texture, 0);

uniform vec4 u_blend;

void main() {
    vec4 texture_color = texture2D(u_texture, v_texcoord0);

    vec4 mixed_color = vec4(
        mix(texture_color.r, u_blend.r, u_blend.a),
        mix(texture_color.g, u_blend.g, u_blend.a),
        mix(texture_color.b, u_blend.b, u_blend.a),
        texture_color.a
    );

    gl_FragColor = mixed_color * v_color0;
}
