#include "precompiled.h"
#include "SolidMovingSprite.h"
#include "CollisionManager.h"
#include "Environment.h"
#include "drawutilities.h"
#include "ParticleEmittermanager.h"
#include "Projectile.h"

static const float kVisualDamageMaxTime = 0.6f;

SolidMovingSprite::SolidMovingSprite(float x, float y , float z , float width , float height , float breadth , float groundFriction , float airResistance ):
	MovingSprite(x,y,z,width, height, breadth,groundFriction, airResistance),
	m_collisionBoxDimensions(width, height, breadth),
	m_passive(false), 
	m_onTopOfOtherSolidObject(false), 
	m_collidingAtSideOfObject(false),
	m_applyDamage(false),
	m_applyDamageAmount(0),
	m_isBeingDamaged(false),
	m_beingDamagedStartTime(0.0f),
	mBouncable(false),
	mBounceDampening(1.0f),
	mCollisionBoxOffset(0,0),
	mCanBeDamaged(true)
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

	if (m_onTopOfOtherSolidObject && m_velocity.Y < -10)
	{
		// if on top of an object and downward velocity is string then balance it out
		m_velocity.Y = -0.05;
	}
}

void SolidMovingSprite::SetupDebugDraw()
{
	MovingSprite::SetupDebugDraw();

	//srand(Timing::Instance()->TotalGameTime());
	float randR = 0; //((rand()%100) + 0.1) * 0.01;
	float randG = 0; //((rand()%100) + 0.1) * 0.01;
	float randB = 0; //((rand()%100) + 0.1) * 0.01;

	VertexPositionColor vertices[] =
    {
		{ D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0), D3DXVECTOR4(randR,randG,randB,1)},
        { D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)},
		{ D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( - m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
		{ D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f, - m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)}, 
        { D3DXVECTOR3( m_collisionBoxDimensions.X * 0.5f,  m_collisionBoxDimensions.Y * 0.5f, 0 ), D3DXVECTOR4(randR,randG,randB,1)}
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
		std::vector<AnimationSequence::SkeletonPart> skel_parts = m_animation->GetSkeletonPartsCurrentFrame("body");

		Vector3 pos = Vector3(m_position.X, m_position.Y, 1);

		for (int i = 0; i < skel_parts.size(); ++i)
		{
			Vector3 circle_pos;
			
			if (!m_horizontalFlip)
			{
				circle_pos = Vector3(pos.X + skel_parts[i].Offset.X,
											 pos.Y + skel_parts[i].Offset.Y,
											 1);
			}
			else
			{
				circle_pos = Vector3(pos.X - skel_parts[i].Offset.X,
											 pos.Y + skel_parts[i].Offset.Y,
											 1);
			}

			DrawUtilities::DrawCircle( circle_pos , skel_parts[i].Radius );
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

	m_collisionBoxVBuffer = 0;
}

void SolidMovingSprite::XmlRead(TiXmlElement * element)
{
	// update base classes
	MovingSprite::XmlRead(element);
	
	// read collision box dimensions
	m_collisionBoxDimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "x");
	m_collisionBoxDimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "y");
	m_collisionBoxDimensions.Z = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "z");
	mCollisionBoxOffset.X = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "offsetX");
	mCollisionBoxOffset.Y = XmlUtilities::ReadAttributeAsFloat(element, "collisionboxdimensions", "offsetY");

	// passive or active collision
	m_passive = XmlUtilities::ReadAttributeAsBool(element, "passive", "value");

	// read if this object damages other objects
	m_applyDamage = XmlUtilities::ReadAttributeAsBool(element, "applydamage", "flag"); 
	m_applyDamageAmount = XmlUtilities::ReadAttributeAsFloat(element, "applydamage", "amount"); 

	mCanBeDamaged = XmlUtilities::ReadAttributeAsBool(element, "can_be_damaged", "value"); 
}

void SolidMovingSprite::XmlWrite(TiXmlElement * element)
{
	MovingSprite::XmlWrite(element);

	TiXmlElement * collisionboxdimensions = new TiXmlElement("collisionboxdimensions");
	collisionboxdimensions->SetAttribute("z", Utilities::ConvertDoubleToString(m_collisionBoxDimensions.Z).c_str());
	collisionboxdimensions->SetAttribute("y", Utilities::ConvertDoubleToString(m_collisionBoxDimensions.Y).c_str());
	collisionboxdimensions->SetAttribute("x", Utilities::ConvertDoubleToString(m_collisionBoxDimensions.X).c_str());
	collisionboxdimensions->SetAttribute("offsetX", Utilities::ConvertDoubleToString(mCollisionBoxOffset.X).c_str());
	collisionboxdimensions->SetAttribute("offsetY", Utilities::ConvertDoubleToString(mCollisionBoxOffset.Y).c_str());
	element->LinkEndChild(collisionboxdimensions);

	const char * passiveAsStr = m_passive ? "true" : "false";
	TiXmlElement * passive = new TiXmlElement("passive");
	passive->SetAttribute("value", passiveAsStr);
	element->LinkEndChild(passive);

	const char * applydamageAsStr = m_applyDamage ? "true" : "false";
	TiXmlElement * applydamage = new TiXmlElement("applydamage");
	applydamage->SetAttribute("flag", applydamageAsStr);
	applydamage->SetAttribute("amount", Utilities::ConvertDoubleToString(m_applyDamageAmount).c_str());
	element->LinkEndChild(applydamage);

	const char * isDamagedAsStr = mCanBeDamaged ? "true" : "false";
	TiXmlElement * canBeDamaged = new TiXmlElement("can_be_damaged");
	canBeDamaged->SetAttribute("value", isDamagedAsStr);
	element->LinkEndChild(canBeDamaged);
}

