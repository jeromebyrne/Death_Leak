#ifndef FEATUREUNLOCKMANAGER_H
#define FEATUREUNLOCKMANAGER_H

class FeatureUnlockManager
{
public:

	// Do NOT change the order of these enums, will affect the save file
	enum FeatureType
	{
		kNone = 0,
		kDownwardDash,
		kRoll,
		kSlowMotion,
		kDeflection,
		kSpeedIncrease,
		kProjectileDamageIncrease,
		kProjectileSpeedIncrease,
		kBombUnlock
	};

	FeatureUnlockManager(void);
	~FeatureUnlockManager(void);

	static FeatureUnlockManager * GetInstance();

	void Initialise();

	bool IsFeatureUnlocked(const FeatureType type);

	void SetFeatureUnlocked(const FeatureType type);

	FeatureType GetFeatureTypeFromString(const string & asString);

	string GetFeatureAsString(FeatureType featureType);

	int GetFeatureCost(const FeatureType featureType);

private:

	static FeatureUnlockManager * m_instance;
};

#endif
