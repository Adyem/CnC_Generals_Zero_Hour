
#ifndef _WIN32

#include <cstdint>

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                      \
    ((std::uint32_t(static_cast<unsigned char>(ch0))) |                                     \
     (std::uint32_t(static_cast<unsigned char>(ch1)) << 8) |                                \
     (std::uint32_t(static_cast<unsigned char>(ch2)) << 16) |                               \
     (std::uint32_t(static_cast<unsigned char>(ch3)) << 24))
#endif

constexpr std::uint32_t D3DCLEAR_TARGET  = 0x00000001u;
constexpr std::uint32_t D3DCLEAR_ZBUFFER = 0x00000002u;
constexpr std::uint32_t D3DCLEAR_STENCIL = 0x00000004u;

constexpr std::uint32_t D3DCLIPPLANE0 = 1u << 0;
constexpr std::uint32_t D3DCLIPPLANE1 = 1u << 1;
constexpr std::uint32_t D3DCLIPPLANE2 = 1u << 2;
constexpr std::uint32_t D3DCLIPPLANE3 = 1u << 3;
constexpr std::uint32_t D3DCLIPPLANE4 = 1u << 4;
constexpr std::uint32_t D3DCLIPPLANE5 = 1u << 5;

constexpr std::uint32_t D3DWRAP_U = 0x1u;
constexpr std::uint32_t D3DWRAP_V = 0x2u;
constexpr std::uint32_t D3DWRAP_W = 0x4u;

constexpr std::uint32_t D3DWRAPCOORD_0 = 0x1u;
constexpr std::uint32_t D3DWRAPCOORD_1 = 0x2u;
constexpr std::uint32_t D3DWRAPCOORD_2 = 0x4u;
constexpr std::uint32_t D3DWRAPCOORD_3 = 0x8u;

constexpr std::uint32_t D3DCOLORWRITEENABLE_RED   = 0x00000001u;
constexpr std::uint32_t D3DCOLORWRITEENABLE_GREEN = 0x00000002u;
constexpr std::uint32_t D3DCOLORWRITEENABLE_BLUE  = 0x00000004u;
constexpr std::uint32_t D3DCOLORWRITEENABLE_ALPHA = 0x00000008u;

constexpr std::uint32_t D3DLOCK_READONLY         = 0x00000010u;
constexpr std::uint32_t D3DLOCK_NOSYSLOCK        = 0x00000800u;
constexpr std::uint32_t D3DLOCK_NOOVERWRITE      = 0x00001000u;
constexpr std::uint32_t D3DLOCK_DISCARD          = 0x00002000u;
constexpr std::uint32_t D3DLOCK_NO_DIRTY_UPDATE  = 0x00008000u;

constexpr std::uint32_t D3DTA_SELECTMASK     = 0x0000000fu;
constexpr std::uint32_t D3DTA_DIFFUSE        = 0x00000000u;
constexpr std::uint32_t D3DTA_CURRENT        = 0x00000001u;
constexpr std::uint32_t D3DTA_TEXTURE        = 0x00000002u;
constexpr std::uint32_t D3DTA_TFACTOR        = 0x00000003u;
constexpr std::uint32_t D3DTA_SPECULAR       = 0x00000004u;
constexpr std::uint32_t D3DTA_TEMP           = 0x00000005u;
constexpr std::uint32_t D3DTA_ALPHAREPLICATE = 0x00000020u;
constexpr std::uint32_t D3DTA_COMPLEMENT     = 0x00000010u;

constexpr std::uint32_t D3DTSS_TCI_PASSTHRU                    = 0x00000000u;
constexpr std::uint32_t D3DTSS_TCI_CAMERASPACENORMAL           = 0x00010000u;
constexpr std::uint32_t D3DTSS_TCI_CAMERASPACEPOSITION         = 0x00020000u;
constexpr std::uint32_t D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR = 0x00030000u;

enum D3DBLEND : std::uint32_t
{
    D3DBLEND_ZERO               = 1,
    D3DBLEND_ONE                = 2,
    D3DBLEND_SRCCOLOR           = 3,
    D3DBLEND_INVSRCCOLOR        = 4,
    D3DBLEND_SRCALPHA           = 5,
    D3DBLEND_INVSRCALPHA        = 6,
    D3DBLEND_DESTALPHA          = 7,
    D3DBLEND_INVDESTALPHA       = 8,
    D3DBLEND_DESTCOLOR          = 9,
    D3DBLEND_INVDESTCOLOR       = 10,
    D3DBLEND_SRCALPHASAT        = 11,
    D3DBLEND_BOTHSRCALPHA       = 12,
    D3DBLEND_BOTHINVSRCALPHA    = 13,
    D3DBLEND_FORCE_DWORD        = 0x7fffffff
};

enum D3DBLENDOP : std::uint32_t
{
    D3DBLENDOP_ADD          = 1,
    D3DBLENDOP_SUBTRACT     = 2,
    D3DBLENDOP_REVSUBTRACT  = 3,
    D3DBLENDOP_MIN          = 4,
    D3DBLENDOP_MAX          = 5,
    D3DBLENDOP_FORCE_DWORD  = 0x7fffffff
};

enum D3DCMPFUNC : std::uint32_t
{
    D3DCMP_NEVER         = 1,
    D3DCMP_LESS          = 2,
    D3DCMP_EQUAL         = 3,
    D3DCMP_LESSEQUAL     = 4,
    D3DCMP_GREATER       = 5,
    D3DCMP_NOTEQUAL      = 6,
    D3DCMP_GREATEREQUAL  = 7,
    D3DCMP_ALWAYS        = 8,
    D3DCMP_FORCE_DWORD   = 0x7fffffff
};

enum D3DCULL : std::uint32_t
{
    D3DCULL_NONE        = 1,
    D3DCULL_CW          = 2,
    D3DCULL_CCW         = 3,
    D3DCULL_FORCE_DWORD = 0x7fffffff
};

enum D3DFILLMODE : std::uint32_t
{
    D3DFILL_POINT       = 1,
    D3DFILL_WIREFRAME   = 2,
    D3DFILL_SOLID       = 3,
    D3DFILL_FORCE_DWORD = 0x7fffffff
};

enum D3DFOGMODE : std::uint32_t
{
    D3DFOG_NONE        = 0,
    D3DFOG_EXP         = 1,
    D3DFOG_EXP2        = 2,
    D3DFOG_LINEAR      = 3,
    D3DFOG_FORCE_DWORD = 0x7fffffff
};

enum D3DPRIMITIVETYPE : std::uint32_t
{
    D3DPT_POINTLIST      = 1,
    D3DPT_LINELIST       = 2,
    D3DPT_LINESTRIP      = 3,
    D3DPT_TRIANGLELIST   = 4,
    D3DPT_TRIANGLESTRIP  = 5,
    D3DPT_TRIANGLEFAN    = 6,
    D3DPT_FORCE_DWORD    = 0x7fffffff
};

