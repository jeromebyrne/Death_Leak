#include "precompiled.h"
#include "SolidMovingSprite.h"
#include "CollisionManager.h"
#include "drawutilities.h"
#include "ParticleEmittermanager.h"
#include "Projectile.h"
#include "AnimationSkeleton.h"
#include "SolidLineStrip.h"
#include "Breakable.h"
#include "Smashable.h"
#include "Game.h"

extern CSteamAchievements* g_SteamAchievements;

SolidMovingSprite::SolidMovingSprite(float x, float y , DepthLayer depthLayer , float width , float height , float groundFriction , float airResistance ):
	MovingSprite(x,y, depthLayer, width, height, groundFriction, airResistance),
	m_collisionBoxDimensions(width, height),
	m_passive(false), 
	m_onTopOfOtherSolidObject(false), 
	m_collidingAtLeftSideOfObject(false),
	m_collidingAtRightSideOfObject(false),
	m_applyDamage(false),
	m_applyDamageAmount(0.0f),
	mBouncable(false),
	mBounceDampening(1.0f),
	mCollisionBoxOffset(0.0f,0.0f),
	mCanBeDamaged(true),
	mIsOnSolidLine(false),
	mCurrentSolidLineStrip(nullptr)
{
	mIsSolidSprite = true;
}

SolidMovingSprite::~SolidMovingSprite(void)
{
#ifdef _DEBUG
 	if(m_collisionBoxVBuffer)
	{
		m_collisionBoxVBuffer->Release();
	}
#endif
}

void SolidMovingSprite::Update(float delta)
{
	MovingSprite::Update(delta);

	if (mMeleeStrikeCooldown > 0.0f)
	{
		mMeleeStrikeCooldown -= delta;
	}

	bool isOnSolidSurface = IsOnSolidSurface();

	if (isOnSolidSurface)
	{
		SetVelocityY(0.0f);
	}
}

void SolidMovingSprite::SetupDebugDraw()
{
	MovingSprite::SetupDebugDraw();

	//srand(Timing::Instance()->TotalGameTime());
	float randR = 0.0f; //((rand()%100) + 0.1) * 0.01;
	float randG = 0.0f; //((rand()%100) + 0.1) * 0.01;
	float randB = 0.0f; //((rand()%100) + 0.1) * 0.01;

	VertexPositionColor vertices[] =
    {
		{ D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)}, 
        { D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)}, 
        { D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0.0f), D3DXVECTOR4(randR,randG,randB,1.0f)},
        { D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)},
		{ D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)}, 
        { D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)}, 
		{ D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)}, 
        { D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0.0f ), D3DXVECTOR4(randR,randG,randB,1.0f)}
    };

	for(int i = 0; i < 8; i++)
	{
		m_collisionBoxVertices[i] = vertices[i];

		m_collisionBoxVertices[i].Pos.x += mCollisionBoxOffset.X;
		m_collisionBoxVertices[i].Pos.y += mCollisionBoxOffset.Y;
	}
    
	m_collisionBoxVBuffer = nullptr;
}

void SolidMovingSprite::DebugDraw(ID3D10Device *  device)
{
	MovingSprite::DebugDraw(device);

	// get our basic effect to draw our lines
	EffectBasic * basicEffect = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));

	// set the world matrix
	basicEffect->SetWorld((float*)&m_world);

	// set the alpha value
	basicEffect->SetAlpha(1.0f);

	// Set the input layout on the device
	device->IASetInputLayout(basicEffect->InputLayout);
	
	if(m_collisionBoxVBuffer == 0)
	{
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(m_collisionBoxVertices[0]) * 8;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = m_collisionBoxVertices;

		device->CreateBuffer( &bd, &InitData, &m_collisionBoxVBuffer );
	}

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &m_collisionBoxVBuffer, &stride, &offset);

	// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

	D3D10_TECHNIQUE_DESC techDesc;
	basicEffect->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		basicEffect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->Draw(8, 0);
	}

	// narrow phase collision circles
	if ( m_isAnimated )
	{
		AnimationSkeleton * sequenceSkeleton = m_animation->GetSkeletonForCurrentSequence("body");
		if (sequenceSkeleton && m_animation)
		{
			 int frameNum = m_animation->GetPart("body")->FrameNumber();

			 vector<AnimationSkeleton::AnimationSkeletonFramePiece> framePieceList = sequenceSkeleton->GetDataForFrame(frameNum);
			 for (auto & framePiece : framePieceList)
			 {
				 if (!m_horizontalFlip)
				 {
					 Vector2 startPos(m_position.X + framePiece.mStartPos.X, m_position.Y + framePiece.mStartPos.Y);
					 Vector2 endPos(m_position.X + framePiece.mEndPos.X, m_position.Y + framePiece.mEndPos.Y);
					 DrawUtilities::DrawLine(startPos, endPos);
				 }
				 else
				 {
					 Vector2 startPos(m_position.X - framePiece.mStartPos.X, m_position.Y + framePiece.mStartPos.Y);
					 Vector2 endPos(m_position.X - framePiece.mEndPos.X, m_position.Y + framePiece.mEndPos.Y);
					 DrawUtilities::DrawLine(startPos, endPos);
				 }
			 }
		}
	}
}

void SolidMovingSprite::Scale(float xScale, float yScale, bool scalePosition)
{
	m_collisionBoxDimensions.X = m_collisionBoxDimensions.X * xScale;
	m_collisionBoxDimensions.Y = m_collisionBoxDimensions.Y * yScale;

	if (scalePosition)
	{
		mCollisionBoxOffset.X *= xScale;
		mCollisionBoxOffset.Y *= yScale;
	}

	MovingSprite::Scale(xScale, yScale, scalePosition);
}

void SolidMovingSprite :: Initialise()
{
	// update base classes
	MovingSprite::Initialise();

	m_collisionBoxVBuffer = nullptr;
}

void SolidMovingSprite::XmlRead(TiXmlElement * element)
{
	// update base classes
	MovingSprite::XmlRead(element);
	
	// read collision box dimensions
	m_collisionBoxDimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "x");
	m_collisionBoxDimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "y");
	mCollisionBoxOffset.X = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "offsetX");
	mCollisionBoxOffset.Y = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "offsetY");

	// passive or active collision
	m_passive = XmlUtilities::ReadAttributeAsBool(element, "passive", "value");

	// read if this object damages other objects
	m_applyDamage = XmlUtilities::ReadAttributeAsBool(element, "applydamage", "flag"); 
	m_applyDamageAmount = XmlUtilities::ReadAttributeAsFloat(element, "applydamage", "amount"); 

	mCanBeDamaged = XmlUtilities::ReadAttributeAsBool(element, "can_be_damaged", "value"); 

	mIsBouncy = XmlUtilities::ReadAttributeAsBool(element, "is_bouncy", "value");
	mBounceMultiplier = XmlUtilities::ReadAttributeAsFloat(element, "is_bouncy", "bounce_multiplier");
}

void SolidMovingSprite::XmlWrite(TiXmlElement * element)
{
	MovingSprite::XmlWrite(element);

	TiXmlElement * collisionboxdimensions = new TiXmlElement("collisionboxdimensions");
	collisionboxdimensions->SetDoubleAttribute("y", m_collisionBoxDimensions.Y);
	collisionboxdimensions->SetDoubleAttribute("x", m_collisionBoxDimensions.X);
	collisionboxdimensions->SetDoubleAttribute("offsetX", mCollisionBoxOffset.X);
	collisionboxdimensions->SetDoubleAttribute("offsetY", mCollisionBoxOffset.Y);
	element->LinkEndChild(collisionboxdimensions);

	const char * passiveAsStr = m_passive ? "true" : "false";
	TiXmlElement * passive = new TiXmlElement("passive");
	passive->SetAttribute("value", passiveAsStr);
	element->LinkEndChild(passive);

	const char * applydamageAsStr = m_applyDamage ? "true" : "false";
	TiXmlElement * applydamage = new TiXmlElement("applydamage");
	applydamage->SetAttribute("flag", applydamageAsStr);
	applydamage->SetDoubleAttribute("amount", m_applyDamageAmount);
	element->LinkEndChild(applydamage);

	const char * isDamagedAsStr = mCanBeDamaged ? "true" : "false";
	TiXmlElement * canBeDamaged = new TiXmlElement("can_be_damaged");
	canBeDamaged->SetAttribute("value", isDamagedAsStr);
	element->LinkEndChild(canBeDamaged);

	const char * isBouncyAsStr = mIsBouncy ? "true" : "false";
	TiXmlElement * isBouncy = new TiXmlElement("is_bouncy");
	isBouncy->SetAttribute("value", isBouncyAsStr);
	isBouncy->SetDoubleAttribute("bounce_multiplier", mBounceMultiplier);
	element->LinkEndChild(isBouncy);
}

