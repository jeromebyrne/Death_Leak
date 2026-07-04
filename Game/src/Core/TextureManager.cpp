#include "precompiled.h"
#include "TextureManager.h"

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>

TextureManager * TextureManager:: m_instance = 0;

namespace
{
bool ReadImageDimensions(const char* fileName, int& width, int& height)
{
	width = 1;
	height = 1;

	if (fileName == nullptr || *fileName == '\0')
	{
		return false;
	}

	CFStringRef path = CFStringCreateWithCString(nullptr, fileName, kCFStringEncodingUTF8);
	if (path == nullptr)
	{
		return false;
	}

	CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, path, kCFURLPOSIXPathStyle, false);
	CFRelease(path);
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
    if (m_textureIDmap[fileName] != nullptr)
    {
        return m_textureIDmap[fileName];
    }

	int width = 1;
	int height = 1;
	ReadImageDimensions(fileName, width, height);

	auto * texture = new MacShaderResourceView(width, height);
    m_textureIDmap[fileName] = texture;
    return texture;
}

ID3D10ShaderResourceView* TextureManager::LoadTexture_ui(const char * fileName)
{
    if (m_textureIDmap_ui[fileName] != nullptr)
    {
        return m_textureIDmap_ui[fileName];
    }

	int width = 1;
	int height = 1;
	ReadImageDimensions(fileName, width, height);

	auto * texture = new MacShaderResourceView(width, height);
    m_textureIDmap_ui[fileName] = texture;
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
