#include "precompiled.h"
#include "GameObject.h"
#include "Material.h"
#include "MaterialManager.h"
#include "DrawUtilities.h"
#include "Game.h"

const float kMaxRadians = 6.28318531; // 360 degrees

GameObject::GameObject(float x, float y , float z, float width, float height, float breadth):
	m_position(x,y,z),
	m_lastPosition(x,y,z),
	m_dimensions(width, height, breadth), 
	m_material(0),
	m_rotationAngle(0),
	mShowDebugText(0),
	m_matScaleX(1.0f),
	m_matScaleY(1.0f),
	m_debugDrawVBuffer(0),
	mClonedXml(nullptr),
	mDrawable(false),
	m_updateable(true),
	mIsSolidSprite(false),
	mIsSolidLine(false),
	mAttachedTo(nullptr),
	mAttachedToOffset(0,0,0),
	mParallaxMultiplierX(1.0f),
	mCurrentParallaxOffsetX(0.0f),
	mParallaxMultiplierY(1.0f),
	mCurrentParallaxOffsetY(0.0f),
	mUpdateToParentsOrientation(false),
	mIsButterfly(false),
	mIsProjectile(false),
	mIsOrb(false),
	mIsCharacter(false),
	mIsParallaxLayer(false),
	mIsAudioObject(false),
	mIsWaterBlock(false),
	mIsPlatform(false)
{
	static int GAME_OBJECT_ID = 1; 
	m_id = GAME_OBJECT_ID;
	GAME_OBJECT_ID++;
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
	srand(timeGetTime());
	m_lastPosition = m_position;

	// initialise our world matrix
    D3DXMatrixIdentity( &m_world );
	D3DXMATRIX translation;
	D3DXMatrixTranslation(&translation, m_position.X, m_position.Y, m_position.Z);
	D3DXMatrixMultiply( &m_world, &translation, &m_world); // take the global world into account

	D3DXMatrixIdentity( &m_translation);
	D3DXMatrixIdentity( &m_rotation);
	D3DXMatrixIdentity( &m_matScale );

#ifdef DEBUG
	SetupDebugDraw();
#endif
}

void GameObject::SetupDebugDraw()
{
	// vertices
	float randR = 1;
	float randG = 1;
	float randB = 1;

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

	if (mParallaxMultiplierX != 1.0f)
	{
		float diff = Camera2D::GetInstance()->X() - m_position.X;
		mCurrentParallaxOffsetX = (diff * mParallaxMultiplierX) - diff;
	}
	if (mParallaxMultiplierY != 1.0f)
	{
		float diff = Camera2D::GetInstance()->Y() - m_position.Y;
		mCurrentParallaxOffsetY = (diff * mParallaxMultiplierY) - diff;
	}

	D3DXMatrixScaling(&m_matScale, m_matScaleX, m_matScaleY, 1.0);
	D3DXMatrixTranslation(&m_translation, m_position.X - mCurrentParallaxOffsetX, m_position.Y - mCurrentParallaxOffsetY, m_position.Z);
	D3DXMatrixRotationZ(&m_rotation, m_rotationAngle);

	D3DXMatrixMultiply(&m_world, &m_translation, &m_world);
	D3DXMatrixMultiply(&m_world, &m_matScale, &m_world);
	D3DXMatrixMultiply(&m_world, &m_rotation, &m_world);

	m_lastPosition = m_position;

	UpdateToParent();
}

void GameObject:: XmlRead(TiXmlElement * element)
{
#if _DEBUG
	mClonedXml = element->Clone();
#endif

	// updateable
	m_updateable = XmlUtilities::ReadAttributeAsBool(element, "", "updateable");

	// position 
	m_position.X = XmlUtilities::ReadAttributeAsFloat(element, "position", "x");
	m_position.Y = XmlUtilities::ReadAttributeAsFloat(element, "position", "y");
	m_position.Z = XmlUtilities::ReadAttributeAsFloat(element, "position", "z");

	//dimensions 
	m_dimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "width");
	m_dimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "height");
	m_dimensions.Z = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "breadth");

	mParallaxMultiplierX = XmlUtilities::ReadAttributeAsFloat(element, "", "parallax_x");
	mParallaxMultiplierY = XmlUtilities::ReadAttributeAsFloat(element, "", "parallax_y");

	//// read material
	string matStr = string(XmlUtilities::ReadAttributeAsString(element, "material", "value"));
	if(matStr != "null" && matStr != "")
	{
		m_material = MaterialManager::Instance()->GetMaterial(matStr);
	}

	// read rotation
	m_rotationAngle = XmlUtilities::ReadAttributeAsFloat(element, "position", "rotation");
}

