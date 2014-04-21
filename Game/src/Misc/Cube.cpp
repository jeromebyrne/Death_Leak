#include "precompiled.h"
#include "Cube.h"

Cube::Cube(float x, float y, float z, float width, float height, float breadth) : m_position(x,y,z), m_dimensions(width, height, breadth),
NUM_VERTEX(CUBE_NUMVERTICES), NUM_INDEX(CUBE_NUMINDICES), m_orientation(0,0,0)
{
	// Initialize the world matrix
    D3DXMatrixIdentity( &m_world );

	VertexBuffer = 0;
	IndexBuffer = 0;
}

void Cube::Initialise(ID3D10Device* device)
{
	VertexPositionTextureNormal vertices[] =
    {
		{ D3DXVECTOR3( -m_dimensions.x/2, m_dimensions.y/2, -m_dimensions.z/2 ),  D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) },
        { D3DXVECTOR3( m_dimensions.x/2, m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DXVECTOR3( 0.0f, 1.0f, 0.0f )},
        { D3DXVECTOR3( m_dimensions.x/2, m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 1.0f ), D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) },
        { D3DXVECTOR3( -m_dimensions.x/2, m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 1.0f ), D3DXVECTOR3( 0.0f, 1.0f, 0.0f )},

		{ D3DXVECTOR3( -m_dimensions.x/2, -m_dimensions.y/2, -m_dimensions.z/2 ),  D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR3( 0.0f, -1.0f, 0.0f )},
        { D3DXVECTOR3( m_dimensions.x/2, -m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DXVECTOR3( 0.0f, -1.0f, 0.0f ) },
        { D3DXVECTOR3( m_dimensions.x/2, -m_dimensions.y/2, m_dimensions.z/2), D3DXVECTOR2( 1.0f, 1.0f ), D3DXVECTOR3( 0.0f, -1.0f, 0.0f )},
        { D3DXVECTOR3( -m_dimensions.x/2, -m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 1.0f ), D3DXVECTOR3( 0.0f, -1.0f, 0.0f )},

        { D3DXVECTOR3( -m_dimensions.x/2, -m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR3( -1.0f, 0.0f, 0.0f )},
        { D3DXVECTOR3( -m_dimensions.x/2, -m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DXVECTOR3( -1.0f, 0.0f, 0.0f ) },
        { D3DXVECTOR3( -m_dimensions.x/2, m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 1.0f ) , D3DXVECTOR3( -1.0f, 0.0f, 0.0f )},
        { D3DXVECTOR3( -m_dimensions.x/2, m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 1.0f ), D3DXVECTOR3( -1.0f, 0.0f, 0.0f ) },

        { D3DXVECTOR3( m_dimensions.x/2, -m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR3( 1.0f, 0.0f, 0.0f ) },
        { D3DXVECTOR3( m_dimensions.x/2, -m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DXVECTOR3( 1.0f, 0.0f, 0.0f ) },
        { D3DXVECTOR3( m_dimensions.x/2, m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 1.0f ), D3DXVECTOR3( 1.0f, 0.0f, 0.0f )},
        { D3DXVECTOR3( m_dimensions.x/2, m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 1.0f ), D3DXVECTOR3( 1.0f, 0.0f, 0.0f ) },

        { D3DXVECTOR3( -m_dimensions.x/2, -m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f )},
        { D3DXVECTOR3( m_dimensions.x/2, -m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f )},
        { D3DXVECTOR3( m_dimensions.x/2, m_dimensions.y/2, -m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 1.0f ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f ) },
        { D3DXVECTOR3( -m_dimensions.x/2, m_dimensions.y/2, -m_dimensions.z/2), D3DXVECTOR2( 0.0f, 1.0f ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f ) },

        { D3DXVECTOR3( -m_dimensions.x/2, -m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) },
        { D3DXVECTOR3( m_dimensions.x/2, -m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 1.0f, 0.0f ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) },
        { D3DXVECTOR3( m_dimensions.x/2, m_dimensions.y/2, m_dimensions.z/2 ),  D3DXVECTOR2( 1.0f, 1.0f ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) },
        { D3DXVECTOR3( -m_dimensions.x/2, m_dimensions.y/2, m_dimensions.z/2 ), D3DXVECTOR2( 0.0f, 1.0f ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) },

    };
	
	for(int i = 0; i < NUM_VERTEX; i++)
	{
		m_vertices[i] = vertices[i];
	}

	DWORD indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

	for(int i = 0; i < NUM_INDEX; i++)
	{
		m_indices[i] = indices[i];
	}

	SetVertexBuffer(device, sizeof(m_vertices[0]) * NUM_VERTEX, m_vertices);
	SetIndexBuffer(device, sizeof(m_indices[0]) * NUM_INDEX, m_indices);
}

Cube::~Cube(void)
{
	if(VertexBuffer) VertexBuffer->Release();
	if(IndexBuffer) IndexBuffer->Release();
}


void Cube::Update(D3DXMATRIX globalWorld)
{
	D3DXMATRIX translation;
	
	D3DXMATRIX rotTrans;

	D3DXMatrixRotationYawPitchRoll(&rotation, m_orientation.y, m_orientation.x, m_orientation.z);
	D3DXMatrixTranslation(&translation, m_position.x, m_position.y, m_position.z);

	D3DXMatrixMultiply(&rotTrans, &rotation, &translation);
	
	D3DXMatrixMultiply( &m_world, &rotTrans, &globalWorld); // take the global world into account
}

void Cube::SetVertexBuffer(ID3D10Device* device, UINT byteSize, VertexPositionTextureNormal vertices[])
{
	// do stuff

	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    
	device->CreateBuffer( &bd, &InitData, &VertexBuffer );
}

void Cube::SetIndexBuffer(ID3D10Device* device, UINT byteSize, DWORD indices[])
{	
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = byteSize;        
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices;

	device->CreateBuffer( &bd, &InitData, &IndexBuffer );

}
