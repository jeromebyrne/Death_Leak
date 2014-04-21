#include "precompiled.h"
#include "Material.h"

Material::Material(const char * materialName):
	mMaterialName(materialName)
{
}

Material::~Material(void)
{
}

void Material::ReadXml(TiXmlElement * element)
{
	// we assume an element which contains 3 children
	//1st child contains particle textures
	//2nd child contains damage sounds
	//3rd child contains footstep sounds

	// 1st child - particle textures
	TiXmlElement * particleChild = element->FirstChildElement();
	TiXmlElement * currentTexture = particleChild->FirstChildElement();

	while(currentTexture)
	{
		char* texFile = XmlUtilities::ReadAttributeAsString(currentTexture, "", "file");

		m_particleTextures.push_back(string(texFile));

		currentTexture = currentTexture->NextSiblingElement();
	}

	// 2nd child element - damage sound files
	TiXmlElement * damageSoundChild = particleChild->NextSiblingElement();
	TiXmlElement * currentDamageSound = damageSoundChild->FirstChildElement();

	while(currentDamageSound)
	{
		char* soundFile = XmlUtilities::ReadAttributeAsString(currentDamageSound, "", "file");
		
		m_damageSoundEffects.push_back(string(soundFile));

		currentDamageSound = currentDamageSound->NextSiblingElement();
	}

	// 3rd child element - footsteps sound files
	TiXmlElement * footstepSoundChild = damageSoundChild->NextSiblingElement();
	TiXmlElement * currentFootstepSound = footstepSoundChild->FirstChildElement();

	while(currentFootstepSound)
	{
		char* soundFile = XmlUtilities::ReadAttributeAsString(currentFootstepSound, "", "file");
		
		m_footstepSoundEffects.push_back(string(soundFile));

		currentFootstepSound = currentFootstepSound->NextSiblingElement();
	}

	m_destroySound = XmlUtilities::ReadAttributeAsString(element, "destroysoundeffect", "file");

	// done
}

string Material::GetRandomDamageSoundFilename()
{
	int soundCount = m_damageSoundEffects.size();
	if (soundCount == 0)
	{
		LOG_ERROR("Damage sound count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(soundCount > 0);
		return "";
	}
	int randNum = rand() % soundCount;

	return m_damageSoundEffects[randNum];
}

string Material::GetRandomFootstepSoundFilename()
{
	int soundCount = m_footstepSoundEffects.size();
	if (soundCount == 0)
	{
		LOG_ERROR("Footstep sound count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(soundCount > 0);
		return "";
	}
	int randNum = rand() % soundCount;

	return m_footstepSoundEffects[randNum];
}

string Material::GetRandomParticleTexture()
{
	int texCount = m_particleTextures.size();
	if (texCount == 0)
	{
		LOG_ERROR("Particle texture count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(texCount > 0);
		return "";
	}
	int randNum = rand() % texCount;

	return m_particleTextures[randNum];
}
