$input v_color0, v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(u_texture, 0);

// @todo: This could possibly be generalised, considering there are 3 elements not in use?

uniform vec4 u_alpha; // Only R channel used.

void main() {
    vec4 texture_color = texture2D(u_texture, v_texcoord0);
    gl_FragColor = texture_color * vec4(1.0, 1.0, 1.0, u_alpha.r) * v_color0;
}
