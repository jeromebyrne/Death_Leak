#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

class ParallaxLayer;
class ScrollingSprite;
class SolidLineStrip;

class LevelEditor
{
public:
	LevelEditor(void);
	virtual ~LevelEditor(void);

	void Update();

	void Draw();

	void SetLevelFile(const char * level) { mCurrentLevel = level; }

	bool IsTerrainEditing() const { return mTerrainEditing; }

	void Reset();

	void ResetSelectedObject() 
	{ 
		mSelectedObject = nullptr;
		mSelectedLineStrip = nullptr;
		mSelectedLinePointIndex = -1;
	}

private:

	void UpdateAnimationPreview();

	void CheckInput_TerrainEditing();

	void CheckInput_Regular();

	void CheckInput_AnimationPreview();

	void CheckForSavePressed();

	void CheckForDepthChange();

	void CheckForSpriteScaling();

	void CheckForCollisionBoxScaling();

	void CheckFlippingSprite();

	// check if we are making a sprite display at native res
	void CheckForSettingNativeRes();

	void CheckForRotating();

	void CheckForDeleting();

	void CheckForCopy();

	void CheckForTerrainPointSelect();

	void CheckForTerrainPointMove();

	void CheckForTerrainNewPoint();

	void CheckForTerrainPointDelete();

	void CheckForNewTerrainObject();

	void CheckForLayerAssign();

	void CheckForMaterialAssign();

	void CheckForSolidLineStripEdgeAssign();

	void CheckForSolidLineSetDropDown();

	void CheckForPixelMovement();

	void CheckForInvokeObjectEditor();

	void CheckForCameraZoom();

	GameObject * GetGameObjectClickedOn(list<shared_ptr<GameObject> > & gameObjects);

	Sprite * GetAsSprite(GameObject * object);

	MovingSprite * GetAsMovingSprite(GameObject * object);

	ParallaxLayer * GetAsParallaxLayer(GameObject * object);

	SolidMovingSprite * GetAsSolidMovingSprite(GameObject * object);

	ScrollingSprite * GetAsScrollingSprite(GameObject * object);

	SolidLineStrip * GetAsSolidLineStrip(GameObject * object);

	ParticleSpray * GetAsParticleSpray(GameObject * object);

	SolidLineStrip * GetSolidLineStripClickedOn(list<shared_ptr<GameObject> > & gameObjects);

	Vector2 GetMouseWorldPos();

	void UpdateParallaxLayers();

	std::string mCurrentLevel;

	bool mTerrainEditing;

	GameObject * mSelectedObject;

	SolidLineStrip * mSelectedLineStrip;

	int mSelectedLinePointIndex;

	Sprite * mAnimationPreviewTargetObject = nullptr;

	float mOriginalCameraZoom = 1.0f;
};

#endif

