#include "precompiled.h"
#include "SolidLine.h"

SolidLine::SolidLine(float x, float y, float z, float width, float height, float breadth) :
	SolidMovingSprite(x, y, z, width, height, breadth),
	mLength(0.0f),
	mDebugLineVBuffer(nullptr)
{
	mIsSolidLine = true;
}

SolidLine::~SolidLine(void)
{
}

void SolidLine::Scale(float xScale, float yScale, bool scalePosition)
{
	SolidMovingSprite::Scale(xScale, yScale, scalePosition);
}

void SolidLine::Update(float delta)
{
	SolidMovingSprite::Update(delta);
}

void SolidLine::Initialise()
{
	// update the base classes
	SolidMovingSprite::Initialise();
}
void SolidLine::XmlRead(TiXmlElement * element)
{
	// update base classes
	SolidMovingSprite::XmlRead(element);

	mStartPos.X = XmlUtilities::ReadAttributeAsFloat(element, "line_start", "x");
	mStartPos.Y = XmlUtilities::ReadAttributeAsFloat(element, "line_start", "y");

	mEndPos.X = XmlUtilities::ReadAttributeAsFloat(element, "line_end", "x");
	mEndPos.Y = XmlUtilities::ReadAttributeAsFloat(element, "line_end", "y");

	CalculateVariables();
}

void SolidLine::XmlWrite(TiXmlElement * element)
{
	SolidMovingSprite::XmlWrite(element);

	TiXmlElement * lineStartElem = new TiXmlElement("line_start");
	lineStartElem->SetAttribute("x", Utilities::ConvertDoubleToString(mStartPos.X).c_str());
	lineStartElem->SetAttribute("y", Utilities::ConvertDoubleToString(mStartPos.Y).c_str());
	element->LinkEndChild(lineStartElem);

	TiXmlElement * lineEndElem = new TiXmlElement("line_end");
	lineEndElem->SetAttribute("x", Utilities::ConvertDoubleToString(mEndPos.X).c_str());
	lineEndElem->SetAttribute("y", Utilities::ConvertDoubleToString(mEndPos.Y).c_str());
	element->LinkEndChild(lineEndElem);
}

void SolidLine::LoadContent(ID3D10Device * graphicsdevice)
{
	SolidMovingSprite::LoadContent(graphicsdevice);
}

void SolidLine::OnCollision(SolidMovingSprite * object)
{
	Player * player = GameObjectManager::Instance()->GetPlayer();

	if (player == object)
	{
		Vector2 intersectPoint;
		bool intersect = Intersect(Vector2(player->CollisionCentreX(), player->CollisionCentreY()), 
								   Vector2(player->CollisionCentreX(), player->CollisionBottom()), intersectPoint);

		if (intersect)
		{
			if (player->VelocityY() <= 0.0f) // if not moving upwards (example: jumping)
			{
				float diffY = intersectPoint.Y - player->CollisionBottom();

				player->SetY(player->Y() + diffY);

				player->StopYAccelerating();
			}
		}
	}
}

void SolidLine::DebugDraw(ID3D10Device *  device)
{
	SolidMovingSprite::DebugDraw(device);

	EffectBasic * basicEffect = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));

	// set the world matrix
	basicEffect->SetWorld((float*)&m_world);

	// set the alpha value
	basicEffect->SetAlpha(1.0f);

	// Set the input layout on the device
	device->IASetInputLayout(basicEffect->InputLayout);

	if (mDebugLineVBuffer == nullptr)
	{
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(mDebugLineVertices[0]) * 8;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = mDebugLineVertices;

		device->CreateBuffer(&bd, &InitData, &mDebugLineVBuffer);
	}

	// Set vertex buffer
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	device->IASetVertexBuffers(0, 1, &mDebugLineVBuffer, &stride, &offset);

	// Set primitive topology
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

	D3D10_TECHNIQUE_DESC techDesc;
	basicEffect->CurrentTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		basicEffect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->Draw(2, 0);
	}
}

void SolidLine::Draw(ID3D10Device * device, Camera2D * camera)
{
	SolidMovingSprite::Draw(device, camera);
}

