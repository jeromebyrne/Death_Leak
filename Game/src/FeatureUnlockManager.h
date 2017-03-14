#ifndef FEATUREUNLOCKMANAGER_H
#define FEATUREUNLOCKMANAGER_H

class FeatureUnlockManager
{
public:

	// Do NOT change the order of these enums, will affect the save file
	enum FeatureType
	{
		kDoubleJump = 1,
		kCrouchJump,
		kDownwardDash
	};

	FeatureUnlockManager(void);
	~FeatureUnlockManager(void);

	static FeatureUnlockManager * GetInstance();

	void Initialise();

	bool IsFeatureUnlocked(const FeatureType type);

	void SetFeatureUnlocked(const FeatureType type, bool unlocked);

private:

	static FeatureUnlockManager * m_instance;

	std::map<FeatureType, bool> mFeatureUnlockCache;
};

#endif
