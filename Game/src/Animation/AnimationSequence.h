#ifndef ANIMATIONSEQUENCE_H
#define ANIMATIONSEQUENCE_H

#include "tinyxml.h"

#include <list>
#include <map>
#include <string>
#include <utility>

class AnimationSkeleton;

struct AnimationFrameResource
{
	ID3D10ShaderResourceView* Texture = nullptr;
	std::string TextureFilename;

	AnimationFrameResource() = default;
	AnimationFrameResource(ID3D10ShaderResourceView* texture, std::string filename) :
		Texture(texture),
		TextureFilename(std::move(filename))
	{
	}

	operator ID3D10ShaderResourceView*() const
	{
		return Texture;
	}

	operator bool() const
	{
		return Texture != nullptr;
	}
};

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

	inline std::list<AnimationFrameResource> * Frames()
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
	inline const std::string & Name()
	{
		return m_name;
	}

	bool HasSFXforFrame(unsigned int frame) const;

	// we definitely know that there is an SFX
	std::string GetSFXforFrame(unsigned int frame) const;

	AnimationSkeleton * GetSkeleton() const { return mSkeleton; }

private:

	std::list<AnimationFrameResource> * m_frames; // holds texture resources or asset names
	std::map<unsigned, std::string> mSFXmap;
	float m_framerate; // the desired framerate at which the frames will be displayed in order
	float m_minFramerate; // the minimum frame speed
	float m_maxFramerate; // the maximum framerate
	std::string m_name; // the name of this animation sequence
	void ScaleBones();
	AnimationSkeleton * mSkeleton;
};

#endif
