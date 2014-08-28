#include "precompiled.h"
#include "AudioObject.h"
#include "AudioManager.h"

AudioObject::AudioObject(float x, float y, float z, float width, float height):
	GameObject(x,y,z,width,height),
	mAdjustVolumeToCamera(false),
	mRepeat(false),
	mSoundInstance(nullptr),
	mHasStartedPlaying(false)
{ 
	mFadeDimensions = Vector2(m_dimensions.X * 2.0f, m_dimensions.Y * 2.0f);
	mIsAudioObject = true;
}

AudioObject::~AudioObject(void)
{
	if (mSoundInstance)
	{
		mSoundInstance->stop();
		mSoundInstance->drop();
		mSoundInstance = nullptr;
	}
}
	
void AudioObject::Initialise()
{
	GameObject::Initialise();
}

void AudioObject::Update(float delta)
{
	GameObject::Update(delta);

	if (!mHasStartedPlaying)
	{
		mSoundInstance = AudioManager::Instance()->PlaySoundEffect(mAudioFilename, mRepeat, true);
		mHasStartedPlaying = true;
	}

	if (mSoundInstance)
	{
		if (!mRepeat && mSoundInstance->isFinished())
		{
			GameObjectManager::Instance()->RemoveGameObject(this);
		}

		if (mAdjustVolumeToCamera)
		{
			// if the centre of the camera is outside the dimensions of the audio
			// then start decreasing the volume
			Camera2D * camera = Camera2D::GetInstance();

			if (camera->IsCameraOriginInsideObject(this))
			{
				mSoundInstance->setVolume(1.0f);
			}
			else if (camera->IsCameraOriginInsideRect(m_position, Vector2(mFadeDimensions.X * 2.0f, m_dimensions.Y)))
			{
				Vector3 camPos = camera->Position();
				float distX = 0;

				if (camPos.X < m_position.X)
				{
					distX = Left() - camPos.X;
				}
				else 
				{
					distX = camPos.X - Right();
				}

				float fadeLenghtX = mFadeDimensions.X - m_dimensions.X;

				float percent = 1.0f - distX / fadeLenghtX;

				mSoundInstance->setVolume(percent);
			}
			else if (camera->IsCameraOriginInsideRect(m_position, Vector2(m_dimensions.X, mFadeDimensions.Y * 2.0f)))
			{
				Vector3 camPos = camera->Position();
				float distY = 0;

				if (camPos.Y < m_position.Y)
				{
					distY = Bottom() - camPos.Y;
				}
				else 
				{
					distY = camPos.Y - Top();
				}

				float fadeLenghtY = mFadeDimensions.Y - m_dimensions.Y;

				float percent = 1.0f - distY / fadeLenghtY;

				mSoundInstance->setVolume(percent);
			}
			else
			{
				mSoundInstance->setVolume(0.0f);
			}
		}

		mSoundInstance->setPlaybackSpeed(Timing::Instance()->GetTimeModifier());
	}
}

void AudioObject::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);

	mAudioFilename = XmlUtilities::ReadAttributeAsString(element, "audiofile", "value");
	mAdjustVolumeToCamera = XmlUtilities::ReadAttributeAsBool(element, "adjustvolumetocamera", "value");
	mRepeat = XmlUtilities::ReadAttributeAsBool(element, "repeat", "value");

	mFadeDimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "fadedimensions", "x");
	mFadeDimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "fadedimensions", "y");
}

void AudioObject::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * audioFile = new TiXmlElement("audiofile");
	audioFile->SetAttribute("value", mAudioFilename.c_str());
	element->LinkEndChild(audioFile);

	TiXmlElement * adjustVolumeToCam = new TiXmlElement("adjustvolumetocamera");
	adjustVolumeToCam->SetAttribute("value", mAdjustVolumeToCamera ? "true" : "false");
	element->LinkEndChild(adjustVolumeToCam);

	TiXmlElement * repeat = new TiXmlElement("repeat");
	repeat->SetAttribute("value", mRepeat ? "true" : "false");
	element->LinkEndChild(repeat);

	TiXmlElement * fadeDimElem = new TiXmlElement("fadedimensions");
	fadeDimElem->SetDoubleAttribute("x", mFadeDimensions.X);
	fadeDimElem->SetDoubleAttribute("y", mFadeDimensions.Y);
	element->LinkEndChild(fadeDimElem);
}

void AudioObject::SetVolume(float value)
{
	if (mSoundInstance)
	{
		mSoundInstance->setVolume(value);
	}
}