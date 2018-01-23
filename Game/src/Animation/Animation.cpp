#include "precompiled.h"
#include "Animation.h"

Animation::Animation(const char* filename)
{
	if(filename == 0 || strlen(filename) <= 0)
	{
		throw new exception("invalid filename for animation");
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
			string partName = (string)child->Value();

			// add this part to the part map
			m_animationParts[partName] = new AnimationPart(child);

			// move to the next game object
			child = child->NextSiblingElement();
		}
	}
}

AnimationPart* Animation::GetPart(const string & partName)
{
	if(!partName.empty())
	{
		AnimationPart * part = m_animationParts[partName];

		return part; 
	}

	return nullptr;
}

void Animation::SetPartSequence(const string & partName, const string & sequenceName)
{
	AnimationPart* part = GetPart(partName);

	if(part != nullptr)
	{
		part->SetSequence(sequenceName);
	}
}

AnimationSkeleton * Animation::GetSkeletonForCurrentSequence(const string & bodyPart)
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

void Animation::JumpToNextFrame(const string & bodyPart)
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

	int numFrames = sequence->Frames()->size();

	int nextFrame = part->FrameNumber() + 1;

	if (nextFrame >= numFrames)
	{
		nextFrame = 0;
	}

	part->SetFrame(nextFrame);
}

void Animation::JumpToPreviousFrame(const string & bodyPart)
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

	int numFrames = sequence->Frames()->size();

	int nextFrame = part->FrameNumber() - 1;

	if (nextFrame < 0)
	{
		nextFrame = numFrames - 1;
	}

	part->SetFrame(nextFrame);
}

void Animation::JumpToNextSequence(const string & bodyPart)
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

void Animation::JumpToPreviousSequence(const string & bodyPart)
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

int Animation::CurrentFrame(const string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return -1;
	}

	return part->FrameNumber();
}

const string & Animation::CurrentSequenceName(const string & bodyPart)
{
	AnimationPart * part = GetPart(bodyPart);
	if (part == nullptr)
	{
		return "Invalid body part";
	}

	auto sequence = part->CurrentSequence();

	if (!sequence)
	{
		return "Sequence not set";
	}

	return sequence->Name();
}



