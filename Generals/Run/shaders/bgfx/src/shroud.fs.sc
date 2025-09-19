$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

#include <bgfx_shader.sh>

uniform vec4 u_materialAmbient;
uniform vec4 u_materialDiffuse;
uniform vec4 u_materialSpecular;
uniform vec4 u_materialEmissive;
uniform vec4 u_materialParams;

uniform vec4 u_shroudParams;

SAMPLER2D(s_shroudTexture, 0);

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
    vec4 shroud = texture2D(s_shroudTexture, v_texcoord0);
    float opacity = clamp(u_shroudParams.x, 0.0, 1.0);
    vec4 color = vec4(shroud.rgb * v_color0.rgb, shroud.a * v_color0.a);
    color.a *= opacity;
    color = applyMaterial(color);
    gl_FragColor = color;
}
