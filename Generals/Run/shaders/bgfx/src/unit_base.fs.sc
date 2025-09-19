$input v_color0, v_texcoord0, v_texcoord1

#include <bgfx_shader.sh>

uniform vec4 u_materialAmbient;
uniform vec4 u_materialDiffuse;
uniform vec4 u_materialSpecular;
uniform vec4 u_materialEmissive;
uniform vec4 u_materialParams;
uniform vec4 u_textureConfig;
uniform vec4 u_alphaTestConfig;

SAMPLER2D(s_texture0, 0);
SAMPLER2D(s_texture1, 1);

vec4 sampleStage0(vec2 uv)
{
    if (u_textureConfig.x > 0.5)
    {
        return texture2D(s_texture0, uv);
    }

    return vec4(1.0);
}

vec4 sampleStage1(vec2 uv)
{
    if (u_textureConfig.y > 0.5)
    {
        return texture2D(s_texture1, uv);
    }

    return vec4(1.0);
}

vec4 applyMaterial(vec4 baseColor)
{
    vec4 result = baseColor;
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
    vec4 base = sampleStage0(v_texcoord0);
    vec4 detail = sampleStage1(v_texcoord1);

    vec4 color = base * v_color0;

    if (u_textureConfig.y > 0.5)
    {
        color.rgb *= detail.rgb;
        color.a *= detail.a;
    }

    float alphaPreMaterial = color.a;

    color = applyMaterial(color);

    float alphaForTest = (u_alphaTestConfig.w > 0.5) ? color.a : alphaPreMaterial;

    if (u_alphaTestConfig.x > 0.5)
    {
        float ref = u_alphaTestConfig.y;
        float func = u_alphaTestConfig.z;
        float alphaValue = alphaForTest;
        bool pass = true;
        float epsilon = 0.5 / 255.0;

        if (func < 1.5)
        {
            pass = false;
        }
        else if (func < 2.5)
        {
            pass = alphaValue < ref;
        }
        else if (func < 3.5)
        {
            pass = abs(alphaValue - ref) <= epsilon;
        }
        else if (func < 4.5)
        {
            pass = alphaValue <= ref;
        }
        else if (func < 5.5)
        {
            pass = alphaValue > ref;
        }
        else if (func < 6.5)
        {
            pass = abs(alphaValue - ref) > epsilon;
        }
        else if (func < 7.5)
        {
            pass = alphaValue >= ref;
        }
        else
        {
            pass = true;
        }

        if (!pass)
        {
            discard;
        }
    }

    gl_FragColor = color;
}
