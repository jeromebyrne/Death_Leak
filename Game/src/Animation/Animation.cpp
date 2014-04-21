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
			//Utilities::ToLower((char*)partName);

			// add this part to the part map
			m_animationParts[partName] = new AnimationPart(child);

			// move to the next game object
			child = child->NextSiblingElement();
		}
	}
}

AnimationPart* Animation::GetPart(string partName)
{
	if(partName.length() != 0)
	{
		AnimationPart * part = m_animationParts[partName];

		return part; 

		//map<string, AnimationPart*>::iterator current = m_animationParts.begin();

		/*for(; current != m_animationParts.end(); current++)
		{GetSkeletonPartsCurrentSequenceFrame
			string currentName = current->first;

			if(partName == currentName)
			{
				return current->second;
			}
		}
		
		return 0;*/

	}

	return 0;
}

void Animation::SetPartSequence(string partName, string sequenceName)
{
	AnimationPart* part = GetPart(partName);

	if(part != 0)
	{
		part->SetSequence(sequenceName);
	}
}

vector<AnimationSequence::SkeletonPart> & Animation::GetSkeletonPartsCurrentFrame(string bodyPart)
{
	return m_animationParts[bodyPart]->GetSkeletonPartsCurrentSequenceFrame();
}
