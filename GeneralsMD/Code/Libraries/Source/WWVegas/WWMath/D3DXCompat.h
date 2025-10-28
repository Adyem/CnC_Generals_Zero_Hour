
#include "matrix4.h"
#include "vector3.h"
#include "vector4.h"

#include <cmath>
#include <d3d8.h>

struct D3DXVECTOR4 {
    float x;
    float y;
    float z;
    float w;

    D3DXVECTOR4() = default;
    D3DXVECTOR4(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}
    explicit D3DXVECTOR4(const Vector4 &v) : x(v.X), y(v.Y), z(v.Z), w(v.W) {}

    D3DXVECTOR4 &operator=(const Vector4 &v) {
        x = v.X;
        y = v.Y;
        z = v.Z;
        w = v.W;
        return *this;
    }

    float &operator[](int index) { return reinterpret_cast<float *>(this)[index]; }
    const float &operator[](int index) const { return reinterpret_cast<const float *>(this)[index]; }

    operator Vector4() const { return Vector4(x, y, z, w); }
};

struct D3DXVECTOR3 {
    float x;
    float y;
    float z;

    D3DXVECTOR3() = default;
    D3DXVECTOR3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
    explicit D3DXVECTOR3(const Vector3 &v) : x(v.X), y(v.Y), z(v.Z) {}

    D3DXVECTOR3 &operator=(const Vector3 &v) {
        x = v.X;
        y = v.Y;
        z = v.Z;
        return *this;
    }

    float &operator[](int index) { return reinterpret_cast<float *>(this)[index]; }
    const float &operator[](int index) const { return reinterpret_cast<const float *>(this)[index]; }

    operator Vector3() const { return Vector3(x, y, z); }
};

using D3DXMATRIX = Matrix4;

constexpr float D3DX_PI = 3.14159265358979323846f;
constexpr unsigned int D3DX_FILTER_NONE = 0x00000001u;
constexpr unsigned int D3DX_FILTER_POINT = 0x00000002u;
constexpr unsigned int D3DX_FILTER_LINEAR = 0x00000004u;
constexpr unsigned int D3DX_FILTER_TRIANGLE = 0x00000008u;
constexpr unsigned int D3DX_FILTER_BOX = 0x00000010u;
constexpr unsigned int D3DX_FILTER_MIRROR = 0x00010000u;
constexpr unsigned int D3DX_FILTER_MIRROR_U = 0x00020000u;
constexpr unsigned int D3DX_FILTER_MIRROR_V = 0x00040000u;
constexpr unsigned int D3DX_FILTER_MIRROR_W = 0x00080000u;
constexpr unsigned int D3DX_FILTER_DITHER = 0x00100000u;
constexpr unsigned int D3DX_FILTER_DITHER_DIFFUSION = 0x00200000u;

inline float WWDeterminant3x3(
    float a1,
    float a2,
    float a3,
    float b1,
    float b2,
    float b3,
    float c1,
    float c2,
    float c3) {
    return a1 * (b2 * c3 - b3 * c2) - a2 * (b1 * c3 - b3 * c1) + a3 * (b1 * c2 - b2 * c1);
}

inline float WWDeterminant4x4(const Matrix4 &m) {
    return m[0][0] * WWDeterminant3x3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]) -
           m[0][1] * WWDeterminant3x3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]) +
           m[0][2] * WWDeterminant3x3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]) -
           m[0][3] * WWDeterminant3x3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);
}

inline D3DXMATRIX *D3DXMatrixIdentity(D3DXMATRIX *out) {
    out->Make_Identity();
    return out;
}

inline D3DXMATRIX *D3DXMatrixScaling(D3DXMATRIX *out, float sx, float sy, float sz) {
    out->Make_Identity();
    (*out)[0][0] = sx;
    (*out)[1][1] = sy;
    (*out)[2][2] = sz;
    return out;
}

inline D3DXMATRIX *D3DXMatrixTranslation(D3DXMATRIX *out, float x, float y, float z) {
    out->Make_Identity();
    (*out)[0][3] = x;
    (*out)[1][3] = y;
    (*out)[2][3] = z;
    return out;
}

inline D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *out, float angle) {
    out->Make_Identity();
    float s = std::sin(angle);
    float c = std::cos(angle);
    (*out)[0][0] = c;
    (*out)[0][1] = -s;
    (*out)[1][0] = s;
    (*out)[1][1] = c;
    return out;
}

inline D3DXMATRIX *D3DXMatrixTranspose(D3DXMATRIX *out, const D3DXMATRIX *in) {
    *out = in->Transpose();
    return out;
}

inline D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *out, const D3DXMATRIX *a, const D3DXMATRIX *b) {
    Matrix4::Multiply(*a, *b, out);
    return out;
}

inline D3DXMATRIX *D3DXMatrixInverse(D3DXMATRIX *out, float *determinant, const D3DXMATRIX *in) {
    if (determinant != nullptr) {
        *determinant = WWDeterminant4x4(*in);
    }
    *out = in->Inverse();
    return out;
}

inline D3DXVECTOR4 *D3DXVec4Transform(D3DXVECTOR4 *out, const D3DXVECTOR4 *v, const D3DXMATRIX *m) {
    Vector4 result;
    Matrix4::Transform_Vector(*m, Vector4(v->x, v->y, v->z, v->w), &result);
    *out = D3DXVECTOR4(result);
    return out;
}

inline float D3DXVec4Dot(const D3DXVECTOR4 *a, const D3DXVECTOR4 *b) {
    return Vector4::Dot_Product(Vector4(a->x, a->y, a->z, a->w), Vector4(b->x, b->y, b->z, b->w));
}

inline D3DXVECTOR4 *D3DXVec3Transform(D3DXVECTOR4 *out, const D3DXVECTOR3 *v, const D3DXMATRIX *m) {
    Vector4 result;
    Matrix4::Transform_Vector(*m, Vector3(v->x, v->y, v->z), &result);
    *out = D3DXVECTOR4(result);
    return out;
}

inline unsigned D3DXGetFVFVertexSize(unsigned FVF) {
    unsigned size = 0;

    switch (FVF & D3DFVF_POSITION_MASK) {
        case D3DFVF_XYZ:
            size += 3 * sizeof(float);
            break;
        case D3DFVF_XYZRHW:
            size += 4 * sizeof(float);
            break;
        case D3DFVF_XYZB1:
            size += 4 * sizeof(float);
            break;
        case D3DFVF_XYZB2:
            size += 5 * sizeof(float);
            break;
        case D3DFVF_XYZB3:
            size += 6 * sizeof(float);
            break;
        case D3DFVF_XYZB4:
            size += 7 * sizeof(float);
            break;
        case D3DFVF_XYZB5:
            size += 8 * sizeof(float);
            break;
        default:
            break;
    }

    if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB4 || (FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB5) {
        if ((FVF & D3DFVF_LASTBETA_UBYTE4) == D3DFVF_LASTBETA_UBYTE4 ||
            (FVF & D3DFVF_LASTBETA_D3DCOLOR) == D3DFVF_LASTBETA_D3DCOLOR) {
            size -= sizeof(float);
            size += sizeof(DWORD);
        }
    }

    if ((FVF & D3DFVF_NORMAL) == D3DFVF_NORMAL) {
        size += 3 * sizeof(float);
    }

    if ((FVF & D3DFVF_PSIZE) == D3DFVF_PSIZE) {
        size += sizeof(float);
    }

    if ((FVF & D3DFVF_DIFFUSE) == D3DFVF_DIFFUSE) {
        size += sizeof(DWORD);
    }

    if ((FVF & D3DFVF_SPECULAR) == D3DFVF_SPECULAR) {
        size += sizeof(DWORD);
    }

    unsigned texCount = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    for (unsigned i = 0; i < texCount; ++i) {
        unsigned coordSize = 2;
        if ((FVF & D3DFVF_TEXCOORDSIZE1(i)) == D3DFVF_TEXCOORDSIZE1(i)) {
            coordSize = 1;
        } else if ((FVF & D3DFVF_TEXCOORDSIZE2(i)) == D3DFVF_TEXCOORDSIZE2(i)) {
            coordSize = 2;
        } else if ((FVF & D3DFVF_TEXCOORDSIZE3(i)) == D3DFVF_TEXCOORDSIZE3(i)) {
            coordSize = 3;
        } else if ((FVF & D3DFVF_TEXCOORDSIZE4(i)) == D3DFVF_TEXCOORDSIZE4(i)) {
            coordSize = 4;
        }
        size += coordSize * sizeof(float);
    }

    return size;
}

inline HRESULT D3DXFilterTexture(
    IDirect3DBaseTexture8 *texture,
    IDirect3DBaseTexture8 *,
    unsigned,
    unsigned) {
    if (texture == nullptr) {
        return D3DERR_INVALIDCALL;
    }
    texture->GenerateMipSubLevels();
    return D3D_OK;
}

inline HRESULT D3DXLoadSurfaceFromSurface(
    IDirect3DSurface8 *dest,
    const PALETTEENTRY *,
    const RECT *destRect,
    IDirect3DSurface8 *src,
    const PALETTEENTRY *,
    const RECT *srcRect,
    unsigned,
    D3DCOLOR) {
    if (dest == nullptr || src == nullptr) {
        return D3DERR_INVALIDCALL;
    }

    RECT source = {};
    if (srcRect != nullptr) {
        source = *srcRect;
    } else {
        D3DSURFACE_DESC desc;
        if (FAILED(src->GetDesc(&desc))) {
            return D3DERR_INVALIDCALL;
        }
        source.left = 0;
        source.top = 0;
        source.right = static_cast<LONG>(desc.Width);
        source.bottom = static_cast<LONG>(desc.Height);
    }

    POINT destPoint = {};
    if (destRect != nullptr) {
        destPoint.x = destRect->left;
        destPoint.y = destRect->top;
    }

    IDirect3DDevice8 *device = nullptr;
    HRESULT hr = dest->GetDevice(&device);
    if (FAILED(hr) || device == nullptr) {
        if (device != nullptr) {
            device->Release();
        }
        return D3DERR_INVALIDCALL;
    }

    hr = device->CopyRects(src, &source, 1, dest, &destPoint);
    device->Release();
    return hr;
}

inline float D3DXToRadian(float degree) {
    return degree * (D3DX_PI / 180.0f);
}

inline float D3DXToDegree(float radian) {
    return radian * (180.0f / D3DX_PI);
}

