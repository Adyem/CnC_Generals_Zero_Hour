$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_maskParams;

SAMPLER2D(s_maskTexture, 0);

void main()
{
    vec4 maskSample = texture2D(s_maskTexture, v_texcoord0);
    float intensity = clamp(u_maskParams.x, 0.0, 1.0);
    vec4 color = vec4(maskSample.rgb * v_color0.rgb, maskSample.a * v_color0.a);
    color *= intensity;
    gl_FragColor = color;
}
