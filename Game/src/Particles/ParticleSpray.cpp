#include "precompiled.h"
#include "ParticleSpray.h"
#include "particleEmitterManager.h"
#include "Projectile.h"
#include "DrawUtilities.h"
#include "Game.h"

ParticleSpray::ParticleSpray(bool isBloodSpray, 
							Vector2 position,
							DepthLayer depthLayer,
							Vector2 dimensions, 
							const char* textureFileName,
							list<Particle> particles,
							bool isLooping, 
							unsigned long loopTime,
							bool scaleByLiveTime, 
							float scaleTo, 
							float spawnSpreadX,
							float spawnSpreadY)
	: DrawableObject(position.X, position.Y, depthLayer, dimensions.X, dimensions.Y), 
	m_vertexBuffer(nullptr), 
	m_isLooping(isLooping), 
	m_particleList(particles), 
	m_loopTime(loopTime), 
	m_scalesByLiveTime(scaleByLiveTime), 
	m_scaleTo(scaleTo),
	mIsBloodSpray(isBloodSpray),
	m_direction(1.0f,0.0f),
	m_spread(1.0f),
	m_minSpeed(5.0f),
	m_maxSpeed(5.0f),
	m_minLivetime(2000.0f),
	m_maxLivetime(4000.0f),
	m_minSize(30.0f),
	m_maxSize(50.0f),
	m_gravity(1.0f),
	m_minBrightness(1.0f),
	m_maxBrightness(1.0f),
	m_numParticles(10),
	mParentHFlipInitial(false),
	mSpawnSpread(spawnSpreadX, spawnSpreadY),
	mFadeOutPercentTime(0.6f),
	mFadeInPercentTime(0.1f)
{
	m_textureFilename = textureFileName;
	m_startedLooping = Timing::Instance()->GetTotalTimeSeconds();
}

ParticleSpray::~ParticleSpray(void)
{
	m_particleList.clear();

	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

void ParticleSpray::SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[])
{
	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
	}
	
	device->CreateBuffer( &bd, &InitData, &m_vertexBuffer );
	GAME_ASSERT(m_vertexBuffer);
}

void ParticleSpray::Reset()
{
	// This function exists for the level editor
	for (auto & currentParticle : m_particleList)
	{
		int flippedVertical = rand() % 2;
		int flippedHorizontal = rand() % 2;

		if (mSpawnSpread.X == 0.0f)
		{
			currentParticle.StartPosX = m_position.X; // our original start position
		}
		else
		{
			float posXOffset = rand() % ((unsigned)(mSpawnSpread.X * 10.0f) + 1);

			if (flippedHorizontal)
			{
				posXOffset *= -1;
			}

			currentParticle.StartPosX = m_position.X + posXOffset;
		}

		if (mSpawnSpread.Y == 0.0f)
		{
			currentParticle.StartPosY = m_position.Y;
		}
		else
		{
			float posYOffset = rand() % ((unsigned)(mSpawnSpread.Y * 10.0f) + 1);

			if (flippedVertical)
			{
				posYOffset *= -1;
			}
			currentParticle.StartPosY = m_position.Y + posYOffset;
		}
	}
}

void ParticleSpray::Draw(ID3D10Device* device, Camera2D * camera)
{
	SetVertexBuffer(device, sizeof(mVertices[0]) * (m_particleList.size() * 6), mVertices);
	//-----------------------------

	// set the texture.
	m_currentEffect->SetTexture(m_texture);

	// set the alpha value
	m_currentEffect->SetAlpha(m_alpha);

	//// Set the input layout on the device
	device->IASetInputLayout(m_currentEffect->InputLayout);

	// Set vertex buffer
	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	device->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//// Set primitive topology
	// TODO OPTIMIZE: should be able to use a triangle strip here rather than a list
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D10_TECHNIQUE_DESC techDesc;
	m_currentEffect->CurrentTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_currentEffect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->Draw(m_particleList.size() * 6, 0);
	}
}

