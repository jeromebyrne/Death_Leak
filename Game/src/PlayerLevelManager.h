#ifndef PLAYERLEVELMANAGER_H
#define PLAYERLEVELMANAGER_H

class PlayerLevelManager
{
public:

	PlayerLevelManager(void);
	~PlayerLevelManager(void);

	static PlayerLevelManager * GetInstance();

private:

	static PlayerLevelManager * m_instance;
};

#endif
