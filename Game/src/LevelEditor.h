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

private:

	void CheckInput_TerrainEditing();

	void CheckInput_Regular();

	void CheckForSavePressed();

	void CheckForZChange();

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

	GameObject * GetGameObjectClickedOn(list<shared_ptr<GameObject> > & gameObjects);

	Sprite * GetAsSprite(GameObject * object);

	MovingSprite * GetAsMovingSprite(GameObject * object);

	ParallaxLayer * GetAsParallaxLayer(GameObject * object);

	SolidMovingSprite * GetAsSolidMovingSprite(GameObject * object);

	ScrollingSprite * GetAsScrollingSprite(GameObject * object);

	SolidLineStrip * GetSolidLineStripClickedOn(list<shared_ptr<GameObject> > & gameObjects);

	Vector2 GetMouseWorldPos();

	void UpdateParallaxLayers();

	std::string mCurrentLevel;

	bool mTerrainEditing;

	GameObject * mSelectedObject;

	SolidLineStrip * mSelectedLineStrip;

	int mSelectedLinePointIndex;
};

#endif

