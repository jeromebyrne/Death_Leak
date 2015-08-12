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

	bool GetIsPierceable() const { return mPierceable; }

private:

	vector<string> m_particleTextures;
	vector<string> m_damageSoundEffects;
	vector<string> m_footstepSoundEffects;
	vector<string> m_destroySoundEffects;
	string mMaterialName;

	bool mPierceable;
};

#endif
