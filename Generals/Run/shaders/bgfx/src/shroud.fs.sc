$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_shroudParams;

SAMPLER2D(s_shroudTexture, 0);

void main()
{
    vec4 shroud = texture2D(s_shroudTexture, v_texcoord0);
    float opacity = clamp(u_shroudParams.x, 0.0, 1.0);
    vec4 color = vec4(shroud.rgb * v_color0.rgb, shroud.a * v_color0.a);
    color.a *= opacity;
    gl_FragColor = color;
}
