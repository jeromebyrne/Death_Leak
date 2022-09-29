#include "precompiled.h"
#include "GameObject.h"
#include "Material.h"
#include "MaterialManager.h"
#include "DrawUtilities.h"
#include "Game.h"
#include "SolidLineStrip.h"
#include "UIManager.h"

const float kMaxRadians = 6.28318531; // 360 degrees

unsigned int GameObject::sGameObjectId = 1;
int GameObject::sCurrentInteractable = -1;

GameObject::GameObject(float x, float y, DepthLayer depthLayer, float width, float height) :
	m_position(x, y),
	mDepthLayer(depthLayer),
	m_lastPosition(x, y),
	m_dimensions(width, height),
	m_material(nullptr),
	m_rotationAngle(0.0f),
	mShowDebugText(false),
	m_matScaleX(1.0f),
	m_matScaleY(1.0f),
	m_debugDrawVBuffer(nullptr),
	mDrawable(false),
	m_updateable(true),
	mIsSolidSprite(false),
	mIsSolidLine(false),
	mAttachedTo(nullptr),
	mAttachedToOffset(0.0f, 0.0f),
	mParallaxMultiplier(1.0f, 1.0f),
	mCurrentParallaxOffsetX(0.0f),
	mCurrentParallaxOffsetY(0.0f),
	mUpdateToParentsOrientation(false),
	mIsButterfly(false),
	mIsProjectile(false),
	mIsDebris(false),
	mIsCharacter(false),
	mIsParallaxLayer(false),
	mIsAudioObject(false), 
	mIsWaterBlock(false),
	mIsPlatform(false),
	mAutoRotationValue(0.0f),
	mLevelEditLocked(false),
	mLevelEditSelectionDimensions(100.0f, 100.0f),
	mLevelEditShowSelected(false),
	mIsSolidLineStrip(false),
	mAlwaysUpdate(false),
	mIsCurrencyOrb(false),
	mIsBreakable(false),
	mIsBombProjectile(false),
	mIsSmashable(false),
	mIsPlayer(false),
	mIsPickup(false),
	mIsFoliage(false)
{
	m_id = sGameObjectId;
	++sGameObjectId;
}

void GameObject::ResetGameIds()
{
	sGameObjectId = 1;
	sCurrentInteractable = -1;
}

GameObject::~GameObject(void)
{
#ifdef _DEBUG
	if(m_debugDrawVBuffer)
	{
		m_debugDrawVBuffer->Release();
	}
#endif

	Detach();
}

void GameObject::Initialise()
{
	m_lastPosition = m_position;

	// initialise our world matrix
    D3DXMatrixIdentity( &m_world );
	D3DXMATRIX translation;
	D3DXMatrixTranslation(&translation, m_position.X, m_position.Y, (float)mDepthLayer);
	D3DXMatrixMultiply( &m_world, &translation, &m_world); // take the global world into account

	D3DXMatrixIdentity( &m_translation);
	D3DXMatrixIdentity( &m_rotation);
	D3DXMatrixIdentity( &m_matScale );

#ifdef DEBUG
	SetupDebugDraw();
#endif

	if (m_dimensions.X < mLevelEditSelectionDimensions.X)
	{
		mLevelEditSelectionDimensions.X = m_dimensions.X;

		if (mLevelEditSelectionDimensions.X < 10.0f)
		{
			mLevelEditSelectionDimensions.X = 10.0f;
		}
	}
	if (m_dimensions.Y < mLevelEditSelectionDimensions.Y)
	{
		mLevelEditSelectionDimensions.Y = m_dimensions.Y;

		if (mLevelEditSelectionDimensions.Y < 10.0f)
		{
			mLevelEditSelectionDimensions.Y = 10.0f;
		}
	}

	float maxWidth = (std::fmax)(m_dimensions.X, m_dimensions.Y);
	mLargestPossibleDimensions = Vector2(maxWidth, maxWidth);

	// sine wave
	float initialStep = mSineWaveProps.RandomiseInitialStep ? (rand() % 999999) : 0;
	mSinWave.Initialise(initialStep, 
						mSineWaveProps.OffsetY, 
						mSineWaveProps.Amplitude,
						mSineWaveProps.InitialYPosition,
						mSineWaveProps.InitialXPosition, 
						mSineWaveProps.OffsetX);

	mOriginalDimensions = m_dimensions;
}