bool SolidMovingSprite::OnCollision(SolidMovingSprite * object)
{
#ifdef _DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return false;
	}
#endif

	if (object->IsSolidLine())
	{
		return false;
	}

	if (object->IsBreakable())
	{
		return false;
	}

	if (object->IsPickup())
	{
		return false;
	}

	if (object->IsSmashable())
	{
		Smashable * smashable = static_cast<Smashable*>(object);
		if (smashable->GetSmashableState() == Smashable::kSmashed)
		{
			return false;
		}
	}

	if (object->IsProjectile())
	{
		Projectile * asProjectile = static_cast<Projectile*>(object);

		if (asProjectile->GetProjectileType() == Projectile::kBloodFXProjectile)
		{
			// blood projectiles only affect solid lines
			return false;
		}
	}

	float objectYVelocity = object->VelocityY();

	float sinkValue = 5.0f; // how much the object is "sunk" into the other object

	// if we are not passive then push ourselves away from the object
	// remember this behaviour can be overwritten in derived function
	if(!m_passive && !object->IsProjectile()) // by default we don't want to be pushed by a projectile
	{
		float otherLeft = object->CollisionLeft();
		float otherRight = object->CollisionRight();
		float otherTop = object->CollisionTop();
		float otherBottom = object->CollisionBottom();
		float otherX = object->X() + object->CollisionBoxOffset().X;
		float otherY = object->Y() + object->CollisionBoxOffset().Y;

		float thisX = this->X() + CollisionBoxOffset().X;
		float thisY = this->Y() + CollisionBoxOffset().Y;
		float thisLeft = this->CollisionLeft();
		float thisRight = this->CollisionRight();
		float thisTop = this->CollisionTop();
		float thisBottom = this->CollisionBottom();
	
		// get the overlap
		float xOverlap = 0.0f;
		float yOverlap = 0.0f;

		bool liesLeftOf = false;

		// if this lies left of the other object
		if(thisX < otherX)
		{
			xOverlap = otherLeft - thisRight;
			liesLeftOf = true;
		}
		else // it lies right of the centre
		{
			xOverlap = otherRight - thisLeft;
		}

		// below
		if(thisY < otherY)
		{
			yOverlap = otherBottom - thisTop;
		}
		else // above
		{
			yOverlap = (otherTop) - thisBottom;
		}
		
		// on top of the object
		if(thisBottom >= otherY)
		{
			m_onTopOfOtherSolidObject = true;
		}
		else
		{
			m_onTopOfOtherSolidObject = false;
		}

		float tempXoverlap = xOverlap;
		float tempYoverlap = yOverlap;
		
		// if the overlap is negative then change to positive by multiplying by -1
		// this is only done to compare the size of the xOverlap and yOverlap,
		// these new values are stored in temporary variables and the originals are
		// not touched
		if(tempXoverlap < 0.0f)
		{
			tempXoverlap = (-tempXoverlap-1.0f);
		}
		else{ tempXoverlap += 1.0f; }
		if(tempYoverlap < 0.0f)
		{
			tempYoverlap = (-tempYoverlap-1.0f);
		}
		else{ tempYoverlap += 0.0f; }
		
		// now determine which overlap is greater,
		// we are going to move the point along the axis which
		// has the shortest overlap.
		if(tempXoverlap <= tempYoverlap)
		{
			m_position.X += xOverlap;

			if (thisBottom > otherBottom)
			{
				if (liesLeftOf)
				{
					m_collidingAtLeftSideOfObject = true;
				}
				else
				{
					m_collidingAtRightSideOfObject = true;
				}
			}

			if (mBouncable)
			{
				// reverse the velocity and dampen it
				m_velocity.X = m_velocity.X * -mBounceDampening;
			}
		}
		else
		{
			if(m_onTopOfOtherSolidObject && !IsDebris()) // only push ourselves away if we are on top, this prevnts objects sinking with pressure
			{
				m_position.Y = otherTop + (((CollisionDimensions().Y * 0.5f) + CollisionBoxOffset().Y) - sinkValue);

				if (!mBouncable && !object->IsBouncy())
				{
					if (m_velocity.Y < 1.0f)
					{
						m_velocity.Y = 0.0f;
					}

					StopYAccelerating();
				}

				// TODO: only collision with solid lines will cause bounce now
				/*
				if (mBouncable && m_velocity.Y < -2.0f)             
				{
					m_velocity.Y = m_velocity.Y * -mBounceDampening;
					m_position.Y += (sinkValue);                                                
				}
				*/

				if (object->IsBouncy() &&
					m_velocity.Y < -5.0f)
				{
					float maxBounceVelY = 50.0f;
					float newYVelocity = m_velocity.Y * (-object->GetBounceMultiplier());
					if (newYVelocity > maxBounceVelY)
					{
						newYVelocity = maxBounceVelY;
					}
					SetVelocityY(newYVelocity);
					m_position.Y += (sinkValue + 1.0f);
				}
			}
			else
			{
				m_position.Y += yOverlap;
			}
		}
	}

	if (m_applyDamage)
	{
		if (object->IsCharacter())
		{
			GAME_ASSERT(dynamic_cast<Character*>(object));
			Character * character = static_cast<Character*>(object);
			character->OnDamage(this, m_applyDamageAmount, Vector2(0.0f,0.0f));

			// The following is GARBAGE code for an achievement added at the 11th hour
			if (character->IsPlayer())
			{
				if (character->GetHealth() <= 0.0f)
				{
					if (ID() == 14)
					{
						if (g_SteamAchievements)
							g_SteamAchievements->SetAchievement("ACH_FIRE_DEATH");
					}
				}
			}
		}
	}

	return true;
}

