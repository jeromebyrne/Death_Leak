#ifndef DEATHLEAK_ENGINE_PLATFORM_MACDXCOMPAT_H
#define DEATHLEAK_ENGINE_PLATFORM_MACDXCOMPAT_H

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <algorithm>
#include <chrono>
#include <vector>
#include <thread>

#include <SDL.h>

using BYTE = unsigned char;
using UINT = unsigned int;
using DWORD = unsigned long;
using ULONG = unsigned long;
using FLOAT = float;
using INT = int;
using BOOL = int;
using errno_t = int;
using HRESULT = long;
using LPCSTR = const char*;
using LPWSTR = wchar_t*;
using HANDLE = void*;
using HINSTANCE = void*;
using HWND = void*;
using LPVOID = void*;
using WNDPROC = void*;
using SHORT = short;
using LONG = long;
constexpr BOOL TRUE = 1;
constexpr BOOL FALSE = 0;
constexpr DWORD ERROR_SUCCESS = 0;

constexpr HRESULT S_OK = 0;
constexpr HRESULT E_FAIL = static_cast<HRESULT>(0x80004005L);
constexpr int MB_OK = 0;

#ifndef TEXT
#define TEXT(x) L##x
#endif

inline bool SUCCEEDED(HRESULT hr)
{
    return hr >= 0;
}

inline bool FAILED(HRESULT hr)
{
    return hr < 0;
}

#ifndef ZeroMemory
#define ZeroMemory(Destination, Length) std::memset((Destination), 0, (Length))
#endif

struct D3DXMATRIX
{
    FLOAT m[4][4];

    operator FLOAT*() { return &m[0][0]; }
    operator const FLOAT*() const { return &m[0][0]; }
};

struct D3DXVECTOR2
{
    FLOAT x;
    FLOAT y;

    D3DXVECTOR2(FLOAT xValue = 0.0f, FLOAT yValue = 0.0f): x(xValue), y(yValue) {}
};

struct D3DXVECTOR3
{
    FLOAT x;
    FLOAT y;
    FLOAT z;

    D3DXVECTOR3(FLOAT xValue = 0.0f, FLOAT yValue = 0.0f, FLOAT zValue = 0.0f): x(xValue), y(yValue), z(zValue) {}
};

struct D3DXVECTOR4
{
    FLOAT x;
    FLOAT y;
    FLOAT z;
    FLOAT w;

    D3DXVECTOR4(FLOAT xValue = 0.0f, FLOAT yValue = 0.0f, FLOAT zValue = 0.0f, FLOAT wValue = 0.0f): x(xValue), y(yValue), z(zValue), w(wValue) {}
    operator FLOAT*() { return &x; }
    operator const FLOAT*() const { return &x; }
};

struct D3DXCOLOR
{
    FLOAT r;
    FLOAT g;
    FLOAT b;
    FLOAT a;

    D3DXCOLOR(FLOAT red = 0.0f, FLOAT green = 0.0f, FLOAT blue = 0.0f, FLOAT alpha = 1.0f):
        r(red),
        g(green),
        b(blue),
        a(alpha)
    {
    }
};

struct RECT
{
    LONG left = 0;
    LONG top = 0;
    LONG right = 0;
    LONG bottom = 0;
};

struct POINT
{
    LONG x = 0;
    LONG y = 0;
};

constexpr int DT_NOCLIP = 0x00000100;
constexpr int DT_VCENTER = 0x00000004;
constexpr int DT_TOP = 0x00000000;
constexpr int DT_BOTTOM = 0x00000008;
constexpr int DT_CENTER = 0x00000001;
constexpr int DT_WORDBREAK = 0x00000010;
constexpr int DT_LEFT = 0x00000000;

constexpr int OUT_DEFAULT_PRECIS = 0;
constexpr int DEFAULT_QUALITY = 0;
constexpr int DEFAULT_PITCH = 0;
constexpr int FF_DONTCARE = 0;

constexpr int VK_ESCAPE = 0x1B;
constexpr int VK_SPACE = 0x20;
constexpr int VK_RETURN = 0x0D;
constexpr int VK_TAB = 0x09;
constexpr int VK_LEFT = 0x25;
constexpr int VK_UP = 0x26;
constexpr int VK_RIGHT = 0x27;
constexpr int VK_DOWN = 0x28;
constexpr int VK_CONTROL = 0x11;
constexpr int VK_LBUTTON = 0x01;
constexpr int VK_RBUTTON = 0x02;
constexpr int VK_MBUTTON = 0x04;
constexpr int VK_DELETE = 0x2E;
constexpr int VK_LSHIFT = 0xA0;
constexpr size_t _TRUNCATE = static_cast<size_t>(-1);