void GameObject::SetupDebugDraw()
{
	// vertices
	float randR = 0.0f;
	float randG = 0.0f;
	float randB = 0.0f;

	VertexPositionColor vertices[] =
    {
		{ D3DXVECTOR3( -m_dimensions.X/2, -m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( m_dimensions.X/2, -m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( m_dimensions.X/2, m_dimensions.Y/2, 0), D3DXVECTOR4(randR,randG,randB,1)},
        { D3DXVECTOR3( -m_dimensions.X/2, m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)},
		{ D3DXVECTOR3( -m_dimensions.X/2, -m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( -m_dimensions.X/2, m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
		{ D3DXVECTOR3( m_dimensions.X/2, -m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( m_dimensions.X/2, m_dimensions.Y/2, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 

    };

	for(int i = 0; i < 8; i++)
	{
		m_debugDrawVertices[i] = vertices[i];
	}
    
	m_debugDrawVBuffer = 0;
}

void GameObject::Update(float delta)
{
	// reset the world matrix and recalculate transformations
	D3DXMatrixIdentity( &m_world ); 

	float targetDelta = Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (mAutoRotationValue != 0.0f)
	{
		SetRotationAngle(m_rotationAngle + (mAutoRotationValue * percentDelta));
	}

	if (mObjectParallaxEnabled)
	{
		if (mParallaxMultiplier.X != 1.0f)
		{
			float diff = Camera2D::GetInstance()->X() - m_position.X;
			mCurrentParallaxOffsetX = (diff * mParallaxMultiplier.X) - diff;
		}
		if (mParallaxMultiplier.Y != 1.0f)
		{
			float diff = Camera2D::GetInstance()->Y() - m_position.Y;
			mCurrentParallaxOffsetY = (diff * mParallaxMultiplier.Y) - diff;
		}
	}

	if (mSineWaveProps.DoSineWave)
	{
#ifdef _DEBUG
		if (!Game::GetInstance()->GetIsLevelEditMode())
		{
#endif 
			mSinWave.Update(delta);
			m_position.Y = mSinWave.GetValueY();
			m_position.X = mSinWave.GetValueX();

#ifdef _DEBUG
		}
#endif 
	}

	// TODO: need to do all of this if nothing has changed?
	D3DXMatrixScaling(&m_matScale, m_matScaleX, m_matScaleY, 1.0);
	D3DXMatrixTranslation(&m_translation, m_position.X - mCurrentParallaxOffsetX, m_position.Y - mCurrentParallaxOffsetY, (float)mDepthLayer);
	D3DXMatrixRotationZ(&m_rotation, m_rotationAngle);

	// TODO; cane just do 1 multiply somehow?
	D3DXMatrixMultiply(&m_world, &m_translation, &m_world);
	D3DXMatrixMultiply(&m_world, &m_matScale, &m_world);
	D3DXMatrixMultiply(&m_world, &m_rotation, &m_world);

	m_lastPosition = m_position;

	UpdateToParent();

	if (mPositionalAudioEnabled)
	{
		if (!mHasPlayedPositionalAudio)
		{
			mPositionalAudioStartDelay -= delta;

			if (mPositionalAudioStartDelay <= 0.0f)
			{
				mPositionalAudio.Play();
				mHasPlayedPositionalAudio = true;
			}
		}
		else
		{
			mPositionalAudio.Update(delta, m_position);
		}
	}

	if (mInteractableProperties.IsInteractable)
	{
		UpdateInteractable(delta);
	}
}

void GameObject::UpdateInteractable(float delta)
{
#ifdef _DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}
#endif

	auto player = GameObjectManager::Instance()->GetPlayer();

	if (player == this || player == nullptr)
	{
		return;
	}

	bool playerCollision = Utilities::IsSolidSpriteInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y);
	if (playerCollision && (sCurrentInteractable < 0 || ID() == sCurrentInteractable))
	{
		if (!CanInteract())
		{
			return;
		}

		sCurrentInteractable = ID();

		mInteractableProperties.CurrentScreenPos = Utilities::WorldToScreen(m_position + mInteractableProperties.PosOffset);

		const InputManager input = Game::GetInstance()->GetInputManager();

		if (input.IsPressingInteractButton())
		{
			mInteractableProperties.InteractCountdown -= delta;

			mInteractableProperties.CurrentAlpha = 1.0f;

			float percentChange = mInteractableProperties.InteractCountdown / mInteractableProperties.InteractTime;

			if (mInteractableProperties.InteractCountdown <= 0.0f)
			{
				OnInteracted();

				if (mInteractableProperties.DisableInteractivityOnInteract)
				{
					// Disable the interactivity of this object (NOTE: may not work for all objects)
					mInteractableProperties.IsInteractable = false;
				}

				sCurrentInteractable = -1;
			}
		}
		else
		{
			// fade in 
			mInteractableProperties.CurrentAlpha += delta * 3.0f;
			if (mInteractableProperties.CurrentAlpha > 0.75f)
			{
				mInteractableProperties.CurrentAlpha = 0.75f;
			}

			mInteractableProperties.InteractCountdown = mInteractableProperties.InteractTime;
		}

		UIManager::Instance()->AddInteractableToDraw(mInteractableProperties);
	}
	else
	{
		mInteractableProperties.CurrentAlpha = 0.0f;

		mInteractableProperties.InteractCountdown = mInteractableProperties.InteractTime;

		if (sCurrentInteractable == ID())
		{
			sCurrentInteractable = -1;
		}
	}
}

void GameObject:: XmlRead(TiXmlElement * element)
{
	// updateable
	m_updateable = XmlUtilities::ReadAttributeAsBool(element, "", "updateable");

	if (mAlwaysUpdate == false)
	{
		// only read it if it hasn't been forced on
		mAlwaysUpdate = XmlUtilities::ReadAttributeAsBool(element, "", "always_update");
	}

	mLevelEditLocked = XmlUtilities::ReadAttributeAsBool(element, "", "level_edit_locked");

	// position 
	m_position.X = XmlUtilities::ReadAttributeAsFloat(element, "position", "x");
	m_position.Y = XmlUtilities::ReadAttributeAsFloat(element, "position", "y");
	DepthLayer depthLayer = ConvertStringToDepthLayer(XmlUtilities::ReadAttributeAsString(element, "position", "depth_layer"));
	SetDepthLayer(depthLayer);

	//dimensions 
	m_dimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "width");
	m_dimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "height");

	//// read material
	string matStr = string(XmlUtilities::ReadAttributeAsString(element, "material", "value"));
	if(matStr != "null" && matStr != "")
	{
		m_material = MaterialManager::Instance()->GetMaterial(matStr);
	}

	// read rotation
	m_rotationAngle = XmlUtilities::ReadAttributeAsFloat(element, "position", "rotation");
	mAutoRotationValue = XmlUtilities::ReadAttributeAsFloat(element, "position", "auto_rotate_value");

	// sine wave properties
	mSineWaveProps.DoSineWave = XmlUtilities::ReadAttributeAsBool(element, "sine_wave_props", "active");

	if (mSineWaveProps.DoSineWave)
	{
		mSineWaveProps.InitialYPosition = m_position.Y;
		mSineWaveProps.InitialXPosition = m_position.X;
		mSineWaveProps.Amplitude = XmlUtilities::ReadAttributeAsFloat(element, "sine_wave_props", "amplitude");
		mSineWaveProps.OffsetX = XmlUtilities::ReadAttributeAsFloat(element, "sine_wave_props", "x_offset");
		mSineWaveProps.OffsetY = XmlUtilities::ReadAttributeAsFloat(element, "sine_wave_props", "y_offset");
		mSineWaveProps.RandomiseInitialStep = XmlUtilities::ReadAttributeAsBool(element, "sine_wave_props", "rand_init_step");
	}

	mPositionalAudioEnabled = XmlUtilities::ReadAttributeAsBool(element, "pos_audio_props", "enabled");

	if (mPositionalAudioEnabled)
	{
		mPositionalAudio.SetAudioFilename(XmlUtilities::ReadAttributeAsString(element, "pos_audio_props", "file"));
		Vector2 dimensions;
		dimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "pos_audio_props", "dim_x");
		dimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "pos_audio_props", "dim_y");
		mPositionalAudio.SetDimensions(dimensions);
		Vector2 fadeDimensions;
		fadeDimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "pos_audio_props", "fade_dim_x");
		fadeDimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "pos_audio_props", "fade_dim_y");
		mPositionalAudio.SetFadeDimensions(fadeDimensions);
		mPositionalAudioStartDelay = XmlUtilities::ReadAttributeAsFloat(element, "pos_audio_props", "play_delay");
		mInitialPosAudioDelay = mPositionalAudioStartDelay;
		mPositionalAudio.SetRepeat(XmlUtilities::ReadAttributeAsBool(element, "pos_audio_props", "repeat"));

		// positional audio always needs to update as it could be out of update bounds
		mAlwaysUpdate = true;
	}
}

void GameObject::XmlWrite(TiXmlElement * element)
{
	element->SetAttribute("id", Utilities::ConvertDoubleToString(m_id).c_str());

	const char * levelEditLockedFlag = mLevelEditLocked ? "true" : "false";
	element->SetAttribute("level_edit_locked", levelEditLockedFlag);

	const char * updateableFlag = m_updateable ? "true" : "false";
	element->SetAttribute("updateable", updateableFlag);

	const char * alwaysUpdateFlag = mAlwaysUpdate ? "true" : "false";
	element->SetAttribute("always_update", alwaysUpdateFlag);

	// position
	TiXmlElement * posElem = new TiXmlElement("position");
	posElem->SetAttribute("depth_layer", ConvertDepthLayerToString(mDepthLayer).c_str());
	posElem->SetDoubleAttribute("y", m_position.Y);
	posElem->SetDoubleAttribute("x", m_position.X);
	posElem->SetDoubleAttribute("rotation", m_rotationAngle);
	posElem->SetDoubleAttribute("auto_rotate_value", mAutoRotationValue); 
	element->LinkEndChild(posElem);

	// dimensions
	TiXmlElement * dimensionsElem = new TiXmlElement("dimensions");
	dimensionsElem->SetDoubleAttribute("height", m_dimensions.Y);
	dimensionsElem->SetDoubleAttribute("width", m_dimensions.X);
	element->LinkEndChild(dimensionsElem);

	// material
	TiXmlElement * materialElem = new TiXmlElement("material");

	if (m_material != nullptr)
	{
		materialElem->SetAttribute("value", m_material->GetMaterialName().c_str());
	}
	else
	{
		materialElem->SetAttribute("value", "");
	}

	element->LinkEndChild(materialElem);

	TiXmlElement * sineWaveElem = new TiXmlElement("sine_wave_props");
	const char * doSineWaveFlag = mSineWaveProps.DoSineWave ? "true" : "false";
	sineWaveElem->SetAttribute("active", doSineWaveFlag);

	sineWaveElem->SetDoubleAttribute("x_offset", mSineWaveProps.OffsetX);
	sineWaveElem->SetDoubleAttribute("y_offset", mSineWaveProps.OffsetY);
	sineWaveElem->SetDoubleAttribute("amplitude", mSineWaveProps.Amplitude);
	sineWaveElem->SetAttribute("rand_init_step", mSineWaveProps.RandomiseInitialStep);

	element->LinkEndChild(sineWaveElem);

	// Positional Audio
	TiXmlElement * posAudioElem = new TiXmlElement("pos_audio_props");
	posAudioElem->SetAttribute("enabled", mPositionalAudioEnabled ? "true" : "false");
	posAudioElem->SetAttribute("file", mPositionalAudio.GetAudioFilename().c_str());
	posAudioElem->SetDoubleAttribute("dim_x", mPositionalAudio.GetDimensions().X);
	posAudioElem->SetDoubleAttribute("dim_y", mPositionalAudio.GetDimensions().Y);
	posAudioElem->SetDoubleAttribute("fade_dim_x", mPositionalAudio.GetFadeDimensions().X);
	posAudioElem->SetDoubleAttribute("fade_dim_y", mPositionalAudio.GetFadeDimensions().Y);
	posAudioElem->SetDoubleAttribute("play_delay", mInitialPosAudioDelay);
	posAudioElem->SetAttribute("repeat", mPositionalAudio.IsRepeat() ? "true" : "false");

	element->LinkEndChild(posAudioElem);
}

void GameObject::Scale(float xScale, float yScale, bool scalePosition)
{
	if (scalePosition)
	{
		m_position.X = m_position.X * xScale;
		m_position.Y = m_position.Y * yScale;
	}

	m_dimensions.X = m_dimensions.X * xScale;
	m_dimensions.Y = m_dimensions.Y * yScale;

#ifdef DEBUG
	// reset debug draw buffer
	SetupDebugDraw();
#endif
}

void GameObject::DebugDraw(ID3D10Device *  device)
{
	if (Game::GetInstance()->IsLevelEditTerrainMode() && !IsSolidLineStrip())
	{
		// If we are terrain editing then just highlight the terrain (SolidLineStrip)
		return;
	}

	if (!IsSolidLineStrip())
	{
		// get our basic effect to draw our lines
		EffectBasic * basicEffect = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));

		// set the world matrix
		basicEffect->SetWorld((float*)&m_world);

		// set the alpha value
		basicEffect->SetAlpha(1.0f);

		// Set the input layout on the device
		device->IASetInputLayout(basicEffect->InputLayout);

		if (m_debugDrawVBuffer == nullptr)
		{
			D3D10_BUFFER_DESC bd;
			bd.Usage = D3D10_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(m_debugDrawVertices[0]) * 8;
			bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;
			bd.MiscFlags = 0;
			D3D10_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = m_debugDrawVertices;

			device->CreateBuffer(&bd, &InitData, &m_debugDrawVBuffer);
		}

		// Set vertex buffer
		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		device->IASetVertexBuffers(0, 1, &m_debugDrawVBuffer, &stride, &offset);

		// Set primitive topology
		device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

		D3D10_TECHNIQUE_DESC techDesc;
		basicEffect->CurrentTechnique->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			basicEffect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
			device->Draw(8, 0);
		}
	}

	if (mShowDebugText)
	{
		DrawDebugText();
	}
	else
	{
		//  always show the id
		const int bufferSize = 50;
		char array[bufferSize];
		memset(array, 0, bufferSize);
		sprintf(array, "ID: %i", m_id);
		Vector2 pos = Utilities::WorldToScreen(Vector2(m_position.X - mLevelEditSelectionDimensions.X * 0.5f, m_position.Y + (mLevelEditSelectionDimensions.Y * 0.5f) + 25));
		Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);
	}

	if (mLevelEditShowSelected)
	{
		if (IsSolidLineStrip())
		{
			const SolidLineStrip * sls = static_cast<SolidLineStrip *>(this);
			DrawUtilities::DrawTexture(Vector3(m_position.X + sls->CollisionBoxOffset().X, m_position.Y + sls->CollisionBoxOffset().Y, 3), 
										Vector2(sls->CollisionDimensions().X, sls->CollisionDimensions().Y), 
										"Media\\editor\\selected.png");
		}
		else
		{
			DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), 
										Vector2(m_dimensions.X, m_dimensions.Y), 
										"Media\\editor\\selected.png");
		}
		
	}

	DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), Vector2(mLevelEditSelectionDimensions.X, mLevelEditSelectionDimensions.Y), "Media\\editor\\selection.png");

	if (mLevelEditLocked)
	{
		DrawUtilities::DrawTexture(Vector3(m_position.X + (mLevelEditSelectionDimensions.X * 0.5f), m_position.Y - (mLevelEditSelectionDimensions.X * 0.5f),3), Vector2((mLevelEditSelectionDimensions.X * 0.5f), (mLevelEditSelectionDimensions.X * 0.5f)), "Media\\editor\\lock.png");
	}
}

