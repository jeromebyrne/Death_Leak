#ifndef AUDIOOBJECT_H
#define AUDIOOBJECT_H

#include "gameobject.h"

class AudioObject : public GameObject
{
public: 
	AudioObject(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
	virtual ~AudioObject(void);
	
	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	void SetAudioFilename(string filename) { mAudioFilename = filename; }
	void SetAdjustVolumeToCamera(bool value) { mAdjustVolumeToCamera = value; }
	void SetRepeat(bool value) { mRepeat = value; }
	void SetVolumeFadeDimensions(Vector2 value) { mFadeDimensions = value; }
	void SetVolume(float value);

private:

	virtual void DebugDraw(ID3D10Device *  device) override;

	string mAudioFilename;
	bool mAdjustVolumeToCamera;
	bool mRepeat;

	Vector2 mFadeDimensions;
	ISound * mSoundInstance;
	bool mHasStartedPlaying;
};

#endif