inline SDL_Scancode MapVirtualKeyToScancode(int key)
{
    switch (key)
    {
        case VK_ESCAPE: return SDL_SCANCODE_ESCAPE;
        case VK_SPACE: return SDL_SCANCODE_SPACE;
        case VK_RETURN: return SDL_SCANCODE_RETURN;
        case VK_TAB: return SDL_SCANCODE_TAB;
        case VK_LEFT: return SDL_SCANCODE_LEFT;
        case VK_UP: return SDL_SCANCODE_UP;
        case VK_RIGHT: return SDL_SCANCODE_RIGHT;
        case VK_DOWN: return SDL_SCANCODE_DOWN;
        case VK_CONTROL: return SDL_SCANCODE_LCTRL;
        case VK_DELETE: return SDL_SCANCODE_DELETE;
        case VK_LSHIFT: return SDL_SCANCODE_LSHIFT;
        default:
            break;
    }

    if (key >= 'A' && key <= 'Z')
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (key - 'A'));
    }

    if (key >= '0' && key <= '9')
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_0 + (key - '0'));
    }

    return SDL_SCANCODE_UNKNOWN;
}

inline SHORT GetAsyncKeyState(int key)
{
    const Uint8* keyboard = SDL_GetKeyboardState(nullptr);
    if (keyboard != nullptr)
    {
        const SDL_Scancode scancode = MapVirtualKeyToScancode(key);
        if (scancode != SDL_SCANCODE_UNKNOWN && keyboard[scancode] != 0)
        {
            return static_cast<SHORT>(0x8000);
        }
    }

    Uint32 mouseButtons = SDL_GetMouseState(nullptr, nullptr);
    switch (key)
    {
        case VK_LBUTTON:
            return (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 ? static_cast<SHORT>(0x8000) : 0;
        case VK_RBUTTON:
            return (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 ? static_cast<SHORT>(0x8000) : 0;
        case VK_MBUTTON:
            return (mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0 ? static_cast<SHORT>(0x8000) : 0;
        default:
            return 0;
    }
}

inline void Sleep(unsigned long milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline BOOL GetCursorPos(POINT* point)
{
    if (point != nullptr)
    {
        point->x = 0;
        point->y = 0;
    }
    return TRUE;
}

inline BOOL ScreenToClient(HWND, POINT*)
{
    return TRUE;
}

inline errno_t mbstowcs_s(size_t* convertedChars, wchar_t* wcstring, size_t sizeInWords, const char* mbstring, size_t)
{
    if (convertedChars != nullptr)
    {
        *convertedChars = 0;
    }
    if (wcstring == nullptr || mbstring == nullptr || sizeInWords == 0)
    {
        return 1;
    }

    std::mbstate_t state{};
    const char* src = mbstring;
    size_t result = std::mbsrtowcs(wcstring, &src, sizeInWords - 1, &state);
    if (result == static_cast<size_t>(-1))
    {
        wcstring[0] = L'\0';
        return 1;
    }
    wcstring[result] = L'\0';
    if (convertedChars != nullptr)
    {
        *convertedChars = result;
    }
    return 0;
}

struct XINPUT_GAMEPAD
{
    UINT wButtons = 0;
    SHORT sThumbLX = 0;
    SHORT sThumbLY = 0;
    SHORT sThumbRX = 0;
    SHORT sThumbRY = 0;
    BYTE bLeftTrigger = 0;
    BYTE bRightTrigger = 0;
};

struct XINPUT_STATE
{
    UINT dwPacketNumber = 0;
    XINPUT_GAMEPAD Gamepad;
};

struct XINPUT_VIBRATION
{
    UINT wLeftMotorSpeed = 0;
    UINT wRightMotorSpeed = 0;
};

constexpr UINT XINPUT_GAMEPAD_DPAD_UP = 0x0001;
constexpr UINT XINPUT_GAMEPAD_DPAD_DOWN = 0x0002;
constexpr UINT XINPUT_GAMEPAD_DPAD_LEFT = 0x0004;
constexpr UINT XINPUT_GAMEPAD_DPAD_RIGHT = 0x0008;
constexpr UINT XINPUT_GAMEPAD_START = 0x0010;
constexpr UINT XINPUT_GAMEPAD_BACK = 0x0020;
constexpr UINT XINPUT_GAMEPAD_LEFT_THUMB = 0x0040;
constexpr UINT XINPUT_GAMEPAD_RIGHT_THUMB = 0x0080;
constexpr UINT XINPUT_GAMEPAD_LEFT_SHOULDER = 0x0100;
constexpr UINT XINPUT_GAMEPAD_RIGHT_SHOULDER = 0x0200;
constexpr UINT XINPUT_GAMEPAD_A = 0x1000;
constexpr UINT XINPUT_GAMEPAD_B = 0x2000;
constexpr UINT XINPUT_GAMEPAD_X = 0x4000;
constexpr UINT XINPUT_GAMEPAD_Y = 0x8000;
constexpr int XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE = 7849;
constexpr int XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE = 8689;
constexpr int XINPUT_GAMEPAD_TRIGGER_THRESHOLD = 30;

inline DWORD XInputGetState(DWORD, XINPUT_STATE*)
{
    return 1;
}

inline DWORD XInputSetState(DWORD, XINPUT_VIBRATION*)
{
    return 1;
}

namespace steamworks
{
    class ISteamUtils
    {
    public:
        bool IsSteamRunningOnSteamDeck() const { return false; }
        int GetAppID() const { return 0; }
    };
}

inline steamworks::ISteamUtils* SteamUtils()
{
    return nullptr;
}

struct D3DX10_FONT_DESC
{
    int Height = 0;
    int Width = 0;
    int Weight = 0;
    int MipLevels = 0;
    bool Italic = false;
    int CharSet = 0;
    int Quality = 0;
    int PitchAndFamily = 0;
    wchar_t FaceName[64] = {};
};

inline void D3DXMatrixIdentity(D3DXMATRIX* out)
{
    ZeroMemory(out, sizeof(D3DXMATRIX));
    out->m[0][0] = 1.0f;
    out->m[1][1] = 1.0f;
    out->m[2][2] = 1.0f;
    out->m[3][3] = 1.0f;
}

inline void D3DXMatrixTranslation(D3DXMATRIX* out, FLOAT x, FLOAT y, FLOAT z)
{
    D3DXMatrixIdentity(out);
    out->m[3][0] = x;
    out->m[3][1] = y;
    out->m[3][2] = z;
}

inline void D3DXMatrixScaling(D3DXMATRIX* out, FLOAT x, FLOAT y, FLOAT z)
{
    ZeroMemory(out, sizeof(D3DXMATRIX));
    out->m[0][0] = x;
    out->m[1][1] = y;
    out->m[2][2] = z;
    out->m[3][3] = 1.0f;
}

inline void D3DXMatrixRotationZ(D3DXMATRIX* out, FLOAT radians)
{
    D3DXMatrixIdentity(out);
    const FLOAT c = std::cos(radians);
    const FLOAT s = std::sin(radians);
    out->m[0][0] = c;
    out->m[0][1] = s;
    out->m[1][0] = -s;
    out->m[1][1] = c;
}

inline void D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* out, FLOAT yaw, FLOAT pitch, FLOAT roll)
{
    const FLOAT cy = std::cos(yaw);
    const FLOAT sy = std::sin(yaw);
    const FLOAT cp = std::cos(pitch);
    const FLOAT sp = std::sin(pitch);
    const FLOAT cr = std::cos(roll);
    const FLOAT sr = std::sin(roll);

    out->m[0][0] = cr * cy + sr * sp * sy;
    out->m[0][1] = sr * cp;
    out->m[0][2] = cr * -sy + sr * sp * cy;
    out->m[0][3] = 0.0f;

    out->m[1][0] = -sr * cy + cr * sp * sy;
    out->m[1][1] = cr * cp;
    out->m[1][2] = sr * sy + cr * sp * cy;
    out->m[1][3] = 0.0f;

    out->m[2][0] = cp * sy;
    out->m[2][1] = -sp;
    out->m[2][2] = cp * cy;
    out->m[2][3] = 0.0f;

    out->m[3][0] = 0.0f;
    out->m[3][1] = 0.0f;
    out->m[3][2] = 0.0f;
    out->m[3][3] = 1.0f;
}

inline void D3DXMatrixMultiply(D3DXMATRIX* out, const D3DXMATRIX* a, const D3DXMATRIX* b)
{
    D3DXMATRIX result{};
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int i = 0; i < 4; ++i)
            {
                result.m[row][col] += a->m[row][i] * b->m[i][col];
            }
        }
    }
    *out = result;
}