void ParticleSpray::Update(float delta)
{
	DrawableObject::Update(delta);

	D3DXVECTOR2 tex1 = D3DXVECTOR2(1, 1);
	D3DXVECTOR2 tex2 = D3DXVECTOR2(0, 1);
	D3DXVECTOR2 tex3 = D3DXVECTOR2(0, 0);
	D3DXVECTOR2 tex4 = D3DXVECTOR2(1, 0);

	D3DXVECTOR3 normal = D3DXVECTOR3(1, 1, 1);

	float currentTime = Timing::Instance()->GetTotalTimeSeconds();
	int numAliveParticles = 0; // the total number of particles we actually need to render

	float targetDelta = Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	int currentVerts = 0;
	for (auto & currentParticle : m_particleList)
	{
		if (!currentParticle.IsDead)
		{
			float size = currentParticle.Size / 2;
			float posX = currentParticle.PosX;
			float posY = currentParticle.PosY;
			float depth = 1;

			// set the correct texture coords
			if (currentParticle.FlippedHorizontal)	{ tex1.x = 0; tex2.x = 1; tex3.x = 1; tex4.x = 0; }
			else									{ tex1.x = 1; tex2.x = 0; tex3.x = 0; tex4.x = 1; }

			if (currentParticle.FlippedVertical)	{ tex1.y = 0; tex2.y = 0; tex3.y = 1; tex4.y = 1; }
			else									{ tex1.y = 1; tex2.y = 1; tex3.y = 0; tex4.y = 0; }

			//NOTE: We store individual particle alpha value in the Normal.X component
			// 1 vertex
			mVertices[currentVerts].Pos = D3DXVECTOR3(posX - size, posY - size, depth + (currentVerts * 0.01f));
			mVertices[currentVerts].TexCoord = tex1;
			mVertices[currentVerts].Normal = normal;

			// 2 vertex
			mVertices[currentVerts + 1].Pos = D3DXVECTOR3(posX + size, posY - size, depth + (currentVerts));
			mVertices[currentVerts + 1].TexCoord = tex2;
			mVertices[currentVerts + 1].Normal = normal;

			// 3 vertex 
			mVertices[currentVerts + 2].Pos = D3DXVECTOR3(posX + size, posY + size, depth);
			mVertices[currentVerts + 2].TexCoord = tex3;
			mVertices[currentVerts + 2].Normal = normal;

			// 4 vertex
			mVertices[currentVerts + 3].Pos = D3DXVECTOR3(posX - size, posY - size, depth);
			mVertices[currentVerts + 3].TexCoord = tex1;
			mVertices[currentVerts + 3].Normal = normal;

			// 5 vertex
			mVertices[currentVerts + 4].Pos = D3DXVECTOR3(posX - size, posY + size, depth);
			mVertices[currentVerts + 4].TexCoord = tex4;
			mVertices[currentVerts + 4].Normal = normal;

			// 6 vertex
			mVertices[currentVerts + 5].Pos = D3DXVECTOR3(posX + size, posY + size, depth);
			mVertices[currentVerts + 5].TexCoord = tex3;
			mVertices[currentVerts + 5].Normal = normal;

			// just do our particle update logic here rather than looping through all again
			if (currentParticle.Gravity > 0)
			{
				// slow our particles down (air resistance)
				currentParticle.Speed *= 0.99;
			}

			currentParticle.PosX = posX + (currentParticle.DirectionX * currentParticle.Speed) * percentDelta;
			currentParticle.PosY = posY + (currentParticle.DirectionY * currentParticle.Speed) * percentDelta;
			currentParticle.PosY -= currentParticle.Gravity * percentDelta; // apply gravity

			float death_time = currentParticle.StartTime + currentParticle.MaxLiveTime;
			float time_left = death_time - currentTime;

			// set alpha 
			float percentTimeLeft = time_left / currentParticle.MaxLiveTime;
			if (mFadeOutPercentTime > 0.0f && percentTimeLeft < (1.0f - mFadeOutPercentTime))
			{
				float alpha = percentTimeLeft / (1.0f - mFadeOutPercentTime);
				mVertices[currentVerts].Normal.x = alpha;
				mVertices[currentVerts + 1].Normal.x = alpha;
				mVertices[currentVerts + 2].Normal.x = alpha;
				mVertices[currentVerts + 3].Normal.x = alpha;
				mVertices[currentVerts + 4].Normal.x = alpha;
				mVertices[currentVerts + 5].Normal.x = alpha;
			}
			else if (mFadeInPercentTime > 0.0f && (1.0f - percentTimeLeft) < mFadeInPercentTime)
			{
				float alpha = (1.0f - percentTimeLeft) / mFadeInPercentTime;
				mVertices[currentVerts].Normal.x = alpha;
				mVertices[currentVerts + 1].Normal.x = alpha;
				mVertices[currentVerts + 2].Normal.x = alpha;
				mVertices[currentVerts + 3].Normal.x = alpha;
				mVertices[currentVerts + 4].Normal.x = alpha;
				mVertices[currentVerts + 5].Normal.x = alpha;
			}

			// set the brightness
			float brightness = currentParticle.Brightness;
			mVertices[currentVerts].Normal.y = brightness;
			mVertices[currentVerts + 1].Normal.y = brightness;
			mVertices[currentVerts + 2].Normal.y = brightness;
			mVertices[currentVerts + 3].Normal.y = brightness;
			mVertices[currentVerts + 4].Normal.y = brightness;
			mVertices[currentVerts + 5].Normal.y = brightness;

			// increment size if scalable
			if (m_scalesByLiveTime)
			{
				float startSize = currentParticle.StartSize;
				float finalSize = currentParticle.StartSize * m_scaleTo;
				float sizeDifference = finalSize - startSize;

				float currentLive = currentTime - currentParticle.StartTime;

				// get the current size as a percentage of the final size
				float livePercent = ((float)currentLive / (float)currentParticle.MaxLiveTime);

				float toAdd = sizeDifference * livePercent;

				// get the percentage  value of the size difference and add it
				currentParticle.Size = currentParticle.StartSize + toAdd;
			}

			if (time_left <= 0)
			{
				if (m_isLooping)
				{
					if (mAttachedTo)
					{
						UpdateParticleToParent(currentParticle);
					}
					else
					{
						currentParticle.PosX = currentParticle.StartPosX;
						currentParticle.PosY = currentParticle.StartPosY;
					}

					currentParticle.Speed = currentParticle.StartSpeed;
					currentParticle.Size = currentParticle.StartSize;
					currentParticle.StartTime = Timing::Instance()->GetTotalTimeSeconds();

					numAliveParticles++;
				}
				else
				{
					// this particle is dead
					currentParticle.IsDead = true;
				}
			}
			else
			{
				numAliveParticles++; // render this many particles
			}
		}
		currentVerts += 6;
	}

	// is it time to break our particle loop?
	if (m_isLooping && m_loopTime > 0)
	{
		float currentTime = Timing::Instance()->GetTotalTimeSeconds();

		if ((m_startedLooping + m_loopTime) < currentTime)
		{
			m_isLooping = false;
		}
	}

	if (numAliveParticles < 1)
	{
		// we have no more particles to show, delete this spray
		GameObjectManager::Instance()->RemoveGameObject(this);
	}
}

