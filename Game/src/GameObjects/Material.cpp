#include "precompiled.h"
#include "Material.h"

Material::Material(const char * materialName):
	mMaterialName(materialName),
	mPierceable(false)
{
	if (strcmp(materialName,"water")==0)
	{
		mIsWater = true;
	}
	else if (strcmp(materialName, "softwood") == 0 ||
		strcmp(materialName, "caverock") == 0 ||
		strcmp(materialName, "stone") == 0)
	{
		mShouldVibrate = true;
	}
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

		mParticleTextures.push_back(string(texFile));

		currentTexture = currentTexture->NextSiblingElement();
	}

	// 2nd child element - damage sound files
	TiXmlElement * damageSoundChild = particleChild->NextSiblingElement();
	TiXmlElement * currentDamageSound = damageSoundChild->FirstChildElement();

	while(currentDamageSound)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentDamageSound, "", "file");
		
		mDamageSoundEffects.push_back(string(soundFile));

		currentDamageSound = currentDamageSound->NextSiblingElement();
	}

	// 3rd child element - footsteps sound files
	TiXmlElement * footstepSoundChild = damageSoundChild->NextSiblingElement();
	TiXmlElement * currentFootstepSound = footstepSoundChild->FirstChildElement();

	while(currentFootstepSound)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentFootstepSound, "", "file");
		
		mFootstepSoundEffects.push_back(string(soundFile));

		currentFootstepSound = currentFootstepSound->NextSiblingElement();
	}

	TiXmlElement * destroyedSoundChild = footstepSoundChild->NextSiblingElement();
	TiXmlElement * currentDestroyedSound = destroyedSoundChild->FirstChildElement();

	while (currentDestroyedSound)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentDestroyedSound, "", "file");

		mDestroySoundEffects.push_back(string(soundFile));

		currentDestroyedSound = currentDestroyedSound->NextSiblingElement();
	}

	mPierceable = XmlUtilities::ReadAttributeAsBool(element, "is_pierceable", "value");

	// really hacky crappy code but trying to get stuff FINISHED!
	TiXmlElement * debrisTexturesChild = destroyedSoundChild->NextSiblingElement();
	debrisTexturesChild = debrisTexturesChild->NextSiblingElement();

	TiXmlElement * currentDebrisTexture = debrisTexturesChild->FirstChildElement();
	while (currentDebrisTexture)
	{
		const char * soundFile = XmlUtilities::ReadAttributeAsString(currentDebrisTexture, "", "file");

		mDebrisTextures.push_back(string(soundFile));

		currentDebrisTexture = currentDebrisTexture->NextSiblingElement();
	}
}

string Material::GetRandomDamageSoundFilename()
{
	int soundCount = mDamageSoundEffects.size();
	if (soundCount == 0)
	{
		LOG_ERROR("Damage sound count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(soundCount > 0);
		return "";
	}
	int randNum = rand() % soundCount;

	return mDamageSoundEffects[randNum];
}

string Material::GetRandomDestroyedSound()
{
	int soundCount = mDestroySoundEffects.size();
	if (soundCount == 0)
	{
		LOG_ERROR("Damage sound count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(soundCount > 0);
		return "";
	}
	int randNum = rand() % soundCount;

	return mDestroySoundEffects[randNum];
}

string Material::GetRandomFootstepSoundFilename()
{
	int soundCount = mFootstepSoundEffects.size();
	if (soundCount == 0)
	{
		LOG_ERROR("Footstep sound count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(soundCount > 0);
		return "";
	}
	int randNum = rand() % soundCount;

	return mFootstepSoundEffects[randNum];
}

string Material::GetRandomParticleTexture()
{
	int texCount = mParticleTextures.size();
	if (texCount == 0)
	{
		LOG_ERROR("Particle texture count is 0 for material: %s", mMaterialName.c_str());
		GAME_ASSERT(texCount > 0);
		return "";
	}
	int randNum = rand() % texCount;

	return mParticleTextures[randNum];
}