inline void D3DXMatrixOrthoLH(D3DXMATRIX* out, FLOAT width, FLOAT height, FLOAT zn, FLOAT zf)
{
    ZeroMemory(out, sizeof(D3DXMATRIX));
    out->m[0][0] = 2.0f / width;
    out->m[1][1] = 2.0f / height;
    out->m[2][2] = 1.0f / (zf - zn);
    out->m[3][2] = zn / (zn - zf);
    out->m[3][3] = 1.0f;
}

enum D3D10_PRIMITIVE_TOPOLOGY
{
    D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED = 0,
    D3D10_PRIMITIVE_TOPOLOGY_POINTLIST = 1,
    D3D10_PRIMITIVE_TOPOLOGY_LINELIST = 2,
    D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4,
    D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5
};

enum DXGI_FORMAT
{
    DXGI_FORMAT_UNKNOWN = 0,
    DXGI_FORMAT_R32_UINT = 42,
    DXGI_FORMAT_R32_FLOAT = 41,
    DXGI_FORMAT_R32G32_FLOAT = 16,
    DXGI_FORMAT_R32G32B32_FLOAT = 6,
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R8G8B8A8_UNORM = 28,
    DXGI_FORMAT_R32_TYPELESS = 1,
    DXGI_FORMAT_D32_FLOAT = 40
};

