$input a_position, a_color0, a_texcoord0, a_texcoord1
$output v_color0, v_texcoord0, v_texcoord1

#include <bgfx_shader.sh>

uniform vec4 u_textureConfig;
uniform vec4 u_uvSource;

uniform mat4 u_texMtx0;
uniform mat4 u_texMtx1;

vec2 fetchTexcoord(float sourceIndex)
{
    int index = int(sourceIndex + 0.5);
    vec2 coord = a_texcoord0;
    if (index == 1)
    {
        coord = a_texcoord1;
    }
    return coord;
}

vec2 applyTransform(vec2 coord, mat4 texMatrix)
{
    vec4 transformed = mul(texMatrix, vec4(coord, 0.0, 1.0));
    return transformed.xy;
}

void main()
{
    vec4 position = vec4(a_position, 1.0);
    gl_Position = mul(u_modelViewProj, position);

    v_color0 = a_color0;

    vec2 uv0 = fetchTexcoord(u_uvSource.x);
    vec2 uv1 = fetchTexcoord(u_uvSource.y);

    if (u_textureConfig.z > 0.5)
    {
        uv0 = applyTransform(uv0, u_texMtx0);
    }

    if (u_textureConfig.w > 0.5)
    {
        uv1 = applyTransform(uv1, u_texMtx1);
    }

    v_texcoord0 = uv0;
    v_texcoord1 = uv1;
}
