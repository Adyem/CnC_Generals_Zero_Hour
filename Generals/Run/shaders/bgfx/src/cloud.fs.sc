$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_cloudParams;

SAMPLER2D(s_cloudTexture, 0);

void main()
{
    vec4 cloudSample = texture2D(s_cloudTexture, v_texcoord0);
    float intensity = clamp(u_cloudParams.x, 0.0, 1.0);
    vec4 color = vec4(cloudSample.rgb * v_color0.rgb, cloudSample.a * v_color0.a);
    color *= intensity;
    gl_FragColor = color;
}