enum D3D10_INPUT_CLASSIFICATION
{
    D3D10_INPUT_PER_VERTEX_DATA = 0
};

enum D3D10_DRIVER_TYPE
{
    D3D10_DRIVER_TYPE_HARDWARE = 1,
    D3D10_DRIVER_TYPE_REFERENCE = 2
};

enum D3D10_USAGE
{
    D3D10_USAGE_DEFAULT = 0
};

constexpr UINT D3D10_SHADER_ENABLE_STRICTNESS = 0x00000001;
constexpr UINT D3D10_SHADER_DEBUG = 0x00000002;
constexpr UINT D3D10_APPEND_ALIGNED_ELEMENT = 0xffffffffu;

enum D3D10_BIND_FLAG
{
    D3D10_BIND_VERTEX_BUFFER = 0x1,
    D3D10_BIND_INDEX_BUFFER = 0x2,
    D3D10_BIND_CONSTANT_BUFFER = 0x4,
    D3D10_BIND_SHADER_RESOURCE = 0x8,
    D3D10_BIND_RENDER_TARGET = 0x20,
    D3D10_BIND_DEPTH_STENCIL = 0x40
};

enum D3D10_CPU_ACCESS_FLAG
{
    D3D10_CPU_ACCESS_NONE = 0
};

enum D3D10_RTV_DIMENSION
{
    D3D10_RTV_DIMENSION_TEXTURE2D = 1
};

enum D3D10_DSV_DIMENSION
{
    D3D10_DSV_DIMENSION_TEXTURE2D = 1
};

enum D3D10_SRV_DIMENSION
{
    D3D10_SRV_DIMENSION_TEXTURE2D = 1
};

enum D3D10_FILL_MODE
{
    D3D10_FILL_SOLID = 3
};

enum D3D10_CULL_MODE
{
    D3D10_CULL_NONE = 1
};

enum D3D10_BLEND
{
    D3D10_BLEND_ZERO = 1,
    D3D10_BLEND_SRC_ALPHA = 5,
    D3D10_BLEND_INV_SRC_ALPHA = 6
};

enum D3D10_BLEND_OP
{
    D3D10_BLEND_OP_ADD = 1
};

enum D3D10_COLOR_WRITE_ENABLE
{
    D3D10_COLOR_WRITE_ENABLE_ALL = 0x0f
};

enum D3D10_CLEAR_FLAG
{
    D3D10_CLEAR_DEPTH = 0x1
};

enum D3D10_DEPTH_WRITE_MASK
{
    D3D10_DEPTH_WRITE_MASK_ALL = 1
};

enum D3D10_COMPARISON_FUNC
{
    D3D10_COMPARISON_ALWAYS = 8
};

