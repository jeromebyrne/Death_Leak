#include "precompiled.h"
#include "TextureManager.h"

#include <filesystem>

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>

TextureManager * TextureManager:: m_instance = 0;

namespace
{
std::filesystem::path NormalizeTexturePath(const char* fileName)
{
    std::string path = fileName != nullptr ? fileName : "";
    for (char& c : path)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }

    return std::filesystem::path(path).relative_path();
}

std::filesystem::path ResolveTexturePath(const char* fileName)
{
    if (fileName == nullptr || *fileName == '\0')
    {
        return {};
    }

    const std::filesystem::path rawPath(fileName);
    if (rawPath.is_absolute())
    {
        return rawPath.lexically_normal();
    }

    const std::filesystem::path normalized = NormalizeTexturePath(fileName);
    if (normalized.empty())
    {
        return {};
    }

    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::vector<std::filesystem::path> roots = {
        cwd,
        cwd.parent_path(),
        cwd.parent_path().parent_path(),
        cwd.parent_path().parent_path().parent_path(),
        cwd / "Game",
        cwd.parent_path() / "Game",
        cwd.parent_path().parent_path() / "Game",
    };

    for (const auto& root : roots)
    {
        if (root.empty())
        {
            continue;
        }

        const std::filesystem::path candidate = root / normalized;
        if (std::filesystem::exists(candidate))
        {
            return candidate.lexically_normal();
        }
    }

    return (cwd / normalized).lexically_normal();
}

bool ReadImageDimensions(const char* fileName, int& width, int& height)
{
	width = 1;
	height = 1;

	if (fileName == nullptr || *fileName == '\0')
	{
		return false;
	}

	const std::filesystem::path resolved = ResolveTexturePath(fileName);
	const std::string resolvedPath = resolved.string();

	CFStringRef cfPath = CFStringCreateWithCString(nullptr, resolvedPath.c_str(), kCFStringEncodingUTF8);
	if (cfPath == nullptr)
	{
		return false;
	}

	CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath, kCFURLPOSIXPathStyle, false);
	CFRelease(cfPath);
	if (url == nullptr)
	{
		return false;
	}

	CGImageSourceRef source = CGImageSourceCreateWithURL(url, nullptr);
	CFRelease(url);
	if (source == nullptr)
	{
		return false;
	}

	CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);
	CFRelease(source);
	if (image == nullptr)
	{
		return false;
	}

	width = static_cast<int>(CGImageGetWidth(image));
	height = static_cast<int>(CGImageGetHeight(image));
	CGImageRelease(image);

	if (width <= 0)
	{
		width = 1;
	}
	if (height <= 0)
	{
		height = 1;
	}

	return true;
}

class MacTexture2D final : public ID3D10Texture2D
{
public:
	MacTexture2D(int width, int height)
	{
		m_desc.Width = static_cast<UINT>(std::max(1, width));
		m_desc.Height = static_cast<UINT>(std::max(1, height));
	}
};

class MacShaderResourceView final : public ID3D10ShaderResourceView
{
public:
	MacShaderResourceView(int width, int height):
		mWidth(width),
		mHeight(height)
	{
	}

	HRESULT GetResource(ID3D10Resource** out) override
	{
		if (out != nullptr)
		{
			*out = new MacTexture2D(mWidth, mHeight);
		}
		return S_OK;
	}

private:
	int mWidth;
	int mHeight;
};
}

TextureManager * TextureManager::Instance()
{
	if(m_instance == 0)
	{
		m_instance = new TextureManager();
	}
	return m_instance;
}

TextureManager::TextureManager(void)
{
}

TextureManager::~TextureManager(void)
{
	for (auto & kvp : m_textureIDmap)
	{
		if (kvp.second != nullptr)
		{
			kvp.second->Release();
		}
	}
	for (auto & kvp : m_textureIDmap_ui)
	{
		if (kvp.second != nullptr)
		{
			kvp.second->Release();
		}
	}
}

void TextureManager::Initialise(ID3D10Device * graphicsDevice)
{
    m_graphicsDevice = graphicsDevice;
}

