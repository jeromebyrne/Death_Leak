#include "precompiled.h"
#include "Animation.h"
#include "XmlDocument.h"

#include <stdexcept>

Animation::Animation(const char* filename)
{
	if(filename == 0 || strlen(filename) <= 0)
	{
		throw std::runtime_error("invalid filename for animation");
	}

	ReadXml(filename);
}

Animation::~Animation(void)
{
} 

void Animation::ReadXml(const char* filename)
{
	m_animationParts.clear(); // clear our parts list first

	// create a new xml document 
	XmlDocument doc;
	doc.Load(filename);

	TiXmlHandle * hdoc = doc.Handle();
	TiXmlElement * root = hdoc->FirstChildElement().Element();

	if(root != 0)
	{
		TiXmlElement * child = root->FirstChildElement();

		// loop through children
		while(child)
		{
			// whats the name of this part
			std::string partName = (std::string)child->Value();

			// add this part to the part map
			m_animationParts[partName] = new AnimationPart(child);

			// move to the next game object
			child = child->NextSiblingElement();
		}
	}
}

AnimationPart* Animation::GetPart(const std::string & partName)
{
	if(!partName.empty())
	{
		AnimationPart * part = m_animationParts[partName];

		return part; 
	}

	return nullptr;
}

void Animation::SetPartSequence(const std::string & partName, const std::string & sequenceName)
{
	AnimationPart* part = GetPart(partName);

	if(part != nullptr)
	{
		part->SetSequence(sequenceName);
	}
}

AnimationSkeleton * Animation::GetSkeletonForCurrentSequence(const std::string & bodyPart)
{
	AnimationSequence * currentSequence = m_animationParts[bodyPart]->CurrentSequence();
	GAME_ASSERT(currentSequence);

	if (!currentSequence)
	{
		return nullptr;
	}

	return currentSequence->GetSkeleton();
}

std::vector<std::string> Animation::GetSequenceNamesForBodyPart(const std::string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return std::vector<std::string>();
	}

	return part->GetSequenceNames();
}

void Animation::ScaleSkeleton(float value)
{
	for (auto & kvp : m_animationParts)
	{
		auto & sequences = kvp.second->GetAllSequences();

		for (auto & kvp : sequences)
		{
			auto skeleton = kvp.second->GetSkeleton();

			if (skeleton)
			{
				skeleton->ScaleBones(value);
			}
		}
	}
}

void Animation::JumpToNextFrame(const std::string & bodyPart)
{
	// move to the next fram in the curent sequence
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return;
	}

	auto sequence = part->CurrentSequence();
	if (sequence == nullptr)
	{
		return;
	}

	int numFrames = (int)sequence->Frames()->size();

	int nextFrame = part->FrameNumber() + 1;

	if (nextFrame >= numFrames)
	{
		nextFrame = 0;
	}

	part->SetFrame(nextFrame);
}

void Animation::JumpToPreviousFrame(const std::string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return;
	}

	auto sequence = part->CurrentSequence();
	if (sequence == nullptr)
	{
		return;
	}

	int numFrames = (int)sequence->Frames()->size();

	int nextFrame = part->FrameNumber() - 1;

	if (nextFrame < 0)
	{
		nextFrame = numFrames - 1;
	}

	part->SetFrame(nextFrame);
}

void Animation::JumpToNextSequence(const std::string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return;
	}

	auto sequence = part->CurrentSequence();
	if (sequence == nullptr)
	{
		return;
	}

	auto allSeq = part->GetAllSequences();

	auto seqIter = allSeq.find(sequence->Name());
	if (seqIter == allSeq.end())
	{
		return;
	}
	++seqIter;

	if (seqIter == allSeq.end())
	{
		seqIter = allSeq.begin();
	}

	part->SetSequence(seqIter->first);
}

void Animation::JumpToPreviousSequence(const std::string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return;
	}

	auto sequence = part->CurrentSequence();
	if (sequence == nullptr)
	{
		return;
	}

	auto allSeq = part->GetAllSequences();

	if (allSeq.size() < 2)
	{
		return;
	}

	auto seqIter = allSeq.find(sequence->Name());
	
	if (seqIter == allSeq.begin())
	{
		seqIter = allSeq.end();
	}
	--seqIter;

	part->SetSequence(seqIter->first);
}

int Animation::CurrentFrame(const std::string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return -1;
	}

	return part->FrameNumber();
}

const std::string & Animation::CurrentSequenceName(const std::string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		static const std::string kEmptySequenceName;
		return kEmptySequenceName;
	}

	auto sequence = part->CurrentSequence();

	if (!sequence)
	{
		static const std::string kEmptySequenceName;
		return kEmptySequenceName;
	}

	return sequence->Name();
}