enum D3DRENDERSTATETYPE : std::uint32_t
{
    D3DRS_ZENABLE                   = 7,
    D3DRS_FILLMODE                  = 8,
    D3DRS_SHADEMODE                 = 9,
    D3DRS_LINEPATTERN               = 10,
    D3DRS_ZWRITEENABLE              = 14,
    D3DRS_ALPHATESTENABLE           = 15,
    D3DRS_LASTPIXEL                 = 16,
    D3DRS_SRCBLEND                  = 19,
    D3DRS_DESTBLEND                 = 20,
    D3DRS_CULLMODE                  = 22,
    D3DRS_ZFUNC                     = 23,
    D3DRS_ALPHAREF                  = 24,
    D3DRS_ALPHAFUNC                 = 25,
    D3DRS_DITHERENABLE              = 26,
    D3DRS_ALPHABLENDENABLE          = 27,
    D3DRS_FOGENABLE                 = 28,
    D3DRS_SPECULARENABLE            = 29,
    D3DRS_ZVISIBLE                  = 30,
    D3DRS_FOGCOLOR                  = 34,
    D3DRS_FOGTABLEMODE              = 35,
    D3DRS_FOGSTART                  = 36,
    D3DRS_FOGEND                    = 37,
    D3DRS_FOGDENSITY                = 38,
    D3DRS_EDGEANTIALIAS             = 40,
    D3DRS_ZBIAS                     = 47,
    D3DRS_RANGEFOGENABLE            = 48,
    D3DRS_STENCILENABLE             = 52,
    D3DRS_STENCILFAIL               = 53,
    D3DRS_STENCILZFAIL              = 54,
    D3DRS_STENCILPASS               = 55,
    D3DRS_STENCILFUNC               = 56,
    D3DRS_STENCILREF                = 57,
    D3DRS_STENCILMASK               = 58,
    D3DRS_STENCILWRITEMASK          = 59,
    D3DRS_TEXTUREFACTOR             = 60,
    D3DRS_WRAP0                     = 128,
    D3DRS_WRAP1                     = 129,
    D3DRS_WRAP2                     = 130,
    D3DRS_WRAP3                     = 131,
    D3DRS_WRAP4                     = 132,
    D3DRS_WRAP5                     = 133,
    D3DRS_WRAP6                     = 134,
    D3DRS_WRAP7                     = 135,
    D3DRS_CLIPPING                  = 136,
    D3DRS_LIGHTING                  = 137,
    D3DRS_AMBIENT                   = 139,
    D3DRS_FOGVERTEXMODE             = 140,
    D3DRS_COLORVERTEX               = 141,
    D3DRS_LOCALVIEWER               = 142,
    D3DRS_NORMALIZENORMALS          = 143,
    D3DRS_DIFFUSEMATERIALSOURCE     = 145,
    D3DRS_SPECULARMATERIALSOURCE    = 146,
    D3DRS_AMBIENTMATERIALSOURCE     = 147,
    D3DRS_EMISSIVEMATERIALSOURCE    = 148,
    D3DRS_VERTEXBLEND               = 151,
    D3DRS_CLIPPLANEENABLE           = 152,
    D3DRS_SOFTWAREVERTEXPROCESSING  = 153,
    D3DRS_POINTSIZE                 = 154,
    D3DRS_POINTSIZE_MIN             = 155,
    D3DRS_POINTSPRITEENABLE         = 156,
    D3DRS_POINTSCALEENABLE          = 157,
    D3DRS_POINTSCALE_A              = 158,
    D3DRS_POINTSCALE_B              = 159,
    D3DRS_POINTSCALE_C              = 160,
    D3DRS_MULTISAMPLEANTIALIAS      = 161,
    D3DRS_MULTISAMPLEMASK           = 162,
    D3DRS_PATCHEDGESTYLE            = 163,
    D3DRS_PATCHSEGMENTS             = 164,
    D3DRS_DEBUGMONITORTOKEN         = 165,
    D3DRS_POINTSIZE_MAX             = 166,
    D3DRS_INDEXEDVERTEXBLENDENABLE  = 167,
    D3DRS_COLORWRITEENABLE          = 168,
    D3DRS_TWEENFACTOR               = 170,
    D3DRS_BLENDOP                   = 171,
    D3DRS_POSITIONORDER             = 172,
    D3DRS_NORMALORDER               = 173,
    D3DRS_FORCE_DWORD               = 0x7fffffff
};

enum D3DTRANSFORMSTATETYPE : std::uint32_t
{
    D3DTS_VIEW        = 2,
    D3DTS_PROJECTION  = 3,
    D3DTS_TEXTURE0    = 16,
    D3DTS_TEXTURE1    = 17,
    D3DTS_TEXTURE2    = 18,
    D3DTS_TEXTURE3    = 19,
    D3DTS_TEXTURE4    = 20,
    D3DTS_TEXTURE5    = 21,
    D3DTS_TEXTURE6    = 22,
    D3DTS_TEXTURE7    = 23,
    D3DTS_FORCE_DWORD = 0x7fffffff
};

#define D3DTS_WORLDMATRIX(index) static_cast<D3DTRANSFORMSTATETYPE>((index) + 256)
#define D3DTS_WORLD  D3DTS_WORLDMATRIX(0)
#define D3DTS_WORLD1 D3DTS_WORLDMATRIX(1)
#define D3DTS_WORLD2 D3DTS_WORLDMATRIX(2)
#define D3DTS_WORLD3 D3DTS_WORLDMATRIX(3)

enum D3DTEXTUREADDRESS : std::uint32_t
{
    D3DTADDRESS_WRAP        = 1,
    D3DTADDRESS_MIRROR      = 2,
    D3DTADDRESS_CLAMP       = 3,
    D3DTADDRESS_BORDER      = 4,
    D3DTADDRESS_MIRRORONCE  = 5,
    D3DTADDRESS_FORCE_DWORD = 0x7fffffff
};

enum D3DTEXTUREFILTERTYPE : std::uint32_t
{
    D3DTEXF_NONE          = 0,
    D3DTEXF_POINT         = 1,
    D3DTEXF_LINEAR        = 2,
    D3DTEXF_ANISOTROPIC   = 3,
    D3DTEXF_FLATCUBIC     = 4,
    D3DTEXF_GAUSSIANCUBIC = 5,
    D3DTEXF_FORCE_DWORD   = 0x7fffffff
};

enum D3DTEXTUREOP : std::uint32_t
{
    D3DTOP_DISABLE                     = 1,
    D3DTOP_SELECTARG1                  = 2,
    D3DTOP_SELECTARG2                  = 3,
    D3DTOP_MODULATE                    = 4,
    D3DTOP_MODULATE2X                  = 5,
    D3DTOP_MODULATE4X                  = 6,
    D3DTOP_ADD                         = 7,
    D3DTOP_ADDSIGNED                   = 8,
    D3DTOP_ADDSIGNED2X                 = 9,
    D3DTOP_SUBTRACT                    = 10,
    D3DTOP_ADDSMOOTH                   = 11,
    D3DTOP_BLENDDIFFUSEALPHA           = 12,
    D3DTOP_BLENDTEXTUREALPHA           = 13,
    D3DTOP_BLENDFACTORALPHA            = 14,
    D3DTOP_BLENDTEXTUREALPHAPM         = 15,
    D3DTOP_BLENDCURRENTALPHA           = 16,
    D3DTOP_PREMODULATE                 = 17,
    D3DTOP_MODULATEALPHA_ADDCOLOR      = 18,
    D3DTOP_MODULATECOLOR_ADDALPHA      = 19,
    D3DTOP_MODULATEINVALPHA_ADDCOLOR   = 20,
    D3DTOP_MODULATEINVCOLOR_ADDALPHA   = 21,
    D3DTOP_BUMPENVMAP                  = 22,
    D3DTOP_BUMPENVMAPLUMINANCE         = 23,
    D3DTOP_DOTPRODUCT3                 = 24,
    D3DTOP_MULTIPLYADD                 = 25,
    D3DTOP_LERP                        = 26,
    D3DTOP_FORCE_DWORD                 = 0x7fffffff
};

