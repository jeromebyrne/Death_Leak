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

	void Stop();

	void Disable();

	bool IsInitialised() const;

	string GetAudioFilename() const { return mAudioFilename; }
	bool IsRepeat() const { return mRepeat; }
	Vector2 GetDimensions() const { return mDimensions; }
	Vector2 GetFadeDimensions() const { return mFadeDimensions; }

	void SetAudioFilename(const string & filename) { mAudioFilename = filename; }
	void SetRepeat(bool repeat) { mRepeat = repeat; }
	void SetDimensions(const Vector2 & dimensions) { mDimensions = dimensions; }
	void SetFadeDimensions(const Vector2 & fadeDimensions) { mFadeDimensions = fadeDimensions; }

private:

	void DeleteSoundInstance();

	string mAudioFilename;
	bool mRepeat;
	Vector2 mDimensions;
	Vector2 mFadeDimensions;
	bool mIsInitialised = false;
	irrklang::ISound * mSoundInstance = nullptr;
	bool mPlaying = false;
	bool mIsDisabled = false;
};

#endif
