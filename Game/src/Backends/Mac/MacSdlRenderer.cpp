#include "Backends/Mac/MacSdlRenderer.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
Uint8 ToByte(float value)
{
    return static_cast<Uint8>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

SDL_RendererFlip ToSdlFlip(SpriteFlip flip)
{
    switch (flip)
    {
        case SpriteFlip::Horizontal:
            return SDL_FLIP_HORIZONTAL;
        case SpriteFlip::Vertical:
            return SDL_FLIP_VERTICAL;
        case SpriteFlip::Both:
            return static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
        case SpriteFlip::None:
        default:
            return SDL_FLIP_NONE;
    }
}

SDL_Surface* LoadPngSurface(const std::string& assetPath)
{
    CFStringRef path = CFStringCreateWithCString(nullptr, assetPath.c_str(), kCFStringEncodingUTF8);
    if (path == nullptr)
    {
        return nullptr;
    }

    CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, path, kCFURLPOSIXPathStyle, false);
    CFRelease(path);
    if (url == nullptr)
    {
        return nullptr;
    }

    CGImageSourceRef source = CGImageSourceCreateWithURL(url, nullptr);
    CFRelease(url);
    if (source == nullptr)
    {
        return nullptr;
    }

    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);
    CFRelease(source);
    if (image == nullptr)
    {
        return nullptr;
    }

    const int width = static_cast<int>(CGImageGetWidth(image));
    const int height = static_cast<int>(CGImageGetHeight(image));
    const int bytesPerPixel = 4;
    const int pitch = width * bytesPerPixel;
    std::vector<unsigned char> pixels(static_cast<std::size_t>(pitch * height));

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(
        pixels.data(),
        width,
        height,
        8,
        pitch,
        colorSpace,
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);

    if (context == nullptr)
    {
        CGImageRelease(image);
        return nullptr;
    }

    CGContextClearRect(context, CGRectMake(0, 0, width, height));
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    CGContextRelease(context);
    CGImageRelease(image);

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        pixels.data(),
        width,
        height,
        32,
        pitch,
        SDL_PIXELFORMAT_RGBA32);

    if (surface == nullptr)
    {
        return nullptr;
    }

    SDL_Surface* ownedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    return ownedSurface;
}
}

MacSdlRenderer::MacSdlRenderer(SDL_Window* window):
    mWindow(window)
{
}

MacSdlRenderer::~MacSdlRenderer()
{
    if (mRenderer != nullptr)
    {
        for (auto& kvp : mTextures)
        {
            if (kvp.second.Texture != nullptr)
            {
                SDL_DestroyTexture(kvp.second.Texture);
            }
        }
        mTextures.clear();
        mTextureCache.clear();
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
}

bool MacSdlRenderer::Initialise(int backBufferWidth, int backBufferHeight)
{
    if (mWindow == nullptr)
    {
        return false;
    }

    mBackBufferWidth = backBufferWidth;
    mBackBufferHeight = backBufferHeight;

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mRenderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_RenderSetLogicalSize(mRenderer, mBackBufferWidth, mBackBufferHeight);
    return true;
}

void MacSdlRenderer::BeginFrame(const Color& clearColor)
{
    if (mRenderer == nullptr)
    {
        return;
    }

    SDL_SetRenderDrawColor(
        mRenderer,
        ToByte(clearColor.R),
        ToByte(clearColor.G),
        ToByte(clearColor.B),
        ToByte(clearColor.A));
    SDL_RenderClear(mRenderer);
}

void MacSdlRenderer::EndFrame()
{
    if (mRenderer != nullptr)
    {
        SDL_RenderPresent(mRenderer);
    }
}

TextureHandle MacSdlRenderer::LoadTexture(const std::string& assetPath)
{
    const auto cached = mTextureCache.find(assetPath);
    if (cached != mTextureCache.end())
    {
        return cached->second;
    }

    SDL_Surface* surface = LoadPngSurface(assetPath);
    if (surface == nullptr)
    {
        std::cerr << "Texture load failed: " << assetPath << " error=" << SDL_GetError() << "\n";
        return 0;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    const int width = surface->w;
    const int height = surface->h;
    SDL_FreeSurface(surface);

    if (texture == nullptr)
    {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
        return 0;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    const TextureHandle handle = mNextTextureHandle++;
    mTextures[handle] = TextureResource{texture, width, height};
    mTextureCache[assetPath] = handle;
    return handle;
}

void MacSdlRenderer::DrawSprite(const SpriteDrawCommand& command)
{
    if (mRenderer == nullptr || command.Texture == 0)
    {
        return;
    }

    auto textureIter = mTextures.find(command.Texture);
    if (textureIter == mTextures.end() || textureIter->second.Texture == nullptr)
    {
        return;
    }

    SDL_Texture* texture = textureIter->second.Texture;
    SDL_SetTextureAlphaMod(texture, ToByte(command.Alpha));

    SDL_FRect destination;
    destination.x = command.X - command.Width * 0.5f;
    destination.y = command.Y - command.Height * 0.5f;
    destination.w = command.Width;
    destination.h = command.Height;

    const double degrees = static_cast<double>(command.RotationRadians) * 180.0 / 3.14159265358979323846;
    SDL_RenderCopyExF(mRenderer, texture, nullptr, &destination, degrees, nullptr, ToSdlFlip(command.Flip));
}

void MacSdlRenderer::DrawRect(const RectDrawCommand& command)
{
    if (mRenderer == nullptr)
    {
        return;
    }

    SDL_FRect destination;
    destination.x = command.X - command.Width * 0.5f;
    destination.y = command.Y - command.Height * 0.5f;
    destination.w = command.Width;
    destination.h = command.Height;

    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(mRenderer, ToByte(command.Fill.R), ToByte(command.Fill.G), ToByte(command.Fill.B), ToByte(command.Fill.A));
    SDL_RenderFillRectF(mRenderer, &destination);
    SDL_SetRenderDrawColor(mRenderer, ToByte(command.Outline.R), ToByte(command.Outline.G), ToByte(command.Outline.B), ToByte(command.Outline.A));
    SDL_RenderDrawRectF(mRenderer, &destination);
}

void MacSdlRenderer::DrawDebugText(const char* text, float top, float left)
{
    if (text == nullptr)
    {
        return;
    }

    std::cout << "debug text [" << left << "," << top << "]: " << text << "\n";
}
