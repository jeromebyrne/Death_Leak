#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "SineWave.h"
#include "PositionalAudio.h"

class Material;
class Camera2D;

class GameObject
{
public:

	struct InteractableProperties
	{
		bool IsInteractable;
		Vector2 CurrentScreenPos;
		Vector2 PosOffset;
		float CurrentAlpha = 1.0f;
		float InteractTime = 0.3f;
		float InteractCountdown = InteractTime;
		float CurrentScale = 1.0f;

		InteractableProperties() :
			IsInteractable(false)
		{}
	};

	// if adding to this enum then make sure to update ConvertDepthLayerToString and ConvertStringToDepthLayer
	// also update GetParallaxMultipliersForDepthLayer
	enum DepthLayer
	{
		kFarBackground = 6000,
		kMiddleBackground = 5500,
		kNearBackground = 5000,
		kGroundBack = 4750,
		kGround = 4500,
		kGroundBlood = 4000,
		kNpc = 3500,
		kOrb = 3250,
		kPlayer = 3000,
		kGhostVomitProjectile = 2530,
		kBombProjectile = 2520,
		kNinjaStarProjectile = 2510,
		kPlayerProjectile = 2500,
		kImpactCircles = 2250,
		kBloodSpray1 = 2000,
		kGroundFront = 1750,
		kFarForeground = 1500,
		kMiddleForeground = 1000,
		kNearForeground = 500,
		kWeatherForeground = 20,
		kSolidLines = 10
	};

	typedef unique_ptr<GameObject> & GameObjectPtrRef;

	GameObject(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 1.0f, float height = 1.0f);
	virtual ~GameObject(void);

	virtual void Draw(ID3D10Device * device, Camera2D * camera) { }
	virtual void LoadContent(ID3D10Device * graphicsdevice) { }

	bool IsUpdateable() const { return m_updateable; }

	void SetUpdateable(bool value) { m_updateable = value; }

	static Vector2 GetParallaxMultipliersForDepthLayer(DepthLayer depthLayer);

	inline int ID() const
	{
		return m_id;
	}

	void SetID(int id);

	string GetTypeName();

	inline void SetXY(float x, float y)
	{
		m_position.X = x;
		m_position.Y = y;
	}
	inline Vector2 Position() const
	{
		return m_position;
	}
	inline Vector2 LastPosition() const
	{
		return m_lastPosition;
	}
	inline Vector2 Dimensions() const
	{
		return m_dimensions;
	}
	inline void SetDimensionsXY(float x, float y)
	{
		m_dimensions.X = x;
		m_dimensions.Y = y;
	}
	inline float X() const
	{
		return m_position.X;
	}
	inline float Y() const
	{
		return m_position.Y;
	}
	inline void SetX(float value)
	{
		m_position.X = value;
	}
	inline void SetY(float value)
	{
		m_position.Y = value;
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

	float GetRightRotated() const
	{
		return m_position.X + (mLargestPossibleDimensions.X * 0.5f);
	}
	float GetLeftRotated() const
	{
		return m_position.X - (mLargestPossibleDimensions.X * 0.5f);
	}
	float GetTopRotated() const
	{
		return m_position.Y + (mLargestPossibleDimensions.Y * 0.5f);
	}
	float GetBottomRotated() const
	{
		return m_position.Y - (mLargestPossibleDimensions.Y * 0.5f);
	}

	virtual void Initialise();
	virtual void Update(float delta);
	virtual void XmlRead(TiXmlElement * element);
	virtual void XmlWrite(TiXmlElement * element);
	virtual void DebugDraw(ID3D10Device *  device);
	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) {}
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

	bool IsDebris() const { return mIsDebris; }

	bool IsCurrencyOrb() const { return mIsCurrencyOrb; }

	bool IsCharacter() const { return mIsCharacter; }

	bool IsParallaxLayer() const { return mIsParallaxLayer; }

	bool IsAudioObject() const { return mIsAudioObject; }

	bool IsWaterBlock() const { return mIsWaterBlock; }

	bool IsPlatform() const { return mIsPlatform; }

	bool IsSolidLineStrip() const { return mIsSolidLineStrip; }

	bool IsBreakable() const { return mIsBreakable; }

	bool IsBombProjectile() const { return mIsBombProjectile; }

	bool IsSmashable() const { return  mIsSmashable; }

	bool IsPlayer() const { return mIsPlayer; }

	bool IsPickup() const { return mIsPickup; }

	virtual void AttachTo(std::shared_ptr<GameObject> & parent, Vector2 offset, DepthLayer depthLayer, bool trackOrientation = true);
	void Detach();

	float GetParallaxMultiplierX() const { return mParallaxMultiplier.X; }
	float GetParallaxMultiplierY() const { return mParallaxMultiplier.Y; }
	float GetCurrentParallaxOffsetX() const { return mCurrentParallaxOffsetX; }
	float GetCurrentParallaxOffsetY() const { return mCurrentParallaxOffsetY; }

	void SetParallaxMultiplier(Vector2 value) { mParallaxMultiplier = value; }

	virtual void SetupDebugDraw();

	void SetMatrixScaleX(float value) { m_matScaleX = value; }
	void SetMatrixScaleY(float value) { m_matScaleY = value; }

	void SetAttachmentOffsetX(float x) { mAttachedToOffset.X = x; }

	bool IsLevelEditLocked() const { return mLevelEditLocked; }

	Vector2 GetLevelEditSelectionDimensions() const { return mLevelEditSelectionDimensions; }

	void SetLevelEditShowSelected(bool value) { mLevelEditShowSelected = value; }

	// used for occlusion checks
	Vector2 GetLargestPossibleDimensions() const { return mLargestPossibleDimensions; }

	bool AlwaysUpdate() const { return mAlwaysUpdate; }

	void SetAlwaysUpdate(bool value) { mAlwaysUpdate = value; }

	static void ResetGameIds();

	static unsigned int GetCurrentGameObjectCount() { return sGameObjectId; }

	static void ForceIncrementGameObjectCount() { sGameObjectId++; }

	void SetDepthLayer(DepthLayer depthLayer);

	static string ConvertDepthLayerToString(DepthLayer depthLayer);

	static DepthLayer ConvertStringToDepthLayer(string depthLayerString);

	DepthLayer GetDepthLayer() const { return mDepthLayer; }

	virtual void PostUpdate(float delta) {  }

	void AttachToCamera(const Vector2 & offset);

	void DetachFromCamera();

	// Made public for XML special case read
	bool mPositionalAudioEnabled = false;
	PositionalAudio mPositionalAudio;
	float mPositionalAudioStartDelay = 0.1f;
	float mInitialPosAudioDelay = mPositionalAudioStartDelay;

protected:

	Vector2 m_position;
	Vector2 m_lastPosition;
	Vector2 m_dimensions;
	Vector2 mOriginalDimensions;

	virtual void DrawDebugText();

	virtual void UpdateToParent();

	void UpdateInteractable(float delta);
	virtual void OnInteracted() {}
	virtual bool CanInteract() { return true; }
	static int sCurrentInteractable;

	float m_rotationAngle;
	float m_matScaleX;
	float m_matScaleY;
	bool m_updateable;
	D3DXMATRIX m_world; 
	Material * m_material;
	bool mShowDebugText;
	bool mDrawable;
	bool mIsSolidSprite;
	bool mIsSolidLine;
	bool mIsButterfly;
	bool mIsProjectile;
	bool mIsDebris;
	bool mIsCurrencyOrb;
	bool mIsCharacter;
	bool mIsParallaxLayer;
	bool mIsAudioObject;
	bool mIsWaterBlock;
	bool mIsPlatform;
	bool mIsSolidLineStrip;
	bool mIsBreakable;
	bool mIsBombProjectile;
	bool mIsSmashable;
	bool mIsPlayer;
	bool mIsPickup;

	std::shared_ptr<GameObject> mAttachedTo;
	Vector2 mAttachedToOffset;
	bool mObjectParallaxEnabled = true;
	Vector2 mParallaxMultiplier;
	float mCurrentParallaxOffsetX;
	float mCurrentParallaxOffsetY;
	bool mUpdateToParentsOrientation;
	float mAutoRotationValue;

	struct SineWaveProps
	{
		bool DoSineWave;
		float OffsetY;
		float OffsetX;
		float Amplitude;
		float InitialYPosition;
		float InitialXPosition;
		bool RandomiseInitialStep;

		SineWaveProps():
			OffsetY(0.0f),
			Amplitude(0.0f),
			InitialYPosition(0.0f),
			DoSineWave(false),
			RandomiseInitialStep(false),
			OffsetX(0.0f),
			InitialXPosition(0.0f)
		{}
	};
	SineWaveProps mSineWaveProps;

	InteractableProperties mInteractableProperties;

	bool mAlwaysUpdate;

	float mAttachedToCamera = false;
	Vector2 mCameraAttachOffset;

private:

	int m_id; // our game object ID (this is unique)

	D3DXMATRIX m_translation;
	D3DXMATRIX m_rotation;
	D3DXMATRIX m_matScale;

	DepthLayer mDepthLayer = kGround;

	VertexPositionColor m_debugDrawVertices[8];
	ID3D10Buffer* m_debugDrawVBuffer;

	bool mLevelEditLocked;

	Vector2 mLevelEditSelectionDimensions;

	bool mLevelEditShowSelected;

	// when objects are rotated their bounding boxes will be larger
	Vector2 mLargestPossibleDimensions;

	static unsigned int sGameObjectId;

	SineWave mSinWave;

	bool mHasPlayedPositionalAudio = false;
	float mPositionalAudioCountdown = 0.0f;
};

#endif
