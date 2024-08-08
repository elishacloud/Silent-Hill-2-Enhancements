#include "Common/GfxUtils.h"

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_dds.h>

#include <cmath>
#include <vector>
#include <filesystem>

// DDS stuff
constexpr uint32_t  kFOURCC_DXT1        = MAKEFOURCC('D', 'X', 'T', '1');
constexpr uint32_t  kFOURCC_DXT2        = MAKEFOURCC('D', 'X', 'T', '2');
constexpr uint32_t  kFOURCC_DXT3        = MAKEFOURCC('D', 'X', 'T', '3');
constexpr uint32_t  kFOURCC_DXT4        = MAKEFOURCC('D', 'X', 'T', '4');
constexpr uint32_t  kFOURCC_DXT5        = MAKEFOURCC('D', 'X', 'T', '5');

// for the future ???
constexpr uint32_t  kFOURCC_ATI1        = MAKEFOURCC('A', 'T', 'I', '1');   // BC4
constexpr uint32_t  kFOURCC_ATI2        = MAKEFOURCC('A', 'T', 'I', '2');   // BC5

constexpr uint32_t  kDDS_Magic          = MAKEFOURCC('D', 'D', 'S', ' ');

constexpr uint32_t  kDDSD_CAPS          = 0x1;
constexpr uint32_t  kDDSD_HEIGHT        = 0x2;
constexpr uint32_t  kDDSD_WIDTH         = 0x4;
constexpr uint32_t  kDDSD_PITCH         = 0x8;
constexpr uint32_t  kDDSD_PIXELFORMAT   = 0x1000;
constexpr uint32_t  kDDSD_MIPMAPCOUNT   = 0x20000;
constexpr uint32_t  kDDSCAPS_TEXTURE    = 0x1000;
constexpr uint32_t  kDDPF_ALPHAPIXELS   = 0x1;
constexpr uint32_t  kDDPF_FOURCC        = 0x4;
constexpr uint32_t  kDDPF_RGB           = 0x40;


struct DDCOLORKEY {
    uint32_t        dwUnused0;
    uint32_t        dwUnused1;
};

struct DDPIXELFORMAT {
    uint32_t        dwSize;
    uint32_t        dwFlags;
    uint32_t        dwFourCC;
    uint32_t        dwRGBBitCount;
    uint32_t        dwRBitMask;
    uint32_t        dwGBitMask;
    uint32_t        dwBBitMask;
    uint32_t        dwRGBAlphaBitMask;
};

struct DDSCAPS2 {
    uint32_t        dwCaps;
    uint32_t        dwCaps2;
    uint32_t        dwCaps3;
    uint32_t        dwCaps4;
};

struct DDSURFACEDESC2 {
    uint32_t        dwMagic;
    uint32_t        dwSize;
    uint32_t        dwFlags;
    uint32_t        dwHeight;
    uint32_t        dwWidth;
    union {
        int         lPitch;
        uint32_t    dwLinearSize;
    };
    uint32_t        dwBackBufferCount;
    uint32_t        dwMipMapCount;
    uint32_t        dwAlphaBitDepth;
    uint32_t        dwUnused0;
    uint32_t        lpSurface;
    DDCOLORKEY      unused0;
    DDCOLORKEY      unused1;
    DDCOLORKEY      unused2;
    DDCOLORKEY      unused3;
    DDPIXELFORMAT   ddpfPixelFormat;
    DDSCAPS2        ddsCaps;
    uint32_t        dwUnused1;
};

struct DDS_HEADER_DXT10 {
    uint32_t        dxgiFormat;
    uint32_t        resourceDimension;
    uint32_t        miscFlag;
    uint32_t        arraySize;
    uint32_t        miscFlags2;
};


#define BC1_BLOCK_SIZE    8
#define BC2_BLOCK_SIZE    16
#define BC3_BLOCK_SIZE    16
#define BC4_BLOCK_SIZE    8
#define BC5_BLOCK_SIZE    16
#define BC6H_BLOCK_SIZE   16
#define BC7_BLOCK_SIZE    16


static size_t Log2I(size_t v) {
    size_t result = 0;
    while (v >>= 1) {
        ++result;
    }
    return result;
}

static void BuildMip(const uint8_t* srcImage, int srcWidth, int srcHeight, uint8_t* dstImage, int dstWidth, int dstHeight) {
    stbir_resize_uint8_generic(srcImage, srcWidth, srcHeight, 0,
                               dstImage, dstWidth, dstHeight, 0,
                               4, -1, 0, STBIR_EDGE_CLAMP,
                               STBIR_FILTER_MITCHELL, STBIR_COLORSPACE_LINEAR, nullptr);
}

