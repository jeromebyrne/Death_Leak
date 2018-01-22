#ifndef ANIMATIONPART_H
#define ANIMATIONPART_H
#include "AnimationSequence.h"

class AnimationPart
{
public:
	
	AnimationPart(TiXmlElement * element);
	~AnimationPart(void);

	void ReadXml(TiXmlElement * element);

	// returns a value indicating if the current frame has finished displaying
	bool IsFinished();

	// returns the currently displayed frame (texture) in the current sequence being played
	ID3D10ShaderResourceView* CurrentFrame();

	// returns the number of frames in the current sequence
	int FrameCount();

	// returns the current frame number in the current sequence
	int FrameNumber();

	bool HasSFXforCurrentFrame() const;

	std::vector<std::string> GetSequenceNames() const;

	inline float OffsetX()
	{
		return m_offsetX;
	}
	inline float OffsetY()
	{
		return m_offsetY;
	}
	inline float SizeX()
	{
		return m_sizeX;
	}
	inline float SizeY()
	{
		return m_sizeY;
	}
	inline AnimationSequence* CurrentSequence()
	{
		return m_currentSequence;
	}

	AnimationSequence * GetSequence(const string & sequenceName);

	inline string Name()
	{
		return m_name;
	}

	// restart the sequence
	void Restart()
	{
		m_currentFrameNumber = 0; m_frameTimeElapsed = 0;
	}

	// finish the animation abruptly
	void Finish() { m_currentFrameNumber = m_currentSequence->Frames()->size(); }

	// set the current sequence
	void SetSequence(const string & name);

	// animate, when we reach the last frame go back to the first
	void AnimateLooped();

	// animate, when we reach the last frame go back to the first
	void AnimateLooped(float frameRate);

	void Animate();

	void Animate(float frameRate);

#ifdef DEBUG // purely for debugging 
	void SetFrame(int frame)
	{
		m_currentFrameNumber = frame;
	}
#endif

	map<string, AnimationSequence*> & GetAllSequences()
	{
		return m_sequences;
	}

private:
	// a map containing all the animation sequences for this part
	map<string,AnimationSequence*> m_sequences;
	
	// the current sequence playing on this part
	AnimationSequence* m_currentSequence;

	// the current frame being displayed in the current sequence
	int m_currentFrameNumber;

	// the time since the last frame change.
	float m_frameTimeElapsed;

	// the x offset of this part from the centre of the parent object
	float m_offsetX;

	// the y offset of this part from the centre of the parent object
	float m_offsetY;

	// the x size of the part
	float m_sizeX;

	// the y size of the part
	float m_sizeY;

	// the name of this animation part
	string m_name;
};

#endif
