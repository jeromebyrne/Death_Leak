#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

class ParallaxLayer;

class LevelEditor
{
public:
	LevelEditor(void);
	virtual ~LevelEditor(void);

	void Update();

private:

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

	GameObject * GetGameObjectClickedOn(list<GameObject*> gameObjects);

	GameObject * mSelectedObject;

	Sprite * GetAsSprite(GameObject * object);

	MovingSprite * GetAsMovingSprite(GameObject * object);

	ParallaxLayer * GetAsParallaxLayer(GameObject * object);

	SolidMovingSprite * GetAsSolidMovingSprite(GameObject * object);

	void UpdateParallaxLayers();
};

#endif