enum D3D10_STENCIL_OP
{
    D3D10_STENCIL_OP_KEEP = 1,
    D3D10_STENCIL_OP_INCR = 2,
    D3D10_STENCIL_OP_DECR = 3
};

enum DXGI_SWAP_EFFECT
{
    DXGI_SWAP_EFFECT_SEQUENTIAL = 3
};

enum DXGI_MODE_SCALING
{
    DXGI_MODE_SCALING_UNSPECIFIED = 0
};

struct D3D10_INPUT_ELEMENT_DESC
{
    const char* SemanticName;
    UINT SemanticIndex;
    DXGI_FORMAT Format;
    UINT InputSlot;
    UINT AlignedByteOffset;
    D3D10_INPUT_CLASSIFICATION InputSlotClass;
    UINT InstanceDataStepRate;
};

struct D3D10_BUFFER_DESC
{
    UINT ByteWidth = 0;
    D3D10_USAGE Usage = D3D10_USAGE_DEFAULT;
    UINT BindFlags = 0;
    UINT CPUAccessFlags = 0;
    UINT MiscFlags = 0;
};

struct D3D10_SUBRESOURCE_DATA
{
    const void* pSysMem = nullptr;
    UINT SysMemPitch = 0;
    UINT SysMemSlicePitch = 0;
};

struct D3D10_TEXTURE2D_DESC
{
    UINT Width = 0;
    UINT Height = 0;
    UINT MipLevels = 1;
    UINT ArraySize = 1;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    struct { UINT Count = 1; UINT Quality = 0; } SampleDesc;
    D3D10_USAGE Usage = D3D10_USAGE_DEFAULT;
    UINT BindFlags = 0;
    UINT CPUAccessFlags = 0;
    UINT MiscFlags = 0;
};

struct D3D10_DEPTH_STENCIL_VIEW_DESC
{
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    D3D10_DSV_DIMENSION ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    struct { UINT MipSlice = 0; } Texture2D;
};

struct D3D10_SHADER_RESOURCE_VIEW_DESC
{
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    D3D10_SRV_DIMENSION ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    struct { UINT MostDetailedMip = 0; UINT MipLevels = 1; } Texture2D;
};

struct D3D10_RENDER_TARGET_VIEW_DESC
{
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    D3D10_RTV_DIMENSION ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    struct { UINT MipSlice = 0; } Texture2D;
};

struct D3D10_VIEWPORT
{
    UINT TopLeftX = 0;
    UINT TopLeftY = 0;
    UINT Width = 0;
    UINT Height = 0;
    FLOAT MinDepth = 0.0f;
    FLOAT MaxDepth = 1.0f;
};

struct D3D10_DEPTH_STENCILOP_DESC
{
    D3D10_STENCIL_OP StencilFailOp = D3D10_STENCIL_OP_KEEP;
    D3D10_STENCIL_OP StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
    D3D10_STENCIL_OP StencilPassOp = D3D10_STENCIL_OP_KEEP;
    D3D10_COMPARISON_FUNC StencilFunc = D3D10_COMPARISON_ALWAYS;
};

struct D3D10_DEPTH_STENCIL_DESC
{
    BOOL DepthEnable = TRUE;
    D3D10_DEPTH_WRITE_MASK DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
    D3D10_COMPARISON_FUNC DepthFunc = D3D10_COMPARISON_ALWAYS;
    BOOL StencilEnable = FALSE;
    UINT StencilReadMask = 0;
    UINT StencilWriteMask = 0;
    D3D10_DEPTH_STENCILOP_DESC FrontFace;
    D3D10_DEPTH_STENCILOP_DESC BackFace;
};

struct D3D10_RASTERIZER_DESC
{
    D3D10_FILL_MODE FillMode = D3D10_FILL_SOLID;
    D3D10_CULL_MODE CullMode = D3D10_CULL_NONE;
};

struct D3D10_BLEND_DESC
{
    BOOL AlphaToCoverageEnable = FALSE;
    BOOL BlendEnable[8] = { FALSE };
    D3D10_BLEND SrcBlend = D3D10_BLEND_SRC_ALPHA;
    D3D10_BLEND DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
    D3D10_BLEND_OP BlendOp = D3D10_BLEND_OP_ADD;
    D3D10_BLEND SrcBlendAlpha = D3D10_BLEND_ZERO;
    D3D10_BLEND DestBlendAlpha = D3D10_BLEND_ZERO;
    D3D10_BLEND_OP BlendOpAlpha = D3D10_BLEND_OP_ADD;
    UINT RenderTargetWriteMask[8] = { D3D10_COLOR_WRITE_ENABLE_ALL };
};

