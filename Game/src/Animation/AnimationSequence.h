#ifndef ANIMATIONSEQUENCE_H
#define ANIMATIONSEQUENCE_H

class AnimationSkeleton;

class AnimationSequence
{
public:

	AnimationSequence(TiXmlElement * element);
	~AnimationSequence(void);

	void ReadXml(TiXmlElement * element);

	void SetFrameRate(float value, bool overrideMinFramerate = false)
	{
		m_framerate = value;
		if (m_framerate > m_maxFramerate)
		{
			m_framerate = m_maxFramerate;
		}
		else if (m_framerate < m_minFramerate && !overrideMinFramerate)
		{
			m_framerate = m_minFramerate;
		}
	}

	inline list<ID3D10ShaderResourceView*> * Frames()
	{
		return m_frames;
	}
	inline float Framerate()
	{
		return m_framerate;
	}
	inline float MinFramerate()
	{
		return m_minFramerate;
	}
	inline float MaxFramerate()
	{
		return m_maxFramerate;
	}
	inline string Name()
	{
		return m_name;
	}

	AnimationSkeleton * GetSkeleton() const { return mSkeleton; }

private:

	list<ID3D10ShaderResourceView*> * m_frames; // holds a list of textures
	float m_framerate; // the desired framerate at which the frames will be displayed in order
	float m_minFramerate; // the minimum frame speed
	float m_maxFramerate; // the maximum framerate
	string m_name; // the name of this animation sequence
	void ScaleBones();
	AnimationSkeleton * mSkeleton;
};

#endif
