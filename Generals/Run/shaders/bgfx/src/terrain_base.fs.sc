$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_materialAmbient;
uniform vec4 u_materialDiffuse;
uniform vec4 u_materialSpecular;
uniform vec4 u_materialEmissive;
uniform vec4 u_materialParams;

uniform vec4 u_layerConfig;

SAMPLER2D(s_baseTexture, 0);
SAMPLER2D(s_detailTexture, 1);

vec4 applyDetailLayer(vec4 baseColor, vec4 detailColor, float blendWeight)
{
    float weight = clamp(blendWeight, 0.0, 1.0);
    return mix(baseColor, baseColor * detailColor, weight);
}

vec4 applyMaterial(vec4 color)
{
    vec4 result = color;
    result.rgb *= u_materialDiffuse.rgb;
    result.a *= u_materialDiffuse.a;

    if (u_materialParams.x > 0.5)
    {
        float specFactor = clamp(u_materialParams.y / 128.0, 0.0, 1.0);
        vec3 lightingAdd = u_materialAmbient.rgb + u_materialEmissive.rgb + u_materialSpecular.rgb * specFactor;
        result.rgb = clamp(result.rgb + lightingAdd, 0.0, 1.0);
    }
    else
    {
        result.rgb = clamp(result.rgb + u_materialEmissive.rgb, 0.0, 1.0);
    }

    return result;
}

void main()
{
    vec4 baseSample = texture2D(s_baseTexture, v_texcoord0);
    vec4 detailSample = texture2D(s_detailTexture, v_texcoord1);

    vec4 color = baseSample * v_color0;
    color = applyDetailLayer(color, detailSample, u_layerConfig.x);
    color = applyMaterial(color);

    gl_FragColor = color;
}
