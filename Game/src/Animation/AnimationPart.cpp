#include "precompiled.h"
#include "AnimationPart.h"

AnimationPart::AnimationPart(TiXmlElement * element):m_sizeX(64), m_sizeY(64), m_offsetX(0), m_offsetY(0), m_currentFrameNumber(0), m_frameTimeElapsed(0)
{
	if(element == 0)
	{
		throw new exception("cannot load animation part, null element");
	}
	
	ReadXml(element); 

    // Get the first animation sequence and set it as the current sequence
	map<string,AnimationSequence*>::iterator firstSequence = m_sequences.begin();

	m_currentSequence = (*firstSequence).second;

}

AnimationPart::~AnimationPart(void)
{
}

void AnimationPart::ReadXml(TiXmlElement * element)
{
	//get the name 
	m_name = (string)element->Value();

	// read the size
	m_sizeX = XmlUtilities::ReadAttributeAsFloat(element, "", "SizeX");
	m_sizeY = XmlUtilities::ReadAttributeAsFloat(element, "", "SizeY");

	// read the offset value
	m_offsetX = XmlUtilities::ReadAttributeAsFloat(element, "", "OffsetX");
	m_offsetY = XmlUtilities::ReadAttributeAsFloat(element, "", "OffsetY");

	TiXmlElement * sequence = element->FirstChildElement();
	while(sequence != 0)
	{
		string sequenceName = (string)sequence->Value();
		m_sequences[sequenceName] = new AnimationSequence(sequence);

		sequence = sequence->NextSiblingElement();
	}
}

bool AnimationPart::IsFinished()
{
	if(m_currentSequence != 0)
	{
		// if we are past the last frame then we are finished
		if(m_currentFrameNumber >= m_currentSequence->Frames()->size())
		{
			return true;
		}
	}
	else
	{
		// the sequence is null

		return true;
	}

	return false;
}

ID3D10ShaderResourceView* AnimationPart::CurrentFrame()
{
	if(m_currentSequence == 0)
	{
		return 0;
	}

	if(m_currentSequence->Frames()->size() <= 0)
	{
		return 0;
	}

	// if we are past the end then just return the last frame
	if(m_currentFrameNumber >= m_currentSequence->Frames()->size())
	{
		list<ID3D10ShaderResourceView*>::iterator end = m_currentSequence->Frames()->end();
		end--;

		return (*end);
	}

	// return the current frame normally
	list<ID3D10ShaderResourceView*>::iterator current = m_currentSequence->Frames()->begin();
	list<ID3D10ShaderResourceView*>::iterator end = m_currentSequence->Frames()->end();

	for(int frameNum = 0; current!=end; current++, frameNum++)
	{
		if(frameNum == m_currentFrameNumber)
		{
			return (*current);
		}
	}

	return 0;
}

int AnimationPart::FrameCount()
{
	// if the sequence is null then return 0
	if(m_currentSequence != 0)
	{
		return m_currentSequence->Frames()->size();
	}
	else
	{
		return 0;
	}
}

int AnimationPart::FrameNumber()
{
	// if no sequence then return 0
	if(m_currentSequence == 0)
	{
		return 0;
	}
	
	int numFrames = m_currentSequence->Frames()->size();

	// if there are no frames then return 0
	if(numFrames <= 0)
	{
		return 0;
	}

	// if we are past the last frame then return the last one
	if(m_currentFrameNumber >= numFrames)
	{
		return numFrames-1;
	}

	// in a normal case just return the current frame number
	return m_currentFrameNumber;
}

void AnimationPart::SetSequence(const string & name)
{	
	// start from the first frame
	Restart();
	
	// convert to lower case 
	//Utilities::ToLower(name);

	// only switch if the part exists

	AnimationSequence* newSequence = m_sequences[name];

	if(newSequence != nullptr)
	{
		m_currentSequence = m_sequences[name];
	}
}

void AnimationPart::AnimateLooped()
{
	// Animate at the frame rate for the current sequence:
	if(m_currentSequence != nullptr)
	{
		AnimateLooped(m_currentSequence->Framerate());
	}
}

void AnimationPart::AnimateLooped(float frameRate)
{
	if(m_currentSequence != nullptr  && frameRate > 0)
	{
		int numFrames = m_currentSequence->Frames()->size();
		if(numFrames > 0)
		{
			// increment the frame time elapsed
			m_frameTimeElapsed += Timing::Instance()->GetLastUpdateDelta();

			// get the amount of time each frame takes
			float timePerFrame = 1 / frameRate;

			// check if we have exceeded the time
			if(m_frameTimeElapsed >= timePerFrame)
			{
				// we're finished with this frame so decrease the frame time elapsed
				m_frameTimeElapsed -= timePerFrame;

				// increment to the next frame
				m_currentFrameNumber++;

				// if we're past the end then go back to the first frame
				if(m_currentFrameNumber >= numFrames)
				{
					m_currentFrameNumber = 0;
				}
			}
		}
	}
}

void AnimationPart::Animate(float frameRate)
{
	if(m_currentSequence != nullptr && frameRate > 0)
	{
		int numFrames = m_currentSequence->Frames()->size();
		if(numFrames > 0)
		{
			// if not finished then move the animation on
			if(!IsFinished())
			{
				// increment the frame time elapsed
				m_frameTimeElapsed += Timing::Instance()->GetLastUpdateDelta();

				// get the amount of time a frame takes
				float timePerFrame = 1 / frameRate;

				// check if it's time to change frame
				if(m_frameTimeElapsed >= timePerFrame)
				{
					// we're done with this frame. decrease the frame time elapsed
					m_frameTimeElapsed -= timePerFrame;

					// increment to the next frame
					m_currentFrameNumber++;
				}
			}
		}
	}
}

void AnimationPart::Animate()
{
	if(m_currentSequence != nullptr)
	{
		Animate(m_currentSequence->Framerate());
	}
}

bool AnimationPart::HasSFXforCurrentFrame() const
{
	if (!m_currentSequence)
	{
		return false;
	}

	return m_currentSequence->HasSFXforFrame(m_currentFrameNumber);
}

std::vector<std::string> AnimationPart::GetSequenceNames() const
{
	std::vector<std::string> retVal;
	retVal.reserve(m_sequences.size());

	for (const auto & kvp : m_sequences)
	{
		retVal.push_back(kvp.first);
	}

	return retVal;
}

