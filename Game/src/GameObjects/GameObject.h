#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

class Material;// forward declaration

class GameObject
{
private:
	int m_id; // our game object ID (this is unique)

	D3DXMATRIX m_translation;
	D3DXMATRIX m_rotation;
	D3DXMATRIX m_matScale;

	VertexPositionColor m_debugDrawVertices[8]; 
	// create a vertex buffer
	ID3D10Buffer* m_debugDrawVBuffer;

	// used in the level editor for copying objects,
	// not valid in release mode
	TiXmlNode * mClonedXml;

protected:
	Vector3 m_position;
	float m_rotationAngle;
	float m_matScaleX;
	float m_matScaleY;
	Vector3 m_lastPosition; // stores the objects last position before update
	Vector3 m_dimensions;
	bool m_updateable; // should this object be updated
	D3DXMATRIX m_world; // world matrix
	Material * m_material; // the objects material
	virtual void SetupDebugDraw();

	bool mShowDebugText;

	virtual void DrawDebugText();
public:

	GameObject(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1);
	virtual ~GameObject(void);

	bool IsUpdateable() const { return m_updateable; }

	inline int ID() const 
	{
		return m_id;
	}

	TiXmlNode * GetClonedXml() const { return mClonedXml; }

	string GetTypeName();

	inline void SetXYZ(float x, float y, float z)
	{
		m_position = Vector3(x,y,z);
	}
	inline Vector3 Position() const
	{
		return m_position;
	}
	inline Vector3 LastPosition() const 
	{
		return m_lastPosition;
	}
	inline Vector3 Dimensions() const 
	{
		return m_dimensions;
	}
	inline void SetDimensionsXYZ(float x, float y, float z)
	{
		m_dimensions = Vector3(x,y,z);
	}
	inline float X() const 
	{
		return m_position.X;
	}
	inline float Y() const 
	{
		return m_position.Y;
	}
	inline float Z() const 
	{
		return m_position.Z;
	}
	inline void SetX(float value)
	{
		m_position.X = value;
	}
	inline void SetY(float value)
	{
		m_position.Y = value;
	}
	inline void SetZ(float value)
	{
		m_position.Z = value;
	}
	inline float Left() const 
	{
		return m_position.X - (m_dimensions.X * 0.5f);
	}
	inline float Right() const 
	{
		return m_position.X + (m_dimensions.X * 0.5f);
	}
	inline float Top() const 
	{
		return m_position.Y + (m_dimensions.Y * 0.5f);
	}
	inline float Bottom() const 
	{
		return m_position.Y - (m_dimensions.Y * 0.5f);
	}
	virtual void Initialise();
	virtual void Update(float delta);
	virtual void XmlRead(TiXmlElement * element);
	virtual void XmlWrite(TiXmlElement * element);
	virtual void DebugDraw(ID3D10Device *  device);
	virtual void OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode = true)
	{
	}
	inline Material* GetMaterial()
	{
		return m_material;
	}

	void SetMaterial(Material * material ) { m_material = material; }

	virtual void Scale(float xScale, float yScale, bool scalePosition = true);

	void SetRotationAngle(float radians);
	float GetRotationAngle() const { return m_rotationAngle; }

	void ShowDebugText(bool value) { mShowDebugText = value; }

};

#endif