static bool ExtensionEqual(LPCWSTR filePath, LPCWSTR extToCompare) {
    const size_t len = wcslen(filePath);
    return towlower(filePath[len - 3]) == towlower(extToCompare[0]) &&
           towlower(filePath[len - 2]) == towlower(extToCompare[1]) &&
           towlower(filePath[len - 1]) == towlower(extToCompare[2]);
}

static size_t ReadFromFile(HANDLE hFile, size_t bytesToRead, void* buffer) {
    size_t result = 0;

    const DWORD dwToRead = bytesToRead;
    DWORD bytesRead = 0;
    if (::ReadFile(hFile, buffer, dwToRead, &bytesRead, nullptr)) {
        result = static_cast<size_t>(bytesRead);
    }

    return result;
}




HRESULT GfxCreateTextureFromFileA(LPDIRECT3DDEVICE8 device, LPCSTR srcFile, LPDIRECT3DTEXTURE8* dstTexture, DWORD flags) {
    std::filesystem::path filePath = srcFile;
    return GfxCreateTextureFromFileW(device, filePath.wstring().c_str(), dstTexture, flags);
}

HRESULT GfxCreateTextureFromFileW(LPDIRECT3DDEVICE8 device, LPCWSTR srcFile, LPDIRECT3DTEXTURE8* dstTexture, DWORD flags) {
    if (!device || (!srcFile || !srcFile[0]) || !dstTexture) {
        return D3DERR_INVALIDCALL;
    }

    HANDLE fh = ::CreateFileW(srcFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!fh || fh == INVALID_HANDLE_VALUE) {
        return D3DERR_NOTAVAILABLE;
    }

    int width = 0, height = 0;
    UINT numMips = 1;
    uint8_t* pixels = nullptr;
    D3DFORMAT format = D3DFMT_UNKNOWN;
    size_t fullDDSDataSize = 0;

    const bool isDDS = ExtensionEqual(srcFile, L"dds");
    if (isDDS) {
        LARGE_INTEGER liFileSize{};
        ::GetFileSizeEx(fh, &liFileSize);

        if (liFileSize.QuadPart > sizeof(DDSURFACEDESC2)) {
            DDSURFACEDESC2 ddsDesc{};
            ReadFromFile(fh, sizeof(ddsDesc), &ddsDesc);

            if (ddsDesc.dwMagic == kDDS_Magic) {
                width = static_cast<int>(ddsDesc.dwWidth);
                height = static_cast<int>(ddsDesc.dwHeight);
                numMips = static_cast<int>((std::max)(ddsDesc.dwMipMapCount, 1u));

                if (ddsDesc.ddpfPixelFormat.dwFlags & kDDPF_FOURCC) {
                    switch (ddsDesc.ddpfPixelFormat.dwFourCC) {
                        case kFOURCC_DXT1:
                            format = D3DFMT_DXT1; break;
                        case kFOURCC_DXT2:
                            format = D3DFMT_DXT2; break;
                        case kFOURCC_DXT3:
                            format = D3DFMT_DXT3; break;
                        case kFOURCC_DXT4:
                            format = D3DFMT_DXT4; break;
                        case kFOURCC_DXT5:
                            format = D3DFMT_DXT5; break;
                        default:
                            format = D3DFMT_UNKNOWN;
                    }
                } else if (ddsDesc.ddpfPixelFormat.dwRGBBitCount == 32 && ddsDesc.ddpfPixelFormat.dwRGBAlphaBitMask == 0xFF000000 &&
                           ddsDesc.ddpfPixelFormat.dwRBitMask == 0x00FF0000 && ddsDesc.ddpfPixelFormat.dwGBitMask == 0x0000FF00 &&
                           ddsDesc.ddpfPixelFormat.dwBBitMask == 0x000000FF) {
                    format = D3DFMT_A8R8G8B8;
                }

                if (format != D3DFMT_UNKNOWN) {
                    fullDDSDataSize = liFileSize.QuadPart - sizeof(DDSURFACEDESC2);
                    pixels = reinterpret_cast<uint8_t*>(malloc(fullDDSDataSize));
                    ReadFromFile(fh, fullDDSDataSize, pixels);
                }
            }
        }
    } else {
        stbi_io_callbacks callbacks{
            // read - fill 'data' with 'size' bytes.  return number of bytes actually read
            [](void* user, char* data, int size) -> int {
                HANDLE file = static_cast<HANDLE>(user);
                const DWORD dwToRead = static_cast<DWORD>(size);
                DWORD bytesRead = 0;
                if (::ReadFile(file, data, dwToRead, &bytesRead, nullptr)) {
                    return static_cast<int>(bytesRead);
                } else {
                    return 0;
                }
            },
            // skip - skip the next 'n' bytes, or 'unget' the last -n bytes if negative
            [](void* user, int n) {
                HANDLE file = static_cast<HANDLE>(user);
                LARGE_INTEGER liDistanceToMove{};
                liDistanceToMove.QuadPart = static_cast<LONGLONG>(n);
                ::SetFilePointerEx(file, liDistanceToMove, nullptr, FILE_CURRENT);
            },
            // eof - returns nonzero if we are at end of file/data
            [](void* user) -> int {
                // there's no easy way to test for the EOF in WinApi, so doing it the hard way
                HANDLE file = static_cast<HANDLE>(user);
                LARGE_INTEGER liFileSize{};
                ::GetFileSizeEx(file, &liFileSize);

                LARGE_INTEGER liDistanceToMove{}, liNewFilePointer{};
                ::SetFilePointerEx(file, liDistanceToMove, &liNewFilePointer, FILE_CURRENT);

                return (liFileSize.QuadPart == liNewFilePointer.QuadPart) ? 1 : 0;
            },
        };

        int channels = 0;
        pixels = stbi_load_from_callbacks(&callbacks, fh, &width, &height, &channels, STBI_rgb_alpha);

        if (pixels) {
            // convert ABGR -> ARGB
            for (int i = 0, numPixels = width * height; i < numPixels; ++i) {
                std::swap(pixels[i * 4 + 0], pixels[i * 4 + 2]);
            }
        }

        format = D3DFMT_A8R8G8B8;
    }

    ::CloseHandle(fh);

    if (!pixels || format == D3DFMT_UNKNOWN) {
        return D3DXERR_INVALIDDATA;
    }

    if (!isDDS) {
        const UINT biggestDimension = static_cast<UINT>((std::max)(width, height));
        numMips = ((flags & GCTFF_BUILD_MIPS) == GCTFF_BUILD_MIPS) ? (Log2I(biggestDimension) + 1) : 1u;
    } else if (numMips == 1u && (flags & GCTFF_BUILD_MIPS) == GCTFF_BUILD_MIPS) {
        numMips = 0u; // setting it to zero will force Direct3D runtime to generate all mips
    }

    HRESULT hr = device->CreateTexture(static_cast<UINT>(width),
                                       static_cast<UINT>(height),
                                       numMips, 0u,
                                       format,
                                       D3DPOOL_MANAGED,
                                       dstTexture);
    if (SUCCEEDED(hr)) {
        D3DLOCKED_RECT lockedRect{};
        hr = dstTexture[0]->LockRect(0u, &lockedRect, nullptr, 0);

        if (SUCCEEDED(hr)) {
            if (!isDDS) {
                std::memcpy(lockedRect.pBits, pixels, static_cast<size_t>(width * height * 4));

                if (numMips > 1u) {
                    uint8_t* dstPointer = reinterpret_cast<uint8_t*>(lockedRect.pBits);
                    std::vector<uint8_t*> mipPointers(numMips);
                    mipPointers[0] = dstPointer;
                    dstPointer += width * height * 4;

                    for (int i = 1; i < static_cast<int>(numMips); ++i) {
                        const int mipW = (std::max)(width >> i, 1);
                        const int mipH = (std::max)(height >> i, 1);

                        mipPointers[i] = dstPointer;

                        // for each subsequent mip we go as far as 3 steps up for a source for a compromise between quality and the speed
                        const int srcMip = (std::max)(0, i - 3);
                        const int srcMipW = (std::max)(width >> srcMip, 1);
                        const int srcMipH = (std::max)(height >> srcMip, 1);

                        BuildMip(mipPointers[srcMip], srcMipW, srcMipH, mipPointers[i], mipW, mipH);

                        dstPointer += mipW * mipH * 4;
                    }
                }
            } else {
                std::memcpy(lockedRect.pBits, pixels, fullDDSDataSize);
            }

            hr = dstTexture[0]->UnlockRect(0u);
        }
    }

    if (isDDS) {
        free(pixels);
    } else {
        stbi_image_free(pixels);
    }

    return hr;
}
