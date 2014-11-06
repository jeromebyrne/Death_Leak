#include "precompiled.h"
#include "AnimationSequence.h"

AnimationSequence::AnimationSequence(TiXmlElement * element)
{
	m_frames = new list<ID3D10ShaderResourceView*>();
	if(element == 0)
	{
		throw new exception("cannot load animation part, null element");
	}
	ReadXml(element);

	// scale the bones according to backbuffer dimensions
	ScaleBones();
}

AnimationSequence::~AnimationSequence(void)
{
}

void AnimationSequence::ReadXml(TiXmlElement * element)
{
	// do stuff

	// Read the name of the sequence.
	m_name = (string)element->Value();
	//Utilities::ToLower(m_name);

	// get frame rate attrib
	m_framerate = XmlUtilities::ReadAttributeAsFloat(element, "", "FrameRate");

	// get the max framerate attrib
	m_maxFramerate = XmlUtilities::ReadAttributeAsFloat(element, "", "MaxFrameRate");

	// get the min framerate attrib
	m_minFramerate = XmlUtilities::ReadAttributeAsFloat(element, "", "MinFrameRate");
	
	//try loading all of the sequence frames
	TiXmlElement * child = element->FirstChildElement();
	
	int frame_count = 0;
	// loop through children
	while(child)
	{
		const char * textureName = XmlUtilities::ReadAttributeAsString(child, "", "filename");

		ID3D10ShaderResourceView* texture = TextureManager::Instance()->LoadTexture(textureName);

		if(texture != 0)
		{
			m_frames->push_back(texture); // add the frame to the list of frames
		}

		// read skeleton data
		TiXmlElement * skeleton_root = child->FirstChildElement();

		vector<SkeletonPart> skel_parts_vector;
		if (skeleton_root )
		{
			string root_name = skeleton_root->Value();
			if (root_name == "skeletonparts")
			{
				// loop through each skeleton part and add to skeleton map
				TiXmlElement * skeleton_part = skeleton_root->FirstChildElement();

				while (skeleton_part)
				{
					SkeletonPart part;
					part.Offset.X = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "xoffset");
					part.Offset.Y = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "yoffset");
					part.Radius = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "radius");

					skel_parts_vector.push_back(part);
					
					skeleton_part = skeleton_part->NextSiblingElement();
				}
			}
		}

		m_skeletonParts[frame_count] = skel_parts_vector; // assign skeleton parts

		child = child->NextSiblingElement();

		++frame_count;
	}
}

vector<AnimationSequence::SkeletonPart> & AnimationSequence::GetSkeletonParts(int frameNum)
{
	return m_skeletonParts[frameNum];
}

void AnimationSequence::ScaleBones()
{
	// SCALE
	// get the backbuffer width and height and determine how much we need to scale by
	// base width and height are 1920x1080 (need to not hard code this)
	float bbWidth = Graphics::GetInstance()->BackBufferWidth();
	float bbHeight = Graphics::GetInstance()->BackBufferHeight();

	float scaleX = bbWidth / 1920;
	float scaleY = bbHeight / 1080;

	map<int, vector<SkeletonPart>>::iterator map_iter = m_skeletonParts.begin();

	for (; map_iter != m_skeletonParts.end(); ++map_iter)
	{
		vector<SkeletonPart>::iterator vec_iter = map_iter->second.begin();

		for (; vec_iter != map_iter->second.end(); ++vec_iter)
		{
			SkeletonPart &part = (*vec_iter);

			part.Offset = Vector2(part.Offset.X * scaleX, part.Offset.Y * scaleY);
			part.Radius = part.Radius * scaleX;
		}
	}
}