enum D3DTEXTURESTAGESTATETYPE : std::uint32_t
{
    D3DTSS_COLOROP              = 1,
    D3DTSS_COLORARG1            = 2,
    D3DTSS_COLORARG2            = 3,
    D3DTSS_ALPHAOP              = 4,
    D3DTSS_ALPHAARG1            = 5,
    D3DTSS_ALPHAARG2            = 6,
    D3DTSS_BUMPENVMAT00         = 7,
    D3DTSS_BUMPENVMAT01         = 8,
    D3DTSS_BUMPENVMAT10         = 9,
    D3DTSS_BUMPENVMAT11         = 10,
    D3DTSS_TEXCOORDINDEX        = 11,
    D3DTSS_ADDRESSU             = 13,
    D3DTSS_ADDRESSV             = 14,
    D3DTSS_BORDERCOLOR          = 15,
    D3DTSS_MAGFILTER            = 16,
    D3DTSS_MINFILTER            = 17,
    D3DTSS_MIPFILTER            = 18,
    D3DTSS_MIPMAPLODBIAS        = 19,
    D3DTSS_MAXMIPLEVEL          = 20,
    D3DTSS_MAXANISOTROPY        = 21,
    D3DTSS_BUMPENVLSCALE        = 22,
    D3DTSS_BUMPENVLOFFSET       = 23,
    D3DTSS_TEXTURETRANSFORMFLAGS= 24,
    D3DTSS_ADDRESSW             = 25,
    D3DTSS_COLORARG0            = 26,
    D3DTSS_ALPHAARG0            = 27,
    D3DTSS_RESULTARG            = 28,
    D3DTSS_FORCE_DWORD          = 0x7fffffff
};

enum D3DSTENCILOP : std::uint32_t
{
    D3DSTENCILOP_KEEP        = 1,
    D3DSTENCILOP_ZERO        = 2,
    D3DSTENCILOP_REPLACE     = 3,
    D3DSTENCILOP_INCRSAT     = 4,
    D3DSTENCILOP_DECRSAT     = 5,
    D3DSTENCILOP_INVERT      = 6,
    D3DSTENCILOP_INCR        = 7,
    D3DSTENCILOP_DECR        = 8,
    D3DSTENCILOP_FORCE_DWORD = 0x7fffffff
};

enum D3DSHADEMODE : std::uint32_t
{
    D3DSHADE_FLAT        = 1,
    D3DSHADE_GOURAUD     = 2,
    D3DSHADE_PHONG       = 3,
    D3DSHADE_FORCE_DWORD = 0x7fffffff
};

enum D3DZBUFFERTYPE : std::uint32_t
{
    D3DZB_FALSE        = 0,
    D3DZB_TRUE         = 1,
    D3DZB_USEW         = 2,
    D3DZB_FORCE_DWORD  = 0x7fffffff
};

enum D3DDEVTYPE : std::uint32_t
{
    D3DDEVTYPE_HAL         = 1,
    D3DDEVTYPE_REF         = 2,
    D3DDEVTYPE_SW          = 3,
    D3DDEVTYPE_FORCE_DWORD = 0x7fffffff
};

enum D3DRESOURCETYPE : std::uint32_t
{
    D3DRTYPE_SURFACE        = 1,
    D3DRTYPE_VOLUME         = 2,
    D3DRTYPE_TEXTURE        = 3,
    D3DRTYPE_VOLUMETEXTURE  = 4,
    D3DRTYPE_CUBETEXTURE    = 5,
    D3DRTYPE_VERTEXBUFFER   = 6,
    D3DRTYPE_INDEXBUFFER    = 7,
    D3DRTYPE_FORCE_DWORD    = 0x7fffffff
};

enum D3DPOOL : std::uint32_t
{
    D3DPOOL_DEFAULT     = 0,
    D3DPOOL_MANAGED     = 1,
    D3DPOOL_SYSTEMMEM   = 2,
    D3DPOOL_SCRATCH     = 3,
    D3DPOOL_FORCE_DWORD = 0x7fffffff
};

