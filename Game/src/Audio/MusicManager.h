#ifndef MUSICMANAGER_H
#define MUSICMANAGER_H

class MusicManager
{
public:
	MusicManager(void);
	~MusicManager(void);

	void Update(float delta);

	void Initialise(const char * musicFile, float musicLength, float initialDelay, float timeBetween);

private:

	bool mHasPlayedMusicOnce;
	string mMusicFile;
	float mMusicLength;
	float mInitialDelay;
	float mTimeBetween;

	float mTimeUntilPlayAgain;
};

#endif