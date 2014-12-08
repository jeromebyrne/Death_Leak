#include "precompiled.h"
#include "AnimationSequence.h"
#include "AnimationSkeleton.h"

AnimationSequence::AnimationSequence(TiXmlElement * element) :
	mSkeleton(nullptr)
{
	mSkeleton = new AnimationSkeleton();
	m_frames = new list<ID3D10ShaderResourceView*>();
	if(element == nullptr)
	{
		throw new exception("cannot load animation part, null element");
	}
	ReadXml(element);

	// scale the bones according to backbuffer dimensions
	ScaleBones();
}

AnimationSequence::~AnimationSequence(void)
{
	if (mSkeleton)
	{
		delete mSkeleton;
		mSkeleton = nullptr;
	}
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

		list<AnimationSkeleton::AnimationSkeletonFramePiece> skeletonPiecesList;
		if (skeleton_root )
		{
			string root_name = skeleton_root->Value();
			if (root_name == "skeletonparts")
			{
				// loop through each skeleton part and add to skeleton map
				TiXmlElement * skeleton_part = skeleton_root->FirstChildElement();

				while (skeleton_part)
				{
					AnimationSkeleton::AnimationSkeletonFramePiece piece;
					piece.mStartPos.X = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "start_x");
					piece.mStartPos.Y = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "start_y");
					piece.mEndPos.X = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "end_x");
					piece.mEndPos.Y = XmlUtilities::ReadAttributeAsFloat(skeleton_part, "", "end_y");

					skeletonPiecesList.push_back(piece);
					
					skeleton_part = skeleton_part->NextSiblingElement();
				}
			}
		}

		GAME_ASSERT(mSkeleton);
		if (mSkeleton)
		{
			mSkeleton->PopulateFrameData(frame_count, skeletonPiecesList);
		}

		child = child->NextSiblingElement();

		++frame_count;
	}
}

void AnimationSequence::ScaleBones()
{
	/*
	// SCALE
	// get the backbuffer width and height and determine how much we need to scale by
	// base width and height are 1920x1080 (need to not hard code this)
	float bbWidth = Graphics::GetInstance()->BackBufferWidth();
	float bbHeight = Graphics::GetInstance()->BackBufferHeight();

	float scaleX = bbWidth / 1920;
	float scaleY = bbHeight / 1080;

	for (map<int, vector<SkeletonPart>>::iterator map_iter = m_skeletonParts.begin(); map_iter != m_skeletonParts.end(); ++map_iter)
	{
		vector<SkeletonPart>::iterator vec_iter = map_iter->second.begin();

		for (; vec_iter != map_iter->second.end(); ++vec_iter)
		{
			SkeletonPart &part = (*vec_iter);

			part.Offset = Vector2(part.Offset.X * scaleX, part.Offset.Y * scaleY);
			part.Radius = part.Radius * scaleX;
		}
	}
	*/
}
