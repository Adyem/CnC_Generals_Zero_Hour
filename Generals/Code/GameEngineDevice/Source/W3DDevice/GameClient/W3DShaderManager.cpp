#if defined(_UNIX)

#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"
#include "WW3D2/WW3DFormat.h"
#include "GameClient/FilterTypes.h"
#include "Common/GameLOD.h"
#include "d3d8.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

TextureClass *W3DShaderManager::m_Textures[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
ChipsetType W3DShaderManager::m_currentChipset = static_cast<ChipsetType>(0);
W3DShaderManager::ShaderTypes W3DShaderManager::m_currentShader = W3DShaderManager::ST_INVALID;
Int W3DShaderManager::m_currentShaderPass = 0;
std::map<W3DShaderManager::ShaderTypes, W3DShaderManager::BgfxProgramDefinition> W3DShaderManager::m_bgfxPrograms;
FilterTypes W3DShaderManager::m_currentFilter = static_cast<FilterTypes>(0);
Bool W3DShaderManager::m_renderingToTexture = false;
IDirect3DSurface8 *W3DShaderManager::m_oldRenderSurface = NULL;
IDirect3DTexture8 *W3DShaderManager::m_renderTexture = NULL;
IDirect3DSurface8 *W3DShaderManager::m_newRenderSurface = NULL;
IDirect3DSurface8 *W3DShaderManager::m_oldDepthSurface = NULL;
TextureClass *W3DShaderManager::m_renderTextureWrapper = NULL;
WW3DFormat W3DShaderManager::m_renderTextureFormat = WW3D_FORMAT_UNKNOWN;

W3DShaderManager::BgfxProgramDefinition::BgfxProgramDefinition()
        : m_preload(FALSE)
#if WW3D_BGFX_AVAILABLE
        , m_programHandle(BGFX_INVALID_HANDLE)
#endif
{
}

W3DShaderManager::BgfxProgramDefinition::BgfxProgramDefinition(const std::string &vertexPath,
        const std::string &fragmentPath, Bool preload)
        : m_vertexShaderPath(vertexPath),
          m_fragmentShaderPath(fragmentPath),
          m_preload(preload)
#if WW3D_BGFX_AVAILABLE
        , m_programHandle(BGFX_INVALID_HANDLE)
#endif
{
}

Bool W3DShaderManager::BgfxProgramDefinition::isValid(void) const
{
        return !m_vertexShaderPath.empty() && !m_fragmentShaderPath.empty();
}

void W3DShaderManager::BgfxProgramDefinition::setSourcePaths(const std::string &vertexSourcePath,
        const std::string &fragmentSourcePath)
{
        m_vertexShaderSourcePath = vertexSourcePath;
        m_fragmentShaderSourcePath = fragmentSourcePath;
}

void W3DShaderManager::BgfxProgramDefinition::setVaryingPath(const std::string &varyingPath)
{
        m_varyingDefPath = varyingPath;
}

void W3DShaderManager::BgfxProgramDefinition::setShaderProfiles(const std::string &vertexProfile,
        const std::string &fragmentProfile)
{
        m_vertexShaderProfile = vertexProfile;
        m_fragmentShaderProfile = fragmentProfile;
}

void W3DShaderManager::BgfxProgramDefinition::addSampler(const std::string &name, std::uint8_t stage)
{
        m_samplerBindings.push_back({ name, stage });
}

void W3DShaderManager::BgfxProgramDefinition::addUniform(const std::string &name,
        const std::string &type, std::uint16_t arraySize)
{
        m_uniformBindings.push_back({ name, type, arraySize });
}

W3DShaderManager::W3DShaderManager(void) = default;

void W3DShaderManager::init(void)
{
        std::fill(std::begin(m_Textures), std::end(m_Textures), static_cast<TextureClass *>(NULL));
        m_currentShader = ST_INVALID;
        m_currentShaderPass = 0;
        m_renderingToTexture = false;
        m_renderTextureWrapper = NULL;
        m_renderTextureFormat = WW3D_FORMAT_UNKNOWN;
}

void W3DShaderManager::shutdown(void)
{
        m_bgfxPrograms.clear();
        m_renderTexture = NULL;
        m_renderTextureWrapper = NULL;
        m_oldRenderSurface = NULL;
        m_newRenderSurface = NULL;
        m_oldDepthSurface = NULL;
        m_renderingToTexture = false;
}

ChipsetType W3DShaderManager::getChipset(void)
{
        return m_currentChipset;
}

Int W3DShaderManager::getShaderPasses(ShaderTypes)
{
        return 1;
}

Int W3DShaderManager::setShader(ShaderTypes shader, Int pass)
{
        m_currentShader = shader;
        m_currentShaderPass = pass;
        return TRUE;
}

void W3DShaderManager::resetShader(ShaderTypes shader)
{
        if (m_currentShader == shader)
        {
                m_currentShader = ST_INVALID;
                m_currentShaderPass = 0;
        }
}

void W3DShaderManager::registerBgfxProgram(ShaderTypes shader, const BgfxProgramDefinition &definition)
{
        if (!definition.isValid())
        {
                return;
        }

        m_bgfxPrograms[shader] = definition;
}

const W3DShaderManager::BgfxProgramDefinition *W3DShaderManager::getBgfxProgram(ShaderTypes shader)
{
        auto it = m_bgfxPrograms.find(shader);
        if (it == m_bgfxPrograms.end())
        {
                return NULL;
        }
        return &it->second;
}

HRESULT W3DShaderManager::LoadAndCreateD3DShader(char *, const DWORD *, DWORD, Bool, DWORD *)
{
        return D3DERR_NOTAVAILABLE;
}

Bool W3DShaderManager::testMinimumRequirements(ChipsetType *videoChipType, CpuType *cpuType, Int *cpuFreq,
        Int *numRAM, Real *intBenchIndex, Real *floatBenchIndex, Real *memBenchIndex)
{
        if (videoChipType)
        {
                *videoChipType = m_currentChipset;
        }
        if (cpuType)
        {
                *cpuType = static_cast<CpuType>(0);
        }
        if (cpuFreq)
        {
                *cpuFreq = 0;
        }
        if (numRAM)
        {
                *numRAM = 0;
        }
        if (intBenchIndex)
        {
                *intBenchIndex = 0.0f;
        }
        if (floatBenchIndex)
        {
                *floatBenchIndex = 0.0f;
        }
        if (memBenchIndex)
        {
                *memBenchIndex = 0.0f;
        }
        return FALSE;
}

StaticGameLODLevel W3DShaderManager::getGPUPerformanceIndex(void)
{
        return static_cast<StaticGameLODLevel>(0);
}

Real W3DShaderManager::GetCPUBenchTime(void)
{
        return 0.0f;
}

Bool W3DShaderManager::filterPreRender(FilterTypes filter, Bool &skipRender, CustomScenePassModes &scenePassMode)
{
        m_currentFilter = filter;
        skipRender = FALSE;
        scenePassMode = static_cast<CustomScenePassModes>(0);
        return FALSE;
}

Bool W3DShaderManager::filterPostRender(FilterTypes filter, enum FilterModes, Coord2D &, Bool &doExtraRender)
{
        m_currentFilter = filter;
        doExtraRender = FALSE;
        return FALSE;
}

Bool W3DShaderManager::filterSetup(FilterTypes filter, enum FilterModes)
{
        m_currentFilter = filter;
        return FALSE;
}

void W3DShaderManager::startRenderToTexture(void)
{
        m_renderingToTexture = TRUE;
}

IDirect3DTexture8 *W3DShaderManager::endRenderToTexture(void)
{
        m_renderingToTexture = FALSE;
        return NULL;
}

IDirect3DTexture8 *W3DShaderManager::getRenderTexture(void)
{
        return m_renderTexture;
}

TextureClass *W3DShaderManager::getRenderTextureClass(void)
{
        return m_renderTextureWrapper;
}

void W3DShaderManager::drawViewport(Int)
{
}

void W3DShaderManager::initializeDefaultBgfxPrograms(void)
{
}

void W3DShaderManager::preloadBgfxPrograms(void)
{
}

void W3DShaderManager::unloadBgfxPrograms(void)
{
        m_bgfxPrograms.clear();
}

#if WW3D_BGFX_AVAILABLE
Bool W3DShaderManager::ensureBgfxProgramLoaded(ShaderTypes)
{
        return FALSE;
}

Bool W3DShaderManager::ensureBgfxShaderBinary(const std::string &, const std::string &, const std::string &,
        const std::string &, const char *)
{
        return FALSE;
}

void W3DShaderManager::destroyBgfxProgram(BgfxProgramDefinition &definition)
{
        definition.m_programHandle = BGFX_INVALID_HANDLE;
}

bgfx::ProgramHandle W3DShaderManager::loadBgfxProgram(BgfxProgramDefinition &definition)
{
        definition.m_programHandle = BGFX_INVALID_HANDLE;
        return BGFX_INVALID_HANDLE;
}
#endif

#else

#include "W3DShaderManager.win.inc"

#endif
