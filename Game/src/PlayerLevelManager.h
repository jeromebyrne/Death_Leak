#ifndef PLAYERLEVELMANAGER_H
#define PLAYERLEVELMANAGER_H

class PlayerLevelManager
{
public:

	PlayerLevelManager(void);
	~PlayerLevelManager(void);

	static PlayerLevelManager * GetInstance();

	void Initialise();
	
	float GetPercentTowardsLevelUp(unsigned int currentLevel, unsigned int currentXPUnits) const;

	bool ShouldLevelUp(unsigned int currentLevel, unsigned int currentXPUnits) const;

private:

	struct PlayerLevelInfo
	{
		bool CanLevelUp;
		unsigned UnitsRequiredToLevelUp;

		/* Add mechanic unlocked */
	};

	void CreateLevelUpData();

	std::map<unsigned int, PlayerLevelInfo> mPlayerLevelData;

	static PlayerLevelManager * m_instance;

	unsigned int mPlayerCurrentLevel;
};

#endif