void GameObject::SetRotationAngle(float radians) 
{
	m_rotationAngle = radians;

	if (m_rotationAngle > kMaxRadians || m_rotationAngle < -kMaxRadians)
	{
		m_rotationAngle = 0.0f;
	}
}

void GameObject::DrawDebugText()
{
	// show our object id
	const int bufferSize = 50;
	char array[bufferSize];
	memset(array, 0, bufferSize);
	sprintf(array, "ID: %i", m_id);
	Vector2 pos = Utilities::WorldToScreen(Vector2(m_position.X - 75, m_position.Y + 175));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);

	// show x,y position
	memset(array, 0, bufferSize);
	sprintf(array, "X: %.02f, Y: %.02f", m_position.X, m_position.Y);
	pos = Utilities::WorldToScreen(Vector2(m_position.X - 75, m_position.Y + 150));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);

	// show our z depth
	memset(array, 0, bufferSize);
	sprintf(array, "Depth: %.02f", (float)mDepthLayer);
	pos = Utilities::WorldToScreen(Vector2(m_position.X - 75, m_position.Y + 125));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);

	// material type
	string materialName;
	if (m_material)
	{
		materialName = m_material->GetMaterialName();
	}
	memset(array, 0, bufferSize);
	sprintf(array, "Material: %s", materialName.empty() ? "none" : materialName.c_str());
	pos = Utilities::WorldToScreen(Vector2(m_position.X - 75, m_position.Y + 100));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);
}