struct D3D10_SWAP_CHAIN_DESC_BUFFER_DESC
{
    UINT Width = 0;
    UINT Height = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    struct { UINT Numerator = 60; UINT Denominator = 1; } RefreshRate;
    DXGI_MODE_SCALING Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
};

struct DXGI_SWAP_CHAIN_DESC
{
    D3D10_SWAP_CHAIN_DESC_BUFFER_DESC BufferDesc;
    struct { UINT Count = 1; UINT Quality = 0; } SampleDesc;
    UINT BufferUsage = 0;
    UINT BufferCount = 1;
    HWND OutputWindow = nullptr;
    BOOL Windowed = TRUE;
    UINT SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
    UINT Flags = 0;
};

constexpr UINT DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH = 0x00000002;

struct D3D10_PASS_DESC
{
    const void* pIAInputSignature = nullptr;
    std::size_t IAInputSignatureSize = 0;
};

struct D3D10_TECHNIQUE_DESC
{
    UINT Passes = 1;
};

class ID3D10Resource
{
public:
    virtual ~ID3D10Resource() = default;
    virtual ULONG Release() { delete this; return 0; }
};

class ID3D10Buffer : public ID3D10Resource { };
class ID3D10Texture2D : public ID3D10Resource
{
public:
    D3D10_TEXTURE2D_DESC m_desc{};

    virtual HRESULT GetDesc(D3D10_TEXTURE2D_DESC* desc)
    {
        if (desc != nullptr)
        {
            *desc = m_desc;
        }
        return S_OK;
    }
};
class ID3D10Device;
class ID3D10ShaderResourceView : public ID3D10Resource
{
public:
    virtual HRESULT GetResource(ID3D10Resource** out)
    {
        if (out != nullptr)
        {
            *out = new ID3D10Texture2D();
        }
        return S_OK;
    }
};
class ID3D10RenderTargetView : public ID3D10Resource { };
class ID3D10DepthStencilView : public ID3D10Resource { };
class ID3D10DepthStencilState : public ID3D10Resource { };
class ID3D10BlendState : public ID3D10Resource { };
class ID3D10RasterizerState : public ID3D10Resource { };
class ID3D10InputLayout : public ID3D10Resource { };
class ID3D10EffectVariable;
class ID3D10EffectTechnique;
class ID3D10EffectPass;
class ID3D10EffectMatrixVariable;
class ID3D10EffectScalarVariable;
class ID3D10EffectVectorVariable;
class ID3D10EffectShaderResourceVariable;
class ID3D10Effect : public ID3D10Resource
{
public:
    virtual ID3D10EffectVariable* GetVariableByName(const char*) { return nullptr; }
    virtual ID3D10EffectTechnique* GetTechniqueByName(const char*) { return nullptr; }
};
class ID3D10EffectTechnique : public ID3D10Resource
{
public:
    virtual HRESULT GetDesc(D3D10_TECHNIQUE_DESC* desc)
    {
        if (desc != nullptr)
        {
            desc->Passes = 1;
        }
        return S_OK;
    }
    virtual ID3D10EffectPass* GetPassByIndex(UINT) { return nullptr; }
};
class ID3D10EffectPass : public ID3D10Resource
{
public:
    virtual HRESULT GetDesc(D3D10_PASS_DESC* desc)
    {
        if (desc != nullptr)
        {
            desc->pIAInputSignature = nullptr;
            desc->IAInputSignatureSize = 0;
        }
        return S_OK;
    }
    virtual HRESULT Apply(UINT) { return S_OK; }
};
class ID3D10EffectVariable : public ID3D10Resource
{
public:
    virtual ID3D10EffectMatrixVariable* AsMatrix() { return nullptr; }
    virtual ID3D10EffectScalarVariable* AsScalar() { return nullptr; }
    virtual ID3D10EffectVectorVariable* AsVector() { return nullptr; }
    virtual ID3D10EffectShaderResourceVariable* AsShaderResource() { return nullptr; }
};
class ID3D10EffectScalarVariable : public ID3D10Resource
{
public:
    HRESULT SetFloat(FLOAT) { return S_OK; }
    HRESULT GetFloat(FLOAT* out)
    {
        if (out != nullptr)
        {
            *out = 0.0f;
        }
        return S_OK;
    }
    HRESULT SetInt(int) { return S_OK; }
    HRESULT GetInt(int* out)
    {
        if (out != nullptr)
        {
            *out = 0;
        }
        return S_OK;
    }
    HRESULT SetBool(BOOL) { return S_OK; }
    HRESULT GetBool(BOOL* out)
    {
        if (out != nullptr)
        {
            *out = FALSE;
        }
        return S_OK;
    }
};
class ID3D10EffectMatrixVariable : public ID3D10Resource
{
public:
    HRESULT SetMatrix(const FLOAT*) { return S_OK; }
};
class ID3D10EffectVectorVariable : public ID3D10Resource
{
public:
    HRESULT SetFloatVector(const FLOAT*) { return S_OK; }
};
class ID3D10EffectShaderResourceVariable : public ID3D10Resource
{
public:
    HRESULT SetResource(ID3D10ShaderResourceView*) { return S_OK; }
};
class ID3D10EffectTechnique1 : public ID3D10EffectTechnique { };
class ID3DX10Font : public ID3D10Resource
{
public:
    HRESULT DrawText(void*, const wchar_t*, int, RECT*, int, D3DXCOLOR) { return S_OK; }
    HRESULT DrawTextA(void*, const char*, int, RECT*, int, D3DXCOLOR) { return S_OK; }
};

