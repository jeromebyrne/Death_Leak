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
	string GetDestroyedSound() const { return m_destroySound; }
	string GetMaterialName() const { return mMaterialName; }

private:

	vector<string> m_particleTextures;
	vector<string> m_damageSoundEffects;
	vector<string> m_footstepSoundEffects;
	string m_destroySound;
	string mMaterialName;
};

#endif