void GameObject::XmlWrite(TiXmlElement * element)
{
	element->SetAttribute("id", Utilities::ConvertDoubleToString(m_id).c_str());

	element->SetAttribute("parallax_x", Utilities::ConvertDoubleToString(mParallaxMultiplierX).c_str());
	element->SetAttribute("parallax_y", Utilities::ConvertDoubleToString(mParallaxMultiplierY).c_str());

	const char * updateableFlag = m_updateable ? "true" : "false";
	element->SetAttribute("updateable", updateableFlag);

	// position
	TiXmlElement * posElem = new TiXmlElement("position");
	posElem->SetAttribute("z", Utilities::ConvertDoubleToString(m_position.Z).c_str());
	posElem->SetAttribute("y", Utilities::ConvertDoubleToString(m_position.Y).c_str());
	posElem->SetAttribute("x", Utilities::ConvertDoubleToString(m_position.X).c_str());
	posElem->SetAttribute("rotation", Utilities::ConvertDoubleToString(m_rotationAngle).c_str());
	element->LinkEndChild(posElem);

	// dimensions
	TiXmlElement * dimensionsElem = new TiXmlElement("dimensions");
	dimensionsElem->SetAttribute("breadth", Utilities::ConvertDoubleToString(m_dimensions.Z).c_str());
	dimensionsElem->SetAttribute("height", Utilities::ConvertDoubleToString(m_dimensions.Y).c_str());
	dimensionsElem->SetAttribute("width", Utilities::ConvertDoubleToString(m_dimensions.X).c_str());
	element->LinkEndChild(dimensionsElem);


	// material
	TiXmlElement * materialElem = new TiXmlElement("material");

	if (m_material != 0)
	{
		materialElem->SetAttribute("value", m_material->GetMaterialName().c_str());
	}
	else
	{
		materialElem->SetAttribute("value", "");
	}

	element->LinkEndChild(materialElem);
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
	// get our basic effect to draw our lines
	EffectBasic * basicEffect = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));

	// set the world matrix
	basicEffect->SetWorld((float*)&m_world);

	// set the alpha value
	basicEffect->SetAlpha(1.0f);

	// Set the input layout on the device
	device->IASetInputLayout(basicEffect->InputLayout);
	
	if(m_debugDrawVBuffer == 0)
	{
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(m_debugDrawVertices[0]) * 8;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = m_debugDrawVertices;

		device->CreateBuffer( &bd, &InitData, &m_debugDrawVBuffer );
	}

    // Set vertex buffer
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	device->IASetVertexBuffers(0,1, &m_debugDrawVBuffer, &stride, &offset);

	// Set primitive topology
	device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

	D3D10_TECHNIQUE_DESC techDesc;
	basicEffect->CurrentTechnique->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		basicEffect->CurrentTechnique->GetPassByIndex(p)->Apply(0);
		device->Draw(8, 0);
	}

	if (mShowDebugText)
	{
		DrawDebugText();
	}
}

void GameObject::SetRotationAngle(float radians) 
{
	m_rotationAngle = radians;

	if (m_rotationAngle > kMaxRadians || m_rotationAngle < -kMaxRadians)
	{
		m_rotationAngle = 0;
	}
}

void GameObject::DrawDebugText()
{
	// show our object id
	const int bufferSize = 50;
	char array[bufferSize];
	memset(array, 0, bufferSize);
	sprintf(array, "ID: %i", m_id);
	Vector2 pos = Utilities::WorldToScreen(Vector2(m_position.X, m_position.Y + 50));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);

	// show x,y position
	memset(array, 0, bufferSize);
	sprintf(array, "X: %.02f, Y: %.02f", m_position.X, m_position.Y);
	pos = Utilities::WorldToScreen(Vector2(m_position.X, m_position.Y + 25));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);

	// show our z depth
	memset(array, 0, bufferSize);
	sprintf(array, "Depth: %.02f", m_position.Z);
	pos = Utilities::WorldToScreen(Vector2(m_position.X, m_position.Y));
	Graphics::GetInstance()->DrawDebugText(array, pos.X, pos.Y);
}

string GameObject::GetTypeName()
{
	string typeName = typeid(*this).name();

	// strip off the "class " prefix
	typeName.replace(0,6, "");

	return typeName;
}

void GameObject::AttachTo(std::shared_ptr<GameObject> & parent, Vector3 offset, bool trackParentsOrientation)
{
	GAME_ASSERT(parent);
	if (!parent)
	{
		return;
	}

	mAttachedTo = parent;
	mAttachedToOffset = offset;
	mUpdateToParentsOrientation = trackParentsOrientation;
}

void GameObject::Detach()
{
	mAttachedTo = nullptr;
	mAttachedToOffset = Vector3(0, 0, 0);
}

void GameObject::UpdateToParent()
{
}
