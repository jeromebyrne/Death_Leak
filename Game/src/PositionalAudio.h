#ifndef POSITIONALAUDIO_H
#define POSITIONALAUDIO_H

namespace irrklang
{
	class ISound;
};

class PositionalAudio
{
public:

	PositionalAudio();

	~PositionalAudio();

	void Initialise(const string & audioFilename, 
					bool repeat, 
					const Vector2 & dimensions, 
					const Vector2 & fadeDimensions);

	void Update(float delta, const Vector2 & position);

	void Play();

	bool IsInitialised() const;

private:

	void DeleteSoundInstance();

	string mAudioFilename;
	bool mRepeat;
	Vector2 mDimensions;
	Vector2 mFadeDimensions;
	bool mIsInitialised = false;
	irrklang::ISound * mSoundInstance = nullptr;
};

#endif