string GameObject::GetTypeName()
{
	string typeName = typeid(*this).name();

	// strip off the "class " prefix
	typeName.replace(0,6, "");

	return typeName;
}

void GameObject::AttachTo(std::shared_ptr<GameObject> & parent, Vector2 offset, DepthLayer depthLayer, bool trackParentsOrientation)
{
	GAME_ASSERT(parent);
	if (!parent)
	{
		return;
	}

	mAttachedTo = parent;
	mAttachedToOffset = offset;
	SetDepthLayer(depthLayer);
	mUpdateToParentsOrientation = trackParentsOrientation;
}

void GameObject::Detach()
{
#ifdef _DEBUG
	if (mAttachedTo != nullptr && mAttachedTo->IsDebris())
	{
		bool debug = true;
	}
#endif

	mAttachedTo.reset();
	mAttachedToOffset = Vector2(0.0f, 0.0f);

	// TODO: revert to the previous depth layer?
}

void GameObject::UpdateToParent()
{
	if (mAttachedTo)
	{
		m_position.Y = mAttachedTo->Position().Y + mAttachedToOffset.Y;
		m_position.X = mAttachedTo->Position().X + mAttachedToOffset.X;
	}
	else if (mAttachedToCamera)
	{
		auto cam = Camera2D::GetInstance();
		m_position.Y = cam->Y() + mCameraAttachOffset.Y;
		m_position.X = cam->X() + mCameraAttachOffset.X;
	}
}

