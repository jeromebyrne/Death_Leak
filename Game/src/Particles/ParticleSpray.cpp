#include "precompiled.h"
#include "ParticleSpray.h"
#include "particleEmitterManager.h"
#include "Projectile.h"

static const unsigned int kMaxParticlesPerSpray = 100;

ParticleSpray::ParticleSpray(bool isBloodSpray, Vector3 position, Vector3 dimensions, const char* textureFileName, list<Particle> particles, bool isLooping, unsigned long loopTime,
							 bool scaleByLiveTime, float scaleTo)
	: DrawableObject(position.X, position.Y, position.Z, dimensions.X, dimensions.Y, dimensions.Z), 
	m_vertexBuffer(0), 
	m_isLooping(isLooping), 
	m_particleList(particles), 
	m_loopTime(loopTime), 
	m_scalesByLiveTime(scaleByLiveTime), 
	m_scaleTo(scaleTo),
	m_parent(0),
	m_parentOffset(0,0),
	mIsBloodSpray(isBloodSpray),
	m_direction(1,0),
	m_spread(1.0),
	m_minSpeed(5),
	m_maxSpeed(5),
	m_minLivetime(2000),
	m_maxLivetime(4000),
	m_minSize(30),
	m_maxSize(50),
	m_gravity(1),
	m_minBrightness(1.0),
	m_maxBrightness(1.0),
	m_numParticles(10)
{
	m_textureFilename = textureFileName;
	m_startedLooping = Timing::Instance()->GetTotalTimeSeconds();
}

