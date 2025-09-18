$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_layerConfig;
uniform vec4 u_fadeConfig;

SAMPLER2D(s_roadTexture, 0);
SAMPLER2D(s_detailTexture, 1);

vec4 applyDetailLayer(vec4 baseColor, vec4 detailColor, float blendWeight)
{
    float weight = clamp(blendWeight, 0.0, 1.0);
    return mix(baseColor, baseColor * detailColor, weight);
}

void main()
{
    vec4 roadSample = texture2D(s_roadTexture, v_texcoord0);
    vec4 detailSample = texture2D(s_detailTexture, v_texcoord1);

    vec4 color = roadSample * v_color0;
    color = applyDetailLayer(color, detailSample, u_layerConfig.x);
    color.a *= clamp(u_fadeConfig.x, 0.0, 1.0);

    gl_FragColor = color;
}