enum D3DTEXTUREFILTERCAPS : std::uint32_t
{
    D3DPTFILTERCAPS_MAGFPOINT           = 0x00000100u,
    D3DPTFILTERCAPS_MAGFLINEAR          = 0x00000200u,
    D3DPTFILTERCAPS_MAGFANISOTROPIC     = 0x00000400u,
    D3DPTFILTERCAPS_MAGFAFLATCUBIC      = 0x00000800u,
    D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC   = 0x00001000u,
    D3DPTFILTERCAPS_MINFPOINT           = 0x00010000u,
    D3DPTFILTERCAPS_MINFLINEAR          = 0x00020000u,
    D3DPTFILTERCAPS_MINFANISOTROPIC     = 0x00040000u,
    D3DPTFILTERCAPS_MIPFPOINT           = 0x01000000u,
    D3DPTFILTERCAPS_MIPFLINEAR          = 0x02000000u
};

enum D3DTEXOPCAPS : std::uint32_t
{
    D3DTEXOPCAPS_ADD                     = 0x00000001u,
    D3DTEXOPCAPS_SUBTRACT                = 0x00000002u,
    D3DTEXOPCAPS_ADDSIGNED               = 0x00000004u,
    D3DTEXOPCAPS_ADDSIGNED2X             = 0x00000008u,
    D3DTEXOPCAPS_ADDSMOOTH               = 0x00000010u,
    D3DTEXOPCAPS_BLENDDIFFUSEALPHA       = 0x00000020u,
    D3DTEXOPCAPS_BLENDTEXTUREALPHA       = 0x00000040u,
    D3DTEXOPCAPS_BLENDFACTORALPHA        = 0x00000080u,
    D3DTEXOPCAPS_BLENDTEXTUREALPHAPM     = 0x00000100u,
    D3DTEXOPCAPS_BLENDCURRENTALPHA       = 0x00000200u,
    D3DTEXOPCAPS_PREMODULATE             = 0x00000400u,
    D3DTEXOPCAPS_MODULATE                = 0x00000800u,
    D3DTEXOPCAPS_MODULATE2X              = 0x00001000u,
    D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR  = 0x00002000u,
    D3DTEXOPCAPS_DOTPRODUCT3             = 0x00004000u,
    D3DTEXOPCAPS_BUMPENVMAP              = 0x00008000u,
    D3DTEXOPCAPS_BUMPENVMAPLUMINANCE     = 0x00010000u
};

enum D3DVTXPCAPS : std::uint32_t
{
    D3DVTXPCAPS_TEXGEN                = 0x00000100u,
    D3DVTXPCAPS_MATERIALSOURCE7       = 0x00000200u,
    D3DVTXPCAPS_DIRECTIONALLIGHTS     = 0x00000400u,
    D3DVTXPCAPS_POSITIONALLIGHTS      = 0x00000800u,
    D3DVTXPCAPS_LOCALVIEWER           = 0x00001000u,
    D3DVTXPCAPS_TWEENING              = 0x00002000u,
    D3DVTXPCAPS_NO_VSDT_UBYTE4        = 0x00004000u,
    D3DVTXPCAPS_TEXGEN_SPHEREMAP      = 0x00010000u,
    D3DVTXPCAPS_TEXGEN_CUBEMAP        = 0x00020000u
};

enum D3DDEVCAPS : std::uint32_t
{
    D3DDEVCAPS_EXECUTESYSTEMMEM     = 0x00000010u,
    D3DDEVCAPS_EXECUTEVIDEOMEM      = 0x00000020u,
    D3DDEVCAPS_TLVERTEXSYSTEMMEM   = 0x00000040u,
    D3DDEVCAPS_TLVERTEXVIDEOMEM    = 0x00000080u,
    D3DDEVCAPS_TEXTURESYSTEMMEM    = 0x00000100u,
    D3DDEVCAPS_TEXTUREVIDEOMEM     = 0x00000200u,
    D3DDEVCAPS_DRAWPRIMTLVERTEX    = 0x00000400u,
    D3DDEVCAPS_CANRENDERAFTERFLIP  = 0x00000800u,
    D3DDEVCAPS_TEXTURENONLOCALVIDMEM = 0x00001000u,
    D3DDEVCAPS_HWTRANSFORMANDLIGHT = 0x00002000u,
    D3DDEVCAPS_HWRASTERIZATION     = 0x00004000u,
    D3DDEVCAPS_NPATCHES            = 0x00008000u,
    D3DDEVCAPS_PUREDEVICE          = 0x00010000u
};

enum D3DPRASTERCAPS : std::uint32_t
{
    D3DPRASTERCAPS_DITHER                = 0x00000002u,
    D3DPRASTERCAPS_ZBUFFERLESSHSR        = 0x00000008u,
    D3DPRASTERCAPS_FOGVERTEX             = 0x00000040u,
    D3DPRASTERCAPS_FOGTABLE              = 0x00000080u,
    D3DPRASTERCAPS_FOGRANGE              = 0x00000100u,
    D3DPRASTERCAPS_WFOG                  = 0x00001000u,
    D3DPRASTERCAPS_ZFOG                  = 0x00002000u,
    D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE = 0x00004000u
};

enum D3DPMISCCAPS : std::uint32_t
{
    D3DPMISCCAPS_COLORWRITEENABLE = 0x00000040u
};

enum D3DFORMAT : std::uint32_t
{
    D3DFMT_UNKNOWN       = 0,
    D3DFMT_R8G8B8        = 20,
    D3DFMT_A8R8G8B8      = 21,
    D3DFMT_X8R8G8B8      = 22,
    D3DFMT_R5G6B5        = 23,
    D3DFMT_X1R5G5B5      = 24,
    D3DFMT_A1R5G5B5      = 25,
    D3DFMT_A4R4G4B4      = 26,
    D3DFMT_R3G3B2        = 27,
    D3DFMT_A8            = 28,
    D3DFMT_A8R3G3B2      = 29,
    D3DFMT_X4R4G4B4      = 30,
    D3DFMT_A2B10G10R10   = 31,
    D3DFMT_G16R16        = 34,
    D3DFMT_A8P8          = 40,
    D3DFMT_P8            = 41,
    D3DFMT_L8            = 50,
    D3DFMT_A8L8          = 51,
    D3DFMT_A4L4          = 52,
    D3DFMT_V8U8          = 60,
    D3DFMT_L6V5U5        = 61,
    D3DFMT_X8L8V8U8      = 62,
    D3DFMT_Q8W8V8U8      = 63,
    D3DFMT_V16U16        = 64,
    D3DFMT_W11V11U10     = 65,
    D3DFMT_A2W10V10U10   = 67,
    D3DFMT_D16_LOCKABLE  = 70,
    D3DFMT_D32           = 71,
    D3DFMT_D15S1         = 73,
    D3DFMT_D24S8         = 75,
    D3DFMT_D24X8         = 77,
    D3DFMT_D24X4S4       = 79,
    D3DFMT_D16           = 80,
    D3DFMT_VERTEXDATA    = 100,
    D3DFMT_INDEX16       = 101,
    D3DFMT_INDEX32       = 102,
    D3DFMT_UYVY          = MAKEFOURCC('U','Y','V','Y'),
    D3DFMT_YUY2          = MAKEFOURCC('Y','U','Y','2'),
    D3DFMT_DXT1          = MAKEFOURCC('D','X','T','1'),
    D3DFMT_DXT2          = MAKEFOURCC('D','X','T','2'),
    D3DFMT_DXT3          = MAKEFOURCC('D','X','T','3'),
    D3DFMT_DXT4          = MAKEFOURCC('D','X','T','4'),
    D3DFMT_DXT5          = MAKEFOURCC('D','X','T','5'),
    D3DFMT_FORCE_DWORD   = 0xffffffffu
};

#endif // !_WIN32