void SolidLine::CalculateVariables()
{
	if (mStartPos.X >= 0.0f)
	{
		GAME_ASSERT(mEndPos.X < 0.0f);
	}
	else
	{
		GAME_ASSERT(mEndPos.X > 0.0f);
	}

	if (mStartPos.Y >= 0.0f)
	{
		GAME_ASSERT(mEndPos.Y < 0.0f);
	}
	else
	{
		GAME_ASSERT(mEndPos.Y > 0.0f);
	}

	Vector2 lineDirection  = mEndPos - mStartPos;

	mLength = lineDirection.Length();

	if (mLength <= 0)
	{
		GAME_ASSERT(mLength <= 0);
		return;
	}

	mLineDirection.X = lineDirection.X / mLength;
	mLineDirection.Y = lineDirection.Y / mLength;

	mNormal.X = -mLineDirection.Y;
	mNormal.Y = mLineDirection.X;

	// figure out the collision box dimensions based on the line start and end points
	m_collisionBoxDimensions.X = std::abs(mEndPos.X - mStartPos.X);
	m_collisionBoxDimensions.Y = std::abs(mEndPos.Y - mStartPos.Y);

	mCollisionBoxOffset.X = (mEndPos.X + mStartPos.X) * 0.5f;
	mCollisionBoxOffset.Y = (mEndPos.Y + mStartPos.Y) * 0.5f;

	mWorldStartPos = Vector2(m_position.X + mStartPos.X, m_position.Y + mStartPos.Y);
	mWorldEndPos = Vector2(m_position.X + mEndPos.X, m_position.Y + mEndPos.Y);
}

void SolidLine::SetupDebugDraw()
{
	SolidMovingSprite::SetupDebugDraw();

	VertexPositionColor vertices[] =
	{
		{ D3DXVECTOR3(mStartPos.X, mStartPos.Y, 0), D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ D3DXVECTOR3(mEndPos.X, mEndPos.Y, 0), D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f) }
	};

	for (int i = 0; i < 2; ++i)
	{
		mDebugLineVertices[i] = vertices[i];
	}

	mDebugLineVBuffer = nullptr;
}

bool SolidLine::Intersect(Vector2 & otherStart, Vector2 & otherEnd, Vector2 & intersectPointOut)
{
	// Calculate matrix determinants
	float det1 = (mWorldStartPos.X * mWorldEndPos.Y) - (mWorldStartPos.Y * mWorldEndPos.X);
	float det2 = (otherStart.X * otherEnd.Y) - (otherStart.Y * otherEnd.X);
	float det3 = ((mWorldStartPos.X - mWorldEndPos.X) * (otherStart.Y - otherEnd.Y)) - ((mWorldStartPos.Y - mWorldEndPos.Y) * (otherStart.X - otherEnd.X));

	// If third determinant is close to zero then abort: two lines are nearly parallel:
	if (det3 >= -0.00000001f && det3 <= 0.00000001f) return false;

	// Otherwise calculate the point of intersection:
	intersectPointOut.X = (det1 * (otherStart.X - otherEnd.X)) - ((mWorldStartPos.X - mWorldEndPos.X) * det2);
	intersectPointOut.X /= det3;
	intersectPointOut.Y = (det1 * (otherStart.Y - otherEnd.Y)) - ((mWorldStartPos.Y - mWorldEndPos.Y) * det2);
	intersectPointOut.Y /= det3;

	// Make sure the point is along both lines: get it relative to the start point of both lines
	Vector2 r1 = intersectPointOut - mWorldStartPos;
	Vector2 r2 = intersectPointOut - otherStart;

	Vector2 otherLineDistance = otherEnd - otherStart;
	Vector2 otherLineDirection = otherLineDistance;
	otherLineDirection.Normalise();

	// Do a dot product with both line directions to see if it is past the end of either of the two lines:
	float dot1 = r1.Dot(mLineDirection);
	float dot2 = r2.Dot(otherLineDirection);

	// If either dot is negative then the point is past the beginning of one of the lines:
	if (dot1 < 0 || dot2 < 0)
	{
		return false;
	}

	// If either dot exceeds the length of the line then point is past the end of the line:
	if (dot1 > mLength)
	{
		return false;
	}

	if (dot2 > otherLineDistance.Length())
	{
		return false;
	}

	return true;
}