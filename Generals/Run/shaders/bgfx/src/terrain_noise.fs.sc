$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_layerConfig;

SAMPLER2D(s_baseTexture, 0);
SAMPLER2D(s_detailTexture, 1);
SAMPLER2D(s_noiseTexture0, 2);
SAMPLER2D(s_noiseTexture1, 3);

vec4 applyDetailLayer(vec4 baseColor, vec4 detailColor, float blendWeight)
{
    float weight = clamp(blendWeight, 0.0, 1.0);
    return mix(baseColor, baseColor * detailColor, weight);
}

vec3 applyNoiseLayer(vec3 baseColor, vec4 noiseColor, float weight)
{
    float noiseWeight = clamp(weight, 0.0, 1.0);
    return mix(baseColor, baseColor * noiseColor.rgb, noiseWeight);
}

void main()
{
    vec4 baseSample = texture2D(s_baseTexture, v_texcoord0);
    vec4 detailSample = texture2D(s_detailTexture, v_texcoord1);
    vec4 noiseSample0 = texture2D(s_noiseTexture0, v_texcoord2);
    vec4 noiseSample1 = texture2D(s_noiseTexture1, v_texcoord3);

    vec4 color = baseSample * v_color0;
    color = applyDetailLayer(color, detailSample, u_layerConfig.x);

    color.rgb = applyNoiseLayer(color.rgb, noiseSample0, u_layerConfig.y);
    color.rgb = applyNoiseLayer(color.rgb, noiseSample1, u_layerConfig.z);

    gl_FragColor = color;
}