inline HRESULT D3DX10CreateFontIndirect(ID3D10Device*, const D3DX10_FONT_DESC*, ID3DX10Font** out)
{
    if (out != nullptr)
    {
        *out = new ID3DX10Font();
    }
    return S_OK;
}

inline HRESULT D3DX10CreateShaderResourceViewFromFile(ID3D10Device*, const wchar_t*, void*, void*, ID3D10ShaderResourceView** out, void*)
{
    if (out != nullptr)
    {
        *out = new ID3D10ShaderResourceView();
    }
    return S_OK;
}

inline HRESULT D3DX10CreateEffectFromFile(const wchar_t*, void*, void*, const char*, UINT, UINT, ID3D10Device*, void*, void*, ID3D10Effect** out, void*, void*)
{
    if (out != nullptr)
    {
        *out = new ID3D10Effect();
    }
    return S_OK;
}

class ID3D10Device
{
public:
    virtual ~ID3D10Device() = default;

    virtual HRESULT CreateBuffer(const D3D10_BUFFER_DESC*, const D3D10_SUBRESOURCE_DATA*, ID3D10Buffer** out)
    {
        if (out != nullptr) { *out = new ID3D10Buffer(); }
        return S_OK;
    }

    virtual HRESULT CreateTexture2D(const D3D10_TEXTURE2D_DESC* desc, const void*, ID3D10Texture2D** out)
    {
        if (out != nullptr)
        {
            auto* texture = new ID3D10Texture2D();
            if (texture != nullptr)
            {
                texture->m_desc = desc != nullptr ? *desc : D3D10_TEXTURE2D_DESC{};
            }
            *out = texture;
        }
        return S_OK;
    }

    virtual HRESULT CreateDepthStencilState(const D3D10_DEPTH_STENCIL_DESC*, ID3D10DepthStencilState** out)
    {
        if (out != nullptr) { *out = new ID3D10DepthStencilState(); }
        return S_OK;
    }

    virtual HRESULT CreateDepthStencilView(ID3D10Texture2D*, const D3D10_DEPTH_STENCIL_VIEW_DESC*, ID3D10DepthStencilView** out)
    {
        if (out != nullptr) { *out = new ID3D10DepthStencilView(); }
        return S_OK;
    }

    virtual HRESULT CreateShaderResourceView(ID3D10Texture2D*, const D3D10_SHADER_RESOURCE_VIEW_DESC*, ID3D10ShaderResourceView** out)
    {
        if (out != nullptr) { *out = new ID3D10ShaderResourceView(); }
        return S_OK;
    }

    virtual HRESULT CreateRenderTargetView(ID3D10Texture2D*, const D3D10_RENDER_TARGET_VIEW_DESC*, ID3D10RenderTargetView** out)
    {
        if (out != nullptr) { *out = new ID3D10RenderTargetView(); }
        return S_OK;
    }

    virtual HRESULT CreateBlendState(const D3D10_BLEND_DESC*, ID3D10BlendState** out)
    {
        if (out != nullptr) { *out = new ID3D10BlendState(); }
        return S_OK;
    }

