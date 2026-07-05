#include "precompiled.h"
#include "XmlDocument.h"
#include "encryption.h"

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
#include <CoreFoundation/CoreFoundation.h>
#include "Backends/Mac/MacLaunchPaths.h"
#include <filesystem>
#include <vector>

namespace
{
std::filesystem::path NormalizeXmlPath(const std::string& fileName)
{
	std::string normalized = fileName;
	for (char& c : normalized)
	{
		if (c == '\\')
		{
			c = '/';
		}
	}

	return std::filesystem::path(normalized).relative_path();
}

std::filesystem::path ResolveCaseInsensitivePath(const std::filesystem::path& base, const std::filesystem::path& relative)
{
	std::filesystem::path current = base;
	if (!std::filesystem::exists(current))
	{
		return base / relative;
	}

	for (const auto& part : relative)
	{
		const std::string wanted = part.string();
		if (wanted.empty() || wanted == ".")
		{
			continue;
		}

		const std::filesystem::path exact = current / part;
		if (std::filesystem::exists(exact))
		{
			current = exact;
			continue;
		}

		std::string wantedLower = wanted;
		std::transform(wantedLower.begin(), wantedLower.end(), wantedLower.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
		});

		bool found = false;
		for (const auto& entry : std::filesystem::directory_iterator(current))
		{
			std::string candidate = entry.path().filename().string();
			std::transform(candidate.begin(), candidate.end(), candidate.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});

			if (candidate == wantedLower)
			{
				current = entry.path();
				found = true;
				break;
			}
		}

		if (!found)
		{
			return base / relative;
		}
	}

	return current;
}

std::filesystem::path BundleResourcesRoot()
{
	CFBundleRef bundle = CFBundleGetMainBundle();
	if (bundle == nullptr)
	{
		return {};
	}

	CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
	if (resourcesUrl == nullptr)
	{
		return {};
	}

	char buffer[PATH_MAX] = {0};
	const Boolean ok = CFURLGetFileSystemRepresentation(resourcesUrl, true, reinterpret_cast<UInt8*>(buffer), PATH_MAX);
	CFRelease(resourcesUrl);
	if (!ok)
	{
		return {};
	}

	return std::filesystem::path(buffer);
}

std::filesystem::path ResolveXmlPath(const std::string& fileName)
{
	const std::filesystem::path rawPath(fileName);
	if (rawPath.is_absolute())
	{
		return rawPath.lexically_normal();
	}

	const std::filesystem::path normalized = NormalizeXmlPath(fileName);
	if (normalized.empty())
	{
		return {};
	}

	const std::filesystem::path cwd = std::filesystem::current_path();
	const std::filesystem::path repoRoot = MacLaunchPaths::DetectRepoRoot();
	const std::vector<std::filesystem::path> roots = {
		BundleResourcesRoot(),
		repoRoot,
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

	return ResolveCaseInsensitivePath(cwd, normalized).lexically_normal();
}
}
#endif

std::string kKey = "babson";

XmlDocument::XmlDocument(void)
{
}

XmlDocument::~XmlDocument(void)
{
}

bool XmlDocument::Load(const std::string & filename, bool doBabbage)
{
	bool loadOkay = false;
#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
	const std::filesystem::path resolvedPath = ResolveXmlPath(filename);
	const std::string pathToLoad = resolvedPath.empty() ? filename : resolvedPath.string();
#else
	const std::string pathToLoad = filename;
#endif
	if (doBabbage)
	{
		TiXmlDocument d;
		bool loaded = d.LoadFile(pathToLoad.c_str());
		if (!loaded)
		{
			return false;
		}

		TiXmlElement* rootContent = d.FirstChildElement();

		std::string content = rootContent->Attribute("z");

		std::string decrypted = decrypt(content, kKey);

		m_document.Parse(decrypted.c_str(), 0, TIXML_ENCODING_UTF8);

		m_pHandle = new TiXmlHandle(&m_document);

		return true;
	}
	else
	{
		loadOkay = m_document.LoadFile(pathToLoad.c_str());

		if (loadOkay)
		{
			// create the handle
			m_pHandle = new TiXmlHandle(&m_document);
		}
	}

	return loadOkay;
}

void XmlDocument::Save(const std::string & filename, TiXmlElement * root, bool doBabbage)
{
	TiXmlDocument doc;

	if (doBabbage)
	{
		// Declare a printer    
		TiXmlPrinter printer;

		// attach it to the document you want to convert in to a std::string 
		root->Accept(&printer);

		std::string xmlAsStr = printer.CStr();

		std::string encrypted = encrypt(xmlAsStr, kKey);

		TiXmlElement * newElement = new TiXmlElement("a");
		newElement->SetAttribute("z", encrypted.c_str());

#if _DEBUG
		newElement->SetAttribute("content", xmlAsStr.c_str());
#endif

		doc.LinkEndChild(newElement);


	}
	else
	{
		doc.LinkEndChild(root);
	}

	doc.SaveFile( filename.c_str() );
}
