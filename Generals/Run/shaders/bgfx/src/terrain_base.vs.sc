$input a_position, a_color0, a_texcoord0, a_texcoord1
$output v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform mat4 u_modelViewProj;
uniform mat4 u_texTransform[4];

void main()
{
    vec4 position = vec4(a_position, 1.0);
    gl_Position = mul(u_modelViewProj, position);

    v_color0 = a_color0;
    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord1;
    v_texcoord2 = (mul(u_texTransform[2], position)).xy;
    v_texcoord3 = (mul(u_texTransform[3], position)).xy;
}