void GameObject::SetID(int id)
{
	// only allow setting ID when in level edit mode
	if (!Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}

	m_id = id;
}

void GameObject::SetDepthLayer(DepthLayer depthLayer)
{
	mDepthLayer = depthLayer;

	mParallaxMultiplier = GetParallaxMultipliersForDepthLayer(mDepthLayer);
}

string GameObject::ConvertDepthLayerToString(DepthLayer depthLayer)
{
	switch (depthLayer)
	{
		case kMoon:
		{
			return "kMoon";
		}
		case kFarBackground:
		{
			return "kFarBackground";
		}
		case kMiddleBackground:
		{
			return "kMiddleBackground";
		}
		case kNearBackground:
		{
			return "kNearBackground";
		}
		case kGroundBack:
		{
			return "kGroundBack";
		}
		case kGround:
		{
			return "kGround";
		}
		case kGroundBlood:
		{
			return "kGroundBlood";
		}
		case kNpc:
		{
			return "kNpc";
		}
		case kOrb:
		{
			return "kOrb";
		}
		case kPlayer:
		{
			return "kPlayer";
		}
		case kGroundFront:
		{
			return "kGroundFront";
		}
		case kGhostVomitProjectile:
		{
			return "kGhostVomitProjectile";
		}
		case kBombProjectile:
		{
			return "kBombProjectile";
		}
		case kNinjaStarProjectile:
		{
			return "kNinjaStarProjectile";
		}
		case kPlayerProjectile:
		{
			return "kPlayerProjectile";
		}
		case kImpactCircles:
		{
			return "kImpactCircles";
		}
		case kBloodSpray1:
		{
			return "kBloodSpray1";
		}
		case kFarForeground:
		{
			return "kFarForeground";
		}
		case kMiddleForeground:
		{
			return "kMiddleForeground";
		}
		case kNearForeground:
		{
			return "kNearForeground";
		}
		case kWeatherForeground:
		{
			return "kWeatherForeground";
		}
		case kSolidLines:
		{
			return "kSolidLines";
		}
		default:
		{
			// missing entry
			GAME_ASSERT(false);
			return "kPlayer";
		}
	}
}

GameObject::DepthLayer GameObject::ConvertStringToDepthLayer(string depthLayerString)
{
	if (depthLayerString == "kMoon")
	{
		return kMoon;
	}
	if (depthLayerString == "kFarBackground")
	{
		return kFarBackground;
	}
	else if (depthLayerString == "kMiddleBackground")
	{
		return kMiddleBackground;
	}
	else if (depthLayerString == "kNearBackground")
	{
		return kNearBackground;
	}
	else if (depthLayerString == "kGroundBack")
	{
		return kGroundBack;
	}
	else if (depthLayerString == "kGround")
	{
		return kGround;
	}
	else if (depthLayerString == "kGroundBlood")
	{
		return kGroundBlood;
	}
	else if (depthLayerString == "kNpc")
	{
		return kNpc;
	}
	else if (depthLayerString == "kOrb")
	{
		return kOrb;
	}
	else if (depthLayerString == "kPlayer")
	{
		return kPlayer;
	}
	else if (depthLayerString == "kGroundFront")
	{
		return kGroundFront;
	}
	else if (depthLayerString == "kGhostVomitProjectile")
	{
		return kGhostVomitProjectile;
	}
	else if (depthLayerString == "kBombProjectile")
	{
		return kBombProjectile;
	}
	else if (depthLayerString == "kNinjaStarProjectile")
	{
		return kNinjaStarProjectile;
	}
	else if (depthLayerString == "kPlayerProjectile")
	{
		return kPlayerProjectile;
	}
	else if (depthLayerString == "kImpactCircles")
	{
		return kImpactCircles;
	}
	else if (depthLayerString == "kBloodSpray1")
	{
		return kBloodSpray1;
	}
	else if (depthLayerString == "kFarForeground")
	{
		return kFarForeground;
	}
	else if (depthLayerString == "kMiddleForeground")
	{
		return kMiddleForeground;
	}
	else if (depthLayerString == "kNearForeground")
	{
		return kNearForeground;
	}
	else if (depthLayerString == "kWeatherForeground")
	{
		return kWeatherForeground;
	}
	else if (depthLayerString == "kSolidLines")
	{
		return kSolidLines;
	}
	else
	{
		GAME_ASSERT(false);
		return kPlayer;
	}
}

Vector2 GameObject::GetParallaxMultipliersForDepthLayer(DepthLayer depthLayer)
{
	switch (depthLayer)
	{
		case kMoon:
		{
			return Vector2(0.03f, 0.90f);
		}
		case kFarBackground:
		{
			return Vector2(0.25f, 0.95f);
		}
		case kMiddleBackground: 
		{
			return Vector2(0.5f, 0.97f);
		}
		case kNearBackground:
		{
			return Vector2(0.75f, 0.99f);
		}
		case kGroundBack:
		case kGround:
		case kGroundBlood:
		case kNpc:
		case kOrb:
		case kPlayer:
		case kGhostVomitProjectile:
		case kBombProjectile:
		case kNinjaStarProjectile:
		case kPlayerProjectile:
		case kImpactCircles:
		case kBloodSpray1:
		case kSolidLines:
		case kGroundFront:
		{
			return Vector2(1.0f, 1.0f);
		}
		case kFarForeground:
		{
			return Vector2(1.1f, 1.05f);
		}
		case kMiddleForeground:
		{
			return Vector2(1.3f, 1.08f);
		}
		case kNearForeground:
		case kWeatherForeground:
		{
			return Vector2(1.8f, 1.2f);
		}
		default:
		{
			return Vector2(1.0f, 1.0f);
		}
	}

	return Vector2(1.0f, 1.0f);
}

void GameObject::AttachToCamera(const Vector2 & offset)
{
	mAttachedToCamera = true;
	mCameraAttachOffset = offset;
}

void GameObject::DetachFromCamera()
{
	mAttachedToCamera = false;
}

