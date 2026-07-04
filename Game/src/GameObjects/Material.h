#ifndef MATERIAL_H
#define MATERIAL_H

#include "tinyxml.h"

#include <string>
#include <vector>

class Material
{
public:

	Material(const char * materialName);
	~Material(void);

	void ReadXml(TiXmlElement * element);

	std::string GetRandomDamageSoundFilename();
	std::string GetRandomFootstepSoundFilename();
	std::string GetRandomParticleTexture();
	std::string GetRandomDestroyedSound();
	std::string GetMaterialName() const { return mMaterialName; }

	const std::vector<std::string> GetDebrisTextures() const { return mDebrisTextures; }

	bool GetIsPierceable() const { return mPierceable; }

	bool IsWater() const { return mIsWater; }

	bool ShouldVibrate() const { return mShouldVibrate; }

private:

	std::vector<std::string> mParticleTextures;
	std::vector<std::string> mDamageSoundEffects;
	std::vector<std::string> mFootstepSoundEffects;
	std::vector<std::string> mDestroySoundEffects;
	std::vector<std::string> mDebrisTextures;
	std::string mMaterialName;

	bool mPierceable;

	bool mIsWater = false;

	bool mShouldVibrate = false;
};

#endif
