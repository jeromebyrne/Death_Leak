#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

class ParallaxLayer;
class ScrollingSprite;

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

	GameObject * GetGameObjectClickedOn(list<shared_ptr<GameObject> > & gameObjects);

	GameObject * mSelectedObject;

	Sprite * GetAsSprite(GameObject * object);

	MovingSprite * GetAsMovingSprite(GameObject * object);

	ParallaxLayer * GetAsParallaxLayer(GameObject * object);

	SolidMovingSprite * GetAsSolidMovingSprite(GameObject * object);

	ScrollingSprite * GetAsScrollingSprite(GameObject * object);

	void UpdateParallaxLayers();

	std::string mCurrentLevel;

	bool mTerrainEditing;
};

#endif