void ParticleSpray::XmlRead(TiXmlElement * element)
{
	DrawableObject::XmlRead(element);
	
	// this is redundant. See ReadParticleSpray() in GameObjectManager
}

void ParticleSpray::XmlWrite(TiXmlElement * element)
{
	DrawableObject::XmlWrite(element);

	TiXmlElement * numparticles = new TiXmlElement("numparticles");
	numparticles->SetDoubleAttribute("value", m_numParticles);
	element->LinkEndChild(numparticles);

	TiXmlElement * texture = new TiXmlElement("texture");
	texture->SetAttribute("filename", m_textureFilename.c_str());
	element->LinkEndChild(texture);

	TiXmlElement * direction = new TiXmlElement("direction");
	direction->SetDoubleAttribute("x", m_direction.X);
	direction->SetDoubleAttribute("y", m_direction.Y);
	element->LinkEndChild(direction);

	TiXmlElement * spread = new TiXmlElement("spread");
	spread->SetDoubleAttribute("value", m_spread);
	element->LinkEndChild(spread);

	TiXmlElement * speed = new TiXmlElement("speed");
	speed->SetDoubleAttribute("min", m_minSpeed);
	speed->SetDoubleAttribute("max", m_maxSpeed);
	element->LinkEndChild(speed);

	TiXmlElement * livetime = new TiXmlElement("livetime");
	livetime->SetDoubleAttribute("min", m_minLivetime);
	livetime->SetDoubleAttribute("max", m_maxLivetime);
	element->LinkEndChild(livetime);

	TiXmlElement * size = new TiXmlElement("size");
	size->SetDoubleAttribute("min", m_minSize);
	size->SetDoubleAttribute("max", m_maxSize);
	const char * scaleToFlag = m_scalesByLiveTime ? "true" : "false";
	size->SetAttribute("scaleto", scaleToFlag);
	size->SetDoubleAttribute("scaletovalue", m_scaleTo);
	element->LinkEndChild(size);

	TiXmlElement * gravity = new TiXmlElement("gravity");
	gravity->SetDoubleAttribute("value", m_gravity);
	element->LinkEndChild(gravity);

	TiXmlElement * loop = new TiXmlElement("loop");
	const char * loopFlag = m_isLooping ? "true" : "false";
	loop->SetAttribute("value", loopFlag);
	loop->SetDoubleAttribute("looptime", m_loopTime);
	element->LinkEndChild(loop);

	TiXmlElement * brightness = new TiXmlElement("brightness");
	brightness->SetDoubleAttribute("min", m_minBrightness);
	brightness->SetDoubleAttribute("max", m_maxBrightness);
	element->LinkEndChild(brightness);

	TiXmlElement * positionElement = element->FirstChildElement("position");
	positionElement->SetDoubleAttribute("spawn_spread_x", mSpawnSpread.X);
	positionElement->SetDoubleAttribute("spawn_spread_y", mSpawnSpread.Y);

	TiXmlElement * fadePercentTime = new TiXmlElement("fade_time_percent");
	fadePercentTime->SetDoubleAttribute("out", mFadeOutPercentTime);
	fadePercentTime->SetDoubleAttribute("in", mFadeInPercentTime); 
	element->LinkEndChild(fadePercentTime);
}

