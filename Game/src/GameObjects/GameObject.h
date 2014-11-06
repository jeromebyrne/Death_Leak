#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

class Material;
class Camera2D;

class GameObject
{
public:
	typedef unique_ptr<GameObject> & GameObjectPtrRef;

	GameObject(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1);
	virtual ~GameObject(void);

	virtual void Draw(ID3D10Device * device, Camera2D * camera) { }
	virtual void LoadContent(ID3D10Device * graphicsdevice) { }

	bool IsUpdateable() const { return m_updateable; }

	inline int ID() const
	{
		return m_id;
	}

	TiXmlNode * GetClonedXml() const { return mClonedXml; }

	string GetTypeName();

	inline void SetXYZ(float x, float y, float z)
	{
		m_position = Vector3(x, y, z);
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
		m_dimensions = Vector3(x, y, z);
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
	virtual void OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) {}
	inline Material* GetMaterial()
	{
		return m_material;
	}

	void SetMaterial(Material * material) { m_material = material; }

	virtual void Scale(float xScale, float yScale, bool scalePosition = true);

	void SetRotationAngle(float radians);
	float GetRotationAngle() const { return m_rotationAngle; }

	void ShowDebugText(bool value) { mShowDebugText = value; }

	bool IsDrawable() const { return mDrawable; }

	bool IsSolidSprite() const { return mIsSolidSprite; }

	bool IsSolidLine() const { return mIsSolidLine; }

	bool IsButterfly() const { return mIsButterfly; }

	bool IsProjectile() const { return mIsProjectile; }

	bool IsOrb() const { return mIsOrb; }

	bool IsCharacter() const { return mIsCharacter; }

	bool IsParallaxLayer() const { return mIsParallaxLayer; }

	bool IsAudioObject() const { return mIsAudioObject; }

	bool IsWaterBlock() const { return mIsWaterBlock; }

	bool IsPlatform() const { return mIsPlatform; }

	bool IsSolidLineStrip() const { return mIsSolidLineStrip; }

	virtual void AttachTo(std::shared_ptr<GameObject> & parent, Vector3 offset, bool trackOrientation = true);
	void Detach();

	float GetParallaxMultiplierX() const { return mParallaxMultiplierX; }
	float GetParallaxMultiplierY() const { return mParallaxMultiplierY; }
	float GetCurrentParallaxOffsetX() const { return mCurrentParallaxOffsetX; }
	float GetCurrentParallaxOffsetY() const { return mCurrentParallaxOffsetY; }

	void SetParallaxMultiplierX(float value) { mParallaxMultiplierX = value; }
	void SetParallaxMultiplierY(float value) { mParallaxMultiplierY = value; }

	virtual void SetupDebugDraw();

	void SetMatrixScaleX(float value) { m_matScaleX = value; }
	void SetMatrixScaleY(float value) { m_matScaleY = value; }

	void SetAttachmentOffsetX(float x) { mAttachedToOffset.X = x; }

	bool IsLevelEditLocked() const { return mLevelEditLocked; }

	Vector2 GetLevelEditSelectionDimensions() const { return mLevelEditSelectionDimensions; }

	void SetLevelEditShowSelected(bool value) { mLevelEditShowSelected = value; }

	void SetXmlForCloning(TiXmlElement * element);

protected:

	virtual void DrawDebugText();

	Vector3 m_position;
	float m_rotationAngle;
	float m_matScaleX;
	float m_matScaleY;
	Vector3 m_lastPosition;
	Vector3 m_dimensions;
	bool m_updateable;
	D3DXMATRIX m_world; 
	Material * m_material;
	bool mShowDebugText;
	bool mDrawable;
	bool mIsSolidSprite;
	bool mIsSolidLine;
	bool mIsButterfly;
	bool mIsProjectile;
	bool mIsOrb;
	bool mIsCharacter;
	bool mIsParallaxLayer;
	bool mIsAudioObject;
	bool mIsWaterBlock;
	bool mIsPlatform;
	bool mIsSolidLineStrip;
	std::shared_ptr<GameObject> mAttachedTo;
	Vector3 mAttachedToOffset;
	float mParallaxMultiplierX;
	float mParallaxMultiplierY;
	float mCurrentParallaxOffsetX;
	float mCurrentParallaxOffsetY;
	bool mUpdateToParentsOrientation;
	float mAutoRotationValue;

private:

	virtual void UpdateToParent();

	int m_id; // our game object ID (this is unique)

	D3DXMATRIX m_translation;
	D3DXMATRIX m_rotation;
	D3DXMATRIX m_matScale;

	VertexPositionColor m_debugDrawVertices[8];
	ID3D10Buffer* m_debugDrawVBuffer;

	// used in the level editor for copying objects,
	// not valid in release mode
	TiXmlNode * mClonedXml;

	bool mLevelEditLocked;

	Vector2 mLevelEditSelectionDimensions;

	bool mLevelEditShowSelected;
};

#endif