void SolidMovingSprite::OnCollision(SolidMovingSprite * object)
{
	// if we are not passive then push ourselves away from the object
	// remember this behaviour can be overwritten in derived function
	if(!m_passive && !dynamic_cast<Projectile*>(object)) // by default we don't want to be pushed by a projectile
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
		float xOverlap;
		float yOverlap;

		// if this lies left of the other object
		if(thisX < otherX)
		{
			xOverlap = otherLeft - thisRight;
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
			mObjectMovingWith = object;
		}
		else
		{
			m_onTopOfOtherSolidObject = false;
			mObjectMovingWith = 0;
		}

		float tempXoverlap = xOverlap;
		float tempYoverlap = yOverlap;
		
		// if the overlap is negative then change to positive by multiplying by -1
		// this is only done to compare the size of the xOverlap and yOverlap,
		// these new values are stored in temporary variables and the originals are
		// not touched
		if(tempXoverlap < 0)
		{
			tempXoverlap = (-tempXoverlap-1);
		}
		else{ tempXoverlap += 1; }
		if(tempYoverlap < 0)
		{
			tempYoverlap = (-tempYoverlap-1);
		}
		else{ tempYoverlap += 1; }
		
		// now determine which overlap is greater,
		// we are going to move the point along the axis which
		// has the shortest overlap.
		if(tempXoverlap <= tempYoverlap)
		{
			m_position.X += xOverlap;
			m_collidingAtSideOfObject = true; // we are colliding mainly with the side of the object

			if (mBouncable)
			{
				// reverse the velocity and dampen it
				m_velocity.X = m_velocity.X * -mBounceDampening;
			}
		}
		else
		{
			if(m_onTopOfOtherSolidObject) // only push ourselves away if we are on top, this prevnts objects sinking with pressure
			{
				m_position.Y += yOverlap;
				StopYAccelerating();
			}

			if (mBouncable)
			{
				m_velocity.Y = m_velocity.Y * -mBounceDampening;
			}
		}
	}

	if (m_applyDamage)
	{
		// check if this object should apply damage to any characters
		Character * character = dynamic_cast<Character*>(object);

		if (character)
		{
			character->OnDamage(m_applyDamageAmount, Vector3(0,0,0));
		}
	}
}

void SolidMovingSprite::Draw(ID3D10Device * device, Camera2D * camera)
{
	if (m_isBeingDamaged && m_currentEffectType == EFFECT_LIGHT_TEXTURE)
	{
		float timeDiff = Timing::Instance()->GetTotalTimeSeconds() - m_beingDamagedStartTime;
		if (timeDiff >= kVisualDamageMaxTime)
		{
			m_isBeingDamaged = false;
		}
		else
		{
			double gb_amount = (double)timeDiff / (double)kVisualDamageMaxTime;
			m_effectLightTexture->SetLightColor((float*)D3DXVECTOR4(1,gb_amount,gb_amount,1));
		}
		
		MovingSprite::Draw(device, camera);
		m_effectLightTexture->SetLightColor((float*)D3DXVECTOR4(1,1,1,1));
	}
	else
	{
		MovingSprite::Draw(device, camera);
	}
}

void SolidMovingSprite::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	if (mCanBeDamaged)
	{
		MovingSprite::OnDamage(damageAmount, pointOfContact);

		m_isBeingDamaged = true;
		m_beingDamagedStartTime = Timing::Instance()->GetTotalTimeSeconds();

		Vector3 pos = Vector3(m_position.X + pointOfContact.X, m_position.Y + pointOfContact.Y, m_position.Z - 1);

		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																pos,
																Vector3(-m_direction.X, 0, 0),
																0.4,
																Vector3(3200, 1200, 0),
																"Media\\explosion_lines.png",
																0.01,
																0.01,
																0.25f,
																0.45f,
																10,
																10,
																0,
																false,
																0.7,
																1.0,
																10000,
																true,
																20);
	}
}