void SolidMovingSprite::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	if (!IsOnSolidSurface() && IsCharacter() && GameObjectManager::Instance()->GetPlayer() != this)
	{
		// if an enemy is hit while in midair then damage should be increased
		damageAmount *= 10.0f; // TODO: this should be config driven
	}

	if (mCanBeDamaged)
	{
		MovingSprite::OnDamage(damageDealer, damageAmount, pointOfContact);

		mShowingBurstTint = true;
		mBurstTintStartTime = Timing::Instance()->GetTotalTimeSeconds();

		Vector2 pos = Vector2(m_position.X + pointOfContact.X, m_position.Y + pointOfContact.Y);

		if (damageDealer && damageDealer->IsProjectile())
		{
			// TODO: optimize this to not be a particle spray
			ParticleSpray * spray =
				ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
					pos,
					GameObject::kImpactCircles,
					Vector2(-m_direction.X, 0.0f),
					0.0f,
					Vector2(3200.0f, 1200.0f),
					"Media\\blast_circle.png",
					0.00f,
					0.00f,
					0.30f,
					0.30f,
					20.0f,
					20.0f,
					0.0f,
					false,
					1.0f,
					1.0f,
					10000.0f,
					true,
					3.0f,
					0.0f,
					0.0f,
					0.0f,
					0.3f);
		}
	}
}

bool SolidMovingSprite::IsOnSolidSurface() const
{
	if (IsOnSolidLine())
	{
		return true;
	}

	if (GetIsCollidingOnTopOfObject() &&
		!GetIsCollidingAtObjectSide())
	{
		return true;
	}
	
	return false;
}

void SolidMovingSprite::SetIsOnSolidLine(bool value, SolidLineStrip * lineStrip)
{
	mIsOnSolidLine = value;

	if (mIsOnSolidLine)
	{
		GAME_ASSERT(lineStrip);
		mCurrentSolidLineStrip = lineStrip;

		mIsOnSolidlineThisFrame = true; // gets reset in post update
	}
	else
	{
		mCurrentSolidLineStrip = nullptr;
	}
}

void SolidMovingSprite::DoWaterAccelerationBubbles()
{
	if (mTimeUntilCanSpawnWaterBubbles <= 0.0f)
	{
		ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
																Vector2((m_position.X - (m_direction.X < 0.0f ? 30 : -30)) + mCollisionBoxOffset.X, 
																		m_position.Y + mCollisionBoxOffset.Y),
																GetDepthLayer(),
																Vector2(-m_direction.X, -m_direction.Y),
																0.15f,
																Vector2(3200.0f, 2000.0f),
																"Media\\Ambient\\bubble.png",
																0.15f,
																0.75f,
																1.5f,
																3.0f,
																4.0f,
																8.0f,
																-0.2f,
																false,
																1.0f,
																1.0f,
																-1.0f,
																true,
																2.0f,
																0.01f * Dimensions().X,
																0.03f * Dimensions().Y,
																0.0f,
																0.9f);

		mTimeUntilCanSpawnWaterBubbles = 0.2f;
	}
}

void SolidMovingSprite::TriggerMeleeCooldown()
{
	mMeleeStrikeCooldown = 0.5f;
}

void SolidMovingSprite::PostUpdate(float delta)
{
	MovingSprite::PostUpdate(delta);

	mIsOnSolidlineThisFrame = false;
}