ID3D10ShaderResourceView* TextureManager::LoadTexture(const char * fileName)
{
    if (fileName == nullptr || *fileName == '\0')
    {
        LOG_ERROR("LoadTexture called with invalid file name");
        return nullptr;
    }

    const std::filesystem::path resolvedPath = ResolveTexturePath(fileName);
    const std::string cacheKey = resolvedPath.string();

    if (m_textureIDmap[cacheKey] != nullptr)
    {
        return m_textureIDmap[cacheKey];
    }

	int width = 1;
	int height = 1;
	ReadImageDimensions(cacheKey.c_str(), width, height);

	auto * texture = new MacShaderResourceView(width, height);
    m_textureIDmap[cacheKey] = texture;
    return texture;
}

ID3D10ShaderResourceView* TextureManager::LoadTexture_ui(const char * fileName)
{
    if (fileName == nullptr || *fileName == '\0')
    {
        LOG_ERROR("LoadTexture_ui called with invalid file name");
        return nullptr;
    }

    const std::filesystem::path resolvedPath = ResolveTexturePath(fileName);
    const std::string cacheKey = resolvedPath.string();

    if (m_textureIDmap_ui[cacheKey] != nullptr)
    {
        return m_textureIDmap_ui[cacheKey];
    }

	int width = 1;
	int height = 1;
	ReadImageDimensions(cacheKey.c_str(), width, height);

	auto * texture = new MacShaderResourceView(width, height);
    m_textureIDmap_ui[cacheKey] = texture;
    return texture;
}

void TextureManager::Release()
{
	delete m_instance;
	m_instance = nullptr;
}

#else

TextureManager * TextureManager:: m_instance = 0;

TextureManager * TextureManager::Instance()
{
	if(m_instance == 0)
	{
		m_instance = new TextureManager();
	}
	return m_instance;
}

TextureManager::TextureManager(void)
{

}

TextureManager::~TextureManager(void)
{
	// release all of our texture resources
	map<string,ID3D10ShaderResourceView*>::iterator current = m_textureIDmap.begin();

	for(; current != m_textureIDmap.end(); current++)
	{
		if(current->second)
		{
			current->second->Release();
		}
	}
	
	// clear the UI cache
	map<string,ID3D10ShaderResourceView*>::iterator current_ui = m_textureIDmap_ui.begin();

	for(; current_ui != m_textureIDmap_ui.end(); current_ui++)
	{
		if(current_ui->second)
		{
			current_ui->second->Release();
		}
	}
}

void TextureManager::Initialise(ID3D10Device * graphicsDevice)
{
	m_graphicsDevice = graphicsDevice;
}

ID3D10ShaderResourceView* TextureManager::LoadTexture(const char * fileName)
{
	// first convert our char* to wchar_t*
	wchar_t* fileNameWide = Utilities::ConvertCharStringToWcharString(fileName);

	// search the map and see have we already got this texture
	if(m_textureIDmap[fileName] != 0)
	{
		return m_textureIDmap[fileName]; // we already have this texture
	}
	
	// we dont already have it so let's load it
	HRESULT hr = S_OK;

	ID3D10ShaderResourceView* texture = nullptr;

	D3DX10CreateShaderResourceViewFromFile( m_graphicsDevice, fileNameWide, nullptr, nullptr, &texture, nullptr );

    if( FAILED( hr ) || !texture)
	{
		LOG_ERROR("Couldn't load texture: %s", fileName);
		GAME_ASSERT(false);
	}
	else
	{
		// add to the map
		m_textureIDmap[fileName] = texture;
	}

	return texture;
}

ID3D10ShaderResourceView* TextureManager::LoadTexture_ui(const char * fileName)
{
	// first convert our char* to wchar_t*
	wchar_t* fileNameWide = Utilities::ConvertCharStringToWcharString(fileName);

	// search the map and see have we already got this texture
	if(m_textureIDmap_ui[fileName] != nullptr)
	{
		return m_textureIDmap_ui[fileName]; // we already have this texture
	}
	
	// we dont already have it so let's load it
	HRESULT hr = S_OK;

	ID3D10ShaderResourceView* texture = nullptr;

	D3DX10CreateShaderResourceViewFromFile( m_graphicsDevice, fileNameWide, nullptr, nullptr, &texture, nullptr );

    if( FAILED( hr ) || !texture)
	{
		LOG_ERROR("Couldn't load texture: %s", fileName);
		GAME_ASSERT(false);
	}
	else
	{
		// add to the map
		m_textureIDmap_ui[fileName] = texture;
	}

	return texture;
}

void TextureManager::Release()
{
	delete m_instance;
	m_instance = nullptr;
}

#endif