ParticleSpray::~ParticleSpray(void)
{
	m_particleList.clear();

	if (m_parent)
	{
		m_parent->OnParticleSprayDead(this);
		DetachFromSprite();
	}

	ParticleEmitterManager::DecrementParticleWorldCount(m_particleList.size());
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
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

void ParticleSpray::Draw(ID3D10Device* device, Camera2D * camera)
{
	D3DXVECTOR2 tex1 = D3DXVECTOR2(1,1);
	D3DXVECTOR2 tex2 = D3DXVECTOR2(0,1);
	D3DXVECTOR2 tex3 = D3DXVECTOR2(0,0);
	D3DXVECTOR2 tex4 = D3DXVECTOR2(1,0);

	D3DXVECTOR3 normal = D3DXVECTOR3(1,1,1);

	VertexPositionTextureNormal vertices[kMaxParticlesPerSpray * 6];

	float currentTime = Timing::Instance()->GetTotalTimeSeconds();
	int numAliveParticles = 0; // the total number of particles we actually need to render

	int currentVerts = 0;
	for (auto & currentParticle : m_particleList)
	{
		if(!currentParticle.IsDead)
		{
			float size = currentParticle.Size/2;
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
			vertices[currentVerts].Pos = D3DXVECTOR3( posX - size, posY - size, depth + (currentVerts * 0.01));
			vertices[currentVerts].TexCoord = tex1;
			vertices[currentVerts].Normal = normal;

			// 2 vertex
			vertices[currentVerts + 1].Pos = D3DXVECTOR3( posX + size, posY - size, depth + (currentVerts) );
			vertices[currentVerts + 1].TexCoord = tex2;
			vertices[currentVerts + 1].Normal = normal;

			// 3 vertex 
			vertices[currentVerts + 2].Pos = D3DXVECTOR3( posX + size, posY + size, depth);
			vertices[currentVerts + 2].TexCoord = tex3;
			vertices[currentVerts + 2].Normal = normal;

			// 4 vertex
			vertices[currentVerts + 3].Pos = D3DXVECTOR3( posX - size, posY - size, depth);
			vertices[currentVerts + 3].TexCoord = tex1;
			vertices[currentVerts + 3].Normal = normal;

			// 5 vertex
			vertices[currentVerts + 4].Pos = D3DXVECTOR3( posX - size, posY + size, depth);
			vertices[currentVerts + 4].TexCoord = tex4;
			vertices[currentVerts + 4].Normal = normal;

			// 6 vertex
			vertices[currentVerts + 5].Pos = D3DXVECTOR3( posX + size, posY + size, depth);
			vertices[currentVerts + 5].TexCoord = tex3;
			vertices[currentVerts + 5].Normal = normal;

			// just do our particle update logic here rather than looping through all again
			if(currentParticle.Gravity > 0)
			{
				// slow our particles down (air resistance)
				currentParticle.Speed *= 0.99;
			}
			currentParticle.PosX = posX + (currentParticle.DirectionX * currentParticle.Speed);
			currentParticle.PosY = posY + (currentParticle.DirectionY * currentParticle.Speed);
			currentParticle.PosY -= currentParticle.Gravity; // apply gravity

			float death_time = currentParticle.StartTime + currentParticle.MaxLiveTime;
			float time_left = death_time - currentTime;

			// set alpha                                                    
			float alpha = (float)time_left / (float)currentParticle.MaxLiveTime;
			vertices[currentVerts].Normal.x = alpha;
			vertices[currentVerts + 1].Normal.x = alpha;
			vertices[currentVerts + 2].Normal.x = alpha;
			vertices[currentVerts + 3].Normal.x = alpha;
			vertices[currentVerts + 4].Normal.x = alpha;
			vertices[currentVerts + 5].Normal.x = alpha;

			// set the brightness
			float brightness = currentParticle.Brightness;
			vertices[currentVerts].Normal.y = brightness;
			vertices[currentVerts + 1].Normal.y = brightness;
			vertices[currentVerts + 2].Normal.y = brightness;
			vertices[currentVerts + 3].Normal.y = brightness;
			vertices[currentVerts + 4].Normal.y = brightness;
			vertices[currentVerts + 5].Normal.y = brightness;

			// increment size if scalable
			if(m_scalesByLiveTime)
			{
				float startSize = currentParticle.StartSize;
				float finalSize = currentParticle.StartSize * m_scaleTo;
				float sizeDifference = finalSize - startSize;

				float currentLive = currentTime - currentParticle.StartTime;

				// get the current size as a percentage of the final size
				float livePercent = ((float)currentLive/ (float)currentParticle.MaxLiveTime);
				
				float toAdd = sizeDifference * livePercent;

				// get the percentage  value of the size difference and add it
				currentParticle.Size = currentParticle.StartSize + toAdd;
			}

			if(time_left <= 0)
			{
				if(m_isLooping)
				{
					if (m_parent) // we are attached to another object
					{
						if (m_parentHFlipInitial)
						{
							if (m_parent->IsHFlipped())
							{
								currentParticle.PosX = m_parent->X() - m_parentOffset.X;
								currentParticle.PosY = m_parent->Y() - m_parentOffset.Y;
							}
							else
							{
								currentParticle.PosX = m_parent->X() + m_parentOffset.X;
								currentParticle.PosY = m_parent->Y() - m_parentOffset.Y;
							}
						}
						else
						{
							if (m_parent->IsHFlipped())
							{
								currentParticle.PosX = m_parent->X() + m_parentOffset.X;
								currentParticle.PosY = m_parent->Y() - m_parentOffset.Y;
							}
							else
							{
								currentParticle.PosX = m_parent->X() - m_parentOffset.X;
								currentParticle.PosY = m_parent->Y() - m_parentOffset.Y;
							}
						}
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
					ParticleEmitterManager::DecrementParticleWorldCount(1);
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
	if(m_isLooping && m_loopTime > 0)
	{
		float currentTime = Timing::Instance()->GetTotalTimeSeconds();

		if((m_startedLooping + m_loopTime) < currentTime)
		{
			m_isLooping = false;
		}
	}

	if(numAliveParticles > 0)
	{
		SetVertexBuffer(device, sizeof(vertices[0]) * (numAliveParticles * 6), vertices);
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
		device->IASetVertexBuffers(0,1, &m_vertexBuffer, &stride, &offset);

		//// Set primitive topology
		device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		D3D10_TECHNIQUE_DESC techDesc;
		m_currentEffect->CurrentTechnique->GetDesc(&techDesc);
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_currentEffect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
			device->Draw(numAliveParticles * 6, 0);
		}
	}
	else
	{
		// we have no more particles to show, delete this spray
		GameObjectManager::Instance()->RemoveGameObject(this);
	}
	/*else if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
	}*/
}

void ParticleSpray::Update(float delta)
{
	DrawableObject::Update(delta);

	// See Draw() for update logic
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
}

void ParticleSpray::Initialise()
{
	// particles are never updaetable
	// we do our updates in the draw function so we dont have to traverse a potentially huge amount of particles again
	m_updateable = false;

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

void ParticleSpray::AttachToSprite(Sprite * parent, Vector2 offset)
{
	if (!parent)
	{
		GAME_ASSERT(parent);
		return;
	}

	GAME_ASSERT((!dynamic_cast<Projectile*>(parent)));

	m_parent = parent;
	m_parentOffset = offset;
	m_parentHFlipInitial = parent->IsHFlipped();

	m_parent->OnParticleAttached(this);
}

void ParticleSpray::DetachFromSprite()
{
	m_parent = nullptr;
	m_parentOffset = Vector2(0,0);
}
