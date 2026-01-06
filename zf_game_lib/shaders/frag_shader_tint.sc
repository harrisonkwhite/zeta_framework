$input v_color0, v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(u_tex, 0);

void main() {
    vec4 tex_color = texture2D(u_tex, v_texcoord0);
    gl_FragColor = tex_color * v_color0 * vec4(1.0, 0.0, 1.0, 1.0);
}
