#ifndef MATERIAL_H
#define MATERIAL_H

class Material
{
public:

	Material(const char * materialName);
	~Material(void);

	void ReadXml(TiXmlElement * element);

	string GetRandomDamageSoundFilename();
	string GetRandomFootstepSoundFilename();
	string GetRandomParticleTexture();
	string GetRandomDestroyedSound();
	string GetMaterialName() const { return mMaterialName; }

	const vector<string> GetDebrisTextures() const { return mDebrisTextures; }

	bool GetIsPierceable() const { return mPierceable; }

	bool IsWater() const { return mIsWater; }

	bool ShouldVibrate() const { return mShouldVibrate; }

private:

	vector<string> mParticleTextures;
	vector<string> mDamageSoundEffects;
	vector<string> mFootstepSoundEffects;
	vector<string> mDestroySoundEffects;
	vector<string> mDebrisTextures;
	string mMaterialName;

	bool mPierceable;

	bool mIsWater = false;

	bool mShouldVibrate = false;
};

#endif