void ParticleSpray::Initialise()
{
	DrawableObject::Initialise();

	m_effectParticleSpray = static_cast<EffectParticleSpray*>(EffectManager::Instance()->GetEffect("effectparticlespray"));
	m_effectBloodParticleSpray = static_cast<EffectBloodParticleSpray*>(EffectManager::Instance()->GetEffect("effectbloodparticlespray"));

	if (mIsBloodSpray)
	{
		m_currentEffect = m_effectBloodParticleSpray;
	}
	else
	{
		m_currentEffect = m_effectParticleSpray;
	}
}

void ParticleSpray::LoadContent(ID3D10Device* device)
{
	DrawableObject::LoadContent(device);

	m_texture = TextureManager::Instance()->LoadTexture(m_textureFilename.c_str());
}

void ParticleSpray::AttachTo(std::shared_ptr<GameObject> & parent, Vector2 offset, DepthLayer depthLayer, bool trackParentsOrientation)
{
	DrawableObject::AttachTo(parent, offset, depthLayer, trackParentsOrientation);

	Sprite * sprite = dynamic_cast<Sprite *>(parent.get());
	GAME_ASSERT(sprite);
	if (!sprite)
	{
		return;
	}

	mParentHFlipInitial = sprite->IsHFlipped();
}

void ParticleSpray::UpdateParticleToParent(Particle & particle)
{
	if (mAttachedTo)
	{
		GAME_ASSERT(dynamic_cast<Sprite*>(mAttachedTo.get()));

		Sprite * parentSprite = static_cast<Sprite *>(mAttachedTo.get());

		if (mParentHFlipInitial)
		{
			if (parentSprite->IsHFlipped())
			{
				particle.PosX = parentSprite->X() - mAttachedToOffset.X;
				particle.PosY = parentSprite->Y() - mAttachedToOffset.Y;
			}
			else
			{
				particle.PosX = parentSprite->X() + mAttachedToOffset.X;
				particle.PosY = parentSprite->Y() - mAttachedToOffset.Y;
			}
		}
		else
		{
			if (parentSprite->IsHFlipped())
			{
				particle.PosX = parentSprite->X() + mAttachedToOffset.X;
				particle.PosY = parentSprite->Y() - mAttachedToOffset.Y;
			}
			else
			{
				particle.PosX = parentSprite->X() - mAttachedToOffset.X;
				particle.PosY = parentSprite->Y() - mAttachedToOffset.Y;
			}
		}
	}
}

void ParticleSpray::DebugDraw(ID3D10Device *  device)
{
	if (Game::GetInstance()->IsLevelEditTerrainMode() )
	{
		// If we are terrain editing then just highlight the terrain (SolidLineStrip)
		return;
	}

	DrawableObject::DebugDraw(device);

	DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), Vector2(GetLevelEditSelectionDimensions().X, GetLevelEditSelectionDimensions().Y), "Media\\editor\\particles.png");
}