    virtual HRESULT CreateRasterizerState(const D3D10_RASTERIZER_DESC*, ID3D10RasterizerState** out)
    {
        if (out != nullptr) { *out = new ID3D10RasterizerState(); }
        return S_OK;
    }
    virtual HRESULT CreateInputLayout(const D3D10_INPUT_ELEMENT_DESC*, UINT, const void*, std::size_t, ID3D10InputLayout** out)
    {
        if (out != nullptr) { *out = new ID3D10InputLayout(); }
        return S_OK;
    }

    virtual void IASetInputLayout(ID3D10InputLayout*) {}

    virtual void IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY) {}
    virtual void IASetIndexBuffer(ID3D10Buffer*, DXGI_FORMAT, UINT) {}
    virtual void IASetVertexBuffers(UINT, UINT, ID3D10Buffer* const*, const UINT*, const UINT*) {}
    virtual void Draw(UINT, UINT) {}
    virtual void DrawIndexed(UINT, UINT, INT) {}
    virtual void OMSetRenderTargets(UINT, ID3D10RenderTargetView* const*, ID3D10DepthStencilView*) {}
    virtual void OMSetDepthStencilState(ID3D10DepthStencilState*, UINT) {}
    virtual void OMSetBlendState(ID3D10BlendState*, const FLOAT[4], UINT) {}
    virtual void RSSetState(ID3D10RasterizerState*) {}
    virtual void RSSetViewports(UINT, const D3D10_VIEWPORT*) {}
    virtual void ClearRenderTargetView(ID3D10RenderTargetView*, const FLOAT[4]) {}
    virtual void ClearDepthStencilView(ID3D10DepthStencilView*, UINT, FLOAT, UINT) {}
    virtual void ClearState() {}
    virtual ULONG Release() { delete this; return 0; }
};

class IDXGISwapChain
{
public:
    virtual ~IDXGISwapChain() = default;
    virtual HRESULT Present(UINT, UINT) { return S_OK; }
    virtual HRESULT SetFullscreenState(BOOL, void*) { return S_OK; }
    virtual HRESULT GetBuffer(UINT, const void*, LPVOID* out)
    {
        if (out != nullptr)
        {
            *out = new ID3D10Texture2D();
        }
        return S_OK;
    }
    virtual ULONG Release() { delete this; return 0; }
};

inline int MessageBoxExW(HWND, const wchar_t*, const wchar_t*, UINT, UINT)
{
    return 0;
}

inline int MessageBox(HWND, const wchar_t*, const wchar_t*, UINT)
{
    return 0;
}

inline HRESULT D3D10CreateDeviceAndSwapChain(
    void*,
    D3D10_DRIVER_TYPE,
    void*,
    UINT,
    UINT,
    const DXGI_SWAP_CHAIN_DESC*,
    IDXGISwapChain** swapChain,
    ID3D10Device** device)
{
    if (swapChain != nullptr)
    {
        *swapChain = new IDXGISwapChain();
    }
    if (device != nullptr)
    {
        *device = new ID3D10Device();
    }
    return S_OK;
}

namespace irrklang
{
    class ISound
    {
    public:
        virtual ~ISound() = default;
        virtual void setPlaybackSpeed(float) {}
        virtual void setVolume(float) {}
        virtual void setPan(float) {}
        virtual void setPlayPosition(unsigned int) {}
        virtual void setIsPaused(bool = true) {}
        virtual bool isFinished() const { return true; }
        virtual void stop() {}
        virtual void drop() { delete this; }
        virtual void setIsLooped(bool) {}
        virtual float getVolume() const { return 1.0f; }
        virtual bool getIsPaused() const { return false; }
    };

    class ISoundEngine
    {
    public:
        virtual ~ISoundEngine() = default;
        virtual ISound* play2D(const char*, bool loop) { return play2D(nullptr, loop, false, false); }
        virtual ISound* play2D(const char*, bool, bool, bool) { return new ISound(); }
        virtual void setSoundVolume(float) {}
        virtual void stopAllSounds() {}
        virtual void addSoundSourceFromFile(const char*, int, bool) {}
        virtual void drop() { delete this; }
    };

    inline ISoundEngine* createIrrKlangDevice(int)
    {
        return new ISoundEngine();
    }

    constexpr int ESOD_AUTO_DETECT = 0;
    constexpr int ESM_AUTO_DETECT = 0;
}

#endif
