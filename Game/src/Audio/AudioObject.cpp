#include "precompiled.h"
#include "AudioObject.h"
#include "AudioManager.h"
#include "DrawUtilities.h"

AudioObject::AudioObject(float x, float y, DepthLayer depthLayer, float width, float height):
	GameObject(x,y,depthLayer,width,height),
	mAdjustVolumeToCamera(false),
	mRepeat(false),
	mSoundInstance(nullptr),
	mHasStartedPlaying(false)
{ 
	mFadeDimensions = Vector2(m_dimensions.X * 2.0f, m_dimensions.Y * 2.0f);
	mIsAudioObject = true;
	mAlwaysUpdate = true;
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

		if (mSoundInstance != nullptr && mAdjustVolumeToCamera)
		{
			mSoundInstance->setVolume(0.0f);
		}
		mHasStartedPlaying = true;
	}

	bool hasVolume = true;

	if (mSoundInstance)
	{
		if (!mRepeat && mSoundInstance->isFinished())
		{
			GameObjectManager::Instance()->RemoveGameObject(this);
		}

		Camera2D * camera = Camera2D::GetInstance();
		const Vector3 & camPos = camera->Position();

		if (mAdjustVolumeToCamera)
		{
			float distX = 0.0f;
			if (camPos.X < m_position.X)
			{
				distX = Left() - camPos.X;
			}
			else
			{
				distX = camPos.X - Right();
			}

			if (camera->IsCameraOriginInsideObject(this))
			{
				mSoundInstance->setVolume(1.0f);
			}
			else if (camera->IsCameraOriginInsideRect(m_position, Vector2(mFadeDimensions.X, m_dimensions.Y)))
			{
				float fadeLenghtX = (mFadeDimensions.X - m_dimensions.X) * 0.5f;

				float percent = distX / fadeLenghtX;

				mSoundInstance->setVolume(1.0f - percent);
			}
			else if (camera->IsCameraOriginInsideRect(m_position, Vector2(m_dimensions.X, mFadeDimensions.Y)))
			{
				float distY = 0.0f;

				if (camPos.Y < m_position.Y)
				{
					distY = Bottom() - camPos.Y;
				}
				else 
				{
					distY = camPos.Y - Top();
				}

				float fadeLenghtY = (mFadeDimensions.Y - m_dimensions.Y) * 0.5f;

				float percent =  distY / fadeLenghtY;

				mSoundInstance->setVolume(1.0f - percent);
			}
			else if (camera->IsCameraOriginInsideRect(m_position, mFadeDimensions))
			{
				float fadeLenghtX = (mFadeDimensions.X - m_dimensions.X) * 0.5f;
				float fadeLenghtY = (mFadeDimensions.Y - m_dimensions.Y) * 0.5f;

				float offsetX = 0;
				float offsetY = 0; 

				float xPercent = 0.0f;
				float yPercent = 0.0f;

				if (camPos.X > Right())
				{
					offsetX = camPos.X - Right();
				}
				else if (camPos.X < Left())
				{
					offsetX = std::abs(camPos.X - Left());
				}

				if (fadeLenghtX > 0.0f)
				{
					xPercent = 1.0f - (offsetX / fadeLenghtX);
				}

				if (camPos.Y > Top())
				{
					offsetY = camPos.Y - Top();
				}
				else if (camPos.Y < Bottom())
				{
					offsetY = std::abs(camPos.Y - Bottom());
				}

				if (fadeLenghtY > 0.0f)
				{
					yPercent = 1.0f - (offsetY / fadeLenghtY);
				}

				if (xPercent < yPercent)
				{
					mSoundInstance->setVolume(xPercent);
				}
				else
				{
					mSoundInstance->setVolume(yPercent);
				}
			}
			else
			{
				hasVolume = false;
				mSoundInstance->setVolume(0.0f);
			}
		}

		if (hasVolume)
		{
			float panVal = camPos.X - m_position.X;
			panVal /= mFadeDimensions.X;
			mSoundInstance->setPan(panVal);

			mSoundInstance->setPlaybackSpeed(Timing::Instance()->GetTimeModifier());
		}
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

void AudioObject::DebugDraw(ID3D10Device *  device)
{
	GameObject::DebugDraw(device);

	DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), Vector2(GetLevelEditSelectionDimensions().X, GetLevelEditSelectionDimensions().Y), "Media\\editor\\audio.png");
}