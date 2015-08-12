#include "precompiled.h"
#include "Material.h"

Material::Material(const char * materialName):
	mMaterialName(materialName),
	mPierceable(false)
{
}

Material::~Material(void)
{
}

void Material::ReadXml(TiXmlElement * element)
{
	// 1st child - particle textures
	TiXmlElement * particleChild = element->FirstChildElement();
	TiXmlElement * currentTexture = particleChild->FirstChildElement();

	while(currentTexture)
	{
		const char * texFile = XmlUtilities::ReadAttributeAsString(currentTexture, "", "file");

		m_particleTextures.push_back(string(texFile));

		currentTexture = currentTexture->NextSiblingElement();
	}

	// 2nd child element - damage sound files
	TiXmlElement * damageSoundChild = particleChild->NextSiblingElement();
	TiXmlElement * currentDamageSound = damageSoundChild->FirstChildElement();

	while(currentDamageSound)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentDamageSound, "", "file");
		
		m_damageSoundEffects.push_back(string(soundFile));

		currentDamageSound = currentDamageSound->NextSiblingElement();
	}

	// 3rd child element - footsteps sound files
	TiXmlElement * footstepSoundChild = damageSoundChild->NextSiblingElement();
	TiXmlElement * currentFootstepSound = footstepSoundChild->FirstChildElement();

	while(currentFootstepSound)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentFootstepSound, "", "file");
		
		m_footstepSoundEffects.push_back(string(soundFile));

		currentFootstepSound = currentFootstepSound->NextSiblingElement();
	}

	TiXmlElement * destroyedSoundChild = footstepSoundChild->NextSiblingElement();
	TiXmlElement * currentDestroyedSound = destroyedSoundChild->FirstChildElement();

	while (currentDestroyedSound)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentDestroyedSound, "", "file");

		m_destroySoundEffects.push_back(string(soundFile));

		currentDestroyedSound = currentDestroyedSound->NextSiblingElement();
	}

	mPierceable = XmlUtilities::ReadAttributeAsBool(element, "is_pierceable", "value");
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

string Material::GetRandomDestroyedSound()
{
	int soundCount = m_destroySoundEffects.size();
	if (soundCount == 0)
	{
		LOG_ERROR("Damage sound count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(soundCount > 0);
		return "";
	}
	int randNum = rand() % soundCount;

	return m_destroySoundEffects[randNum];
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
