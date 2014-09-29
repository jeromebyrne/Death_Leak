#include "precompiled.h"
#include "LevelEditor.h"
#include "dxwindow.h"
#include "parallaxlayer.h"
#include "AudioObject.h"
#include "ScrollingSprite.h"
#include "Graphics.h"
#include "SolidLineStrip.h"

#if _DEBUG

LevelEditor::LevelEditor(void):
	mSelectedObject(nullptr),
	mTerrainEditing(false),
	mSelectedLineStrip(nullptr)
{
}


LevelEditor::~LevelEditor(void)
{
}

void LevelEditor::Update()
{
	static bool isPressingTerrainEdit = false;

	if (!isPressingTerrainEdit && GetAsyncKeyState('T') < 0)
	{
		isPressingTerrainEdit = true;

		if (mTerrainEditing)
		{
			mTerrainEditing = false;
		}
		else
		{
			mTerrainEditing = true;
		}
	}
	if (GetAsyncKeyState('T') >= 0)
	{
		isPressingTerrainEdit = false;
	}

	UpdateParallaxLayers();

	if (mTerrainEditing)
	{
		CheckInput_TerrainEditing();
	}
	else
	{
		CheckInput_Regular();
	}

	list<shared_ptr<GameObject> > & gameObjects = GameObjectManager::Instance()->GetGameObjectList();
	for (auto & obj : gameObjects)
	{
		if (obj->GetParallaxMultiplierX() != 1.0f ||
			obj->GetParallaxMultiplierY() != 1.0f)
		{
			obj->Update(1.0f);
		}
	}
}

GameObject * LevelEditor::GetGameObjectClickedOn(list<shared_ptr<GameObject> > & gameObjects)
{
	Vector2 worldPos = GetMouseWorldPos();

	GameObject * selectedObj = nullptr;
	for (auto & obj : gameObjects)
	{
		if (obj->IsLevelEditLocked())
		{
			continue;
		}

		ParallaxLayer * paraLayer = dynamic_cast<ParallaxLayer*>(obj.get());
		if (dynamic_cast<ParticleSpray*>(obj.get()) || (paraLayer && paraLayer->FollowCamX()))
		{
			continue;
		}

		float left = obj->Position().X - (obj->GetLevelEditSelectionDimensions().X * 0.5f);
		float right = obj->Position().X + (obj->GetLevelEditSelectionDimensions().X * 0.5f);
		float top = obj->Position().Y + (obj->GetLevelEditSelectionDimensions().Y * 0.5f);
		float bottom = obj->Position().Y - (obj->GetLevelEditSelectionDimensions().Y * 0.5f);

		if (worldPos.X > left &&
			worldPos.X < right &&
			worldPos.Y > bottom &&
			worldPos.Y < top)
		{
			if (selectedObj)
			{
				// check if the area of this object is smaller,
				// if it is then we pick this instead
				GameObject * newObj = obj.get();
				float newArea = (newObj->Right() - newObj->Left()) * (newObj->Top() - newObj->Bottom());
				float selectedObjArea = (selectedObj->Right() - selectedObj->Left()) * (selectedObj->Top() - selectedObj->Bottom());

				if (newArea < selectedObjArea)
				{
					selectedObj = newObj;
				}
			}
			else 
			{
				selectedObj = obj.get();
			}
		}
	}

	return selectedObj;
}

Sprite * LevelEditor::GetAsSprite(GameObject * object)
{
	Sprite * sprite = dynamic_cast<Sprite*>(object);

	return sprite;
}

void LevelEditor::UpdateParallaxLayers()
{
	list<shared_ptr<GameObject> > gameObjects = GameObjectManager::Instance()->GetGameObjectList();

	for (auto & obj : gameObjects)
	{
		ParallaxLayer * pLayer = GetAsParallaxLayer(obj.get());

		if (pLayer)
		{
			pLayer->Update(0);
		}
	}
}

ParallaxLayer * LevelEditor::GetAsParallaxLayer(GameObject * object)
{
	ParallaxLayer * pLayer = dynamic_cast<ParallaxLayer*>(object);

	return pLayer;
}

MovingSprite * LevelEditor::GetAsMovingSprite(GameObject * object)
{
	MovingSprite * msprite = dynamic_cast<MovingSprite*>(object);

	return msprite;
}

SolidMovingSprite * LevelEditor::GetAsSolidMovingSprite(GameObject * object)
{
	SolidMovingSprite * sms = dynamic_cast<SolidMovingSprite*>(object);

	return sms;
}

ScrollingSprite * LevelEditor::GetAsScrollingSprite(GameObject * object)
{
	ScrollingSprite * ss = dynamic_cast<ScrollingSprite*>(object);

	return ss;
}

void LevelEditor::CheckForSavePressed()
{
	if (GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState('S') < 0)
	{
		GameObjectManager::Instance()->SaveObjectsToFile(mCurrentLevel.c_str());
	}
}

void LevelEditor::CheckForZChange()
{
	if (mSelectedObject)
	{
		static bool isPressingZChange = false;

		if (!isPressingZChange && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_LMENU ) < 0 && GetAsyncKeyState(VK_UP))
		{
			mSelectedObject->SetZ(mSelectedObject->Z() + 0.1f);
			GameObjectManager::Instance()->OrderDrawablesByDepth();
			isPressingZChange = true;
		}
		else if (!isPressingZChange && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_UP))
		{
			mSelectedObject->SetZ(mSelectedObject->Z() + 1);
			GameObjectManager::Instance()->OrderDrawablesByDepth();
			isPressingZChange = true;
		}

		if (!isPressingZChange && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_LMENU ) < 0 && GetAsyncKeyState(VK_DOWN))
		{
			mSelectedObject->SetZ(mSelectedObject->Z() - 0.1f);
			GameObjectManager::Instance()->OrderDrawablesByDepth();
			isPressingZChange = true;
		}
		else if (!isPressingZChange && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_DOWN))
		{
			mSelectedObject->SetZ(mSelectedObject->Z() - 1);
			GameObjectManager::Instance()->OrderDrawablesByDepth();
			isPressingZChange = true;
		}

		if (GetAsyncKeyState(VK_UP) >= 0 && GetAsyncKeyState(VK_DOWN) >= 0)
		{
			isPressingZChange = false;
		}
	}
}

void LevelEditor::CheckForDeleting()
{
	if (mSelectedObject)
	{
		static bool isPressingDelete = false;

		if (!isPressingDelete && GetAsyncKeyState(VK_DELETE) < 0)
		{
			GameObjectManager::Instance()->RemoveGameObject(mSelectedObject, false);
			mSelectedObject = nullptr;
			isPressingDelete = true;
		}

		if (GetAsyncKeyState(VK_DELETE) >= 0)
		{
			isPressingDelete = false;
		}
	}
}

void LevelEditor::CheckForCopy()
{
	if (mSelectedObject)
	{
		static bool isPressingCopy = false;

		if (!isPressingCopy && GetAsyncKeyState('K') < 0 && mSelectedObject != GameObjectManager::Instance()->GetPlayer())
		{
			GameObject * newObject = GameObjectManager::Instance()->CopyObject(mSelectedObject);
			if (newObject)
			{
				DrawableObject * drawable = dynamic_cast<DrawableObject*>(newObject);
				AudioObject * audiobj = dynamic_cast<AudioObject*>(newObject);
				if (drawable)
				{
					GameObjectManager::Instance()->AddGameObject(drawable, true);
				}
				else if (audiobj)
				{
					GameObjectManager::Instance()->AddGameObject(audiobj, true);
				}
				else 
				{
					throw new exception();
				}
				
				newObject->SetX(mSelectedObject->X() + 50);
				newObject->SetY(mSelectedObject->Y() + 50);

				mSelectedObject->ShowDebugText(false);
				mSelectedObject->SetLevelEditShowSelected(false);
				newObject->ShowDebugText(true);
				newObject->SetLevelEditShowSelected(true);

				mSelectedObject = newObject;

				newObject->Update(0);
			}
			else
			{
				mSelectedObject = nullptr;
			}
			isPressingCopy = true;
		}

		if (GetAsyncKeyState('K') >= 0)
		{
			isPressingCopy = false;
		}
	}
}

void LevelEditor::CheckForSpriteScaling()
{
	if (mSelectedObject)
	{
		static bool pressingScale = false;
		static Vector2 mouseStartPos = Vector2(0,0);
		static Vector2 originalDimensions = Vector2(0,0);
		if (!pressingScale && GetAsyncKeyState('X') < 0 && GetAsyncKeyState(VK_LBUTTON) < 0 && GetAsyncKeyState(VK_CONTROL) >= 0)
		{
			pressingScale = true;

			POINT currentMouse;
			GetCursorPos(&currentMouse);
			ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);
			mouseStartPos = Vector2(currentMouse.x, currentMouse.y);

			originalDimensions = Vector2(mSelectedObject->Dimensions().X, mSelectedObject->Dimensions().Y);
		}
		else if (pressingScale && GetAsyncKeyState(VK_LBUTTON) < 0)
		{
			// get the initial distance between the game object and the mouse
			Vector2 origDistance = mouseStartPos - Vector2(mSelectedObject->X(), mSelectedObject->Y());
			// origDistance = Vector2(abs(origDistance.X), abs(origDistance.Y));

			float origLength = origDistance.Length();

			// now get the distance between the current mouse position
			POINT currentMouse;
			GetCursorPos(&currentMouse);
			ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);
			Vector2 newDistance = Vector2(currentMouse.x, currentMouse.y) - Vector2(mSelectedObject->X(), mSelectedObject->Y());

			float newLength = newDistance.Length();

			float scale = newLength / origLength;

			if (scale != 0)
			{
				mSelectedObject->SetDimensionsXYZ(originalDimensions.X * scale, originalDimensions.Y * scale, mSelectedObject->Dimensions().Z);

				Sprite * s = GetAsSprite(mSelectedObject);
				if (s)
				{
					s->ScaleSpriteOnly(scale, scale);
					s->SetIsNativeDimensions(false);
					s->ApplyChange(Graphics::GetInstance()->Device());
				}
			}
		}

		if (GetAsyncKeyState('X') >= 0)
		{
			pressingScale = false;
			mouseStartPos = Vector2(0,0);
			originalDimensions = Vector2(0,0);
		}
	}
}

void LevelEditor::CheckFlippingSprite()
{
	if (mSelectedObject)
	{
		Sprite * sprite = GetAsSprite(mSelectedObject);

		if (sprite)
		{
			// h flip
			static bool pressingHFlip = false;

			if (!pressingHFlip && GetAsyncKeyState('H') < 0)
			{
				pressingHFlip = true;

				if (sprite->IsHFlipped())
				{
					sprite->UnFlipHorizontal();
				}
				else
				{
					sprite->FlipHorizontal();
				}
				sprite->Update(0);
			}

			if (GetAsyncKeyState('H') >= 0)
			{
				pressingHFlip = false;
			}

			// v flip

			static bool pressingVFlip = false;

			if (!pressingVFlip && GetAsyncKeyState('V') < 0)
			{
				pressingVFlip = true;

				if (sprite->IsVFlipped())
				{
					sprite->UnFlipVertical();
				}
				else
				{
					sprite->FlipVertical();
				}
				sprite->Update(0);
			}

			if (GetAsyncKeyState('V') >= 0)
			{
				pressingVFlip = false;
			}
		}
	}
}

void LevelEditor::CheckForCollisionBoxScaling()
{
	if (mSelectedObject)
	{
		SolidMovingSprite * solidSprite = GetAsSolidMovingSprite(mSelectedObject);

		if (solidSprite)
		{
			static bool pressingCollisionScale = false;
			static Vector2 mouseStartPos = Vector2(0,0);
			static Vector2 originalDimensions = Vector2(0,0);
			if (!pressingCollisionScale && GetAsyncKeyState('C') < 0 && GetAsyncKeyState(VK_LBUTTON) < 0)
			{
				pressingCollisionScale = true;

				POINT currentMouse;
				GetCursorPos(&currentMouse);
				ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);
				mouseStartPos = Vector2(currentMouse.x, currentMouse.y);

				originalDimensions = Vector2(solidSprite->CollisionDimensions().X, solidSprite->CollisionDimensions().Y);
			}
			else if (pressingCollisionScale && GetAsyncKeyState(VK_LBUTTON) < 0)
			{
				// get the initial distance between the game object and the mouse
				Vector2 origDistance = mouseStartPos - Vector2(mSelectedObject->X(), mSelectedObject->Y());

				float origLength = origDistance.Length();

				// now get the distance between the current mouse position
				POINT currentMouse;
				GetCursorPos(&currentMouse);
				ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);
				Vector2 newDistance = Vector2(currentMouse.x, currentMouse.y) - Vector2(mSelectedObject->X(), mSelectedObject->Y());

				float newLength = newDistance.Length();

				float scale = newLength / origLength;

				if (scale != 0)
				{
					solidSprite->SetCollisionDimensions(Vector3(originalDimensions.X * scale, originalDimensions.Y * scale, solidSprite->CollisionDimensions().Z));
					solidSprite->RecalculateVertices();
					solidSprite->ApplyChange(Graphics::GetInstance()->Device());
				}
			}

			if (GetAsyncKeyState('C') >= 0)
			{
				pressingCollisionScale = false;
				mouseStartPos = Vector2(0,0);
				originalDimensions = Vector2(0,0);
			}
		}
	}
}

void LevelEditor::CheckForSettingNativeRes()
{
	if (mSelectedObject)
	{
		if (GetAsyncKeyState('N') < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			Sprite * s = GetAsSprite(mSelectedObject);

			if (s)
			{
				s->SetIsNativeDimensions(true);
				s->ScaleSpriteOnly(1,1); // resets vertices and debug bounding box
				s->ApplyChange(Graphics::GetInstance()->Device());
			}
		}
	}
}

void LevelEditor::CheckForRotating()
{
	if (mSelectedObject)
	{
		static bool pressingRotation = false;
		static Vector2 mouseStartPos = Vector2(0,0);

		if (!pressingRotation && GetAsyncKeyState('R') < 0 && (GetAsyncKeyState(VK_LBUTTON) < 0 || GetAsyncKeyState(VK_RBUTTON) < 0))
		{
			pressingRotation = true;

				POINT currentMouse;
				GetCursorPos(&currentMouse);
				ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);
				mouseStartPos = Vector2(currentMouse.x, currentMouse.y);
		}
		else if (pressingRotation && GetAsyncKeyState(VK_LBUTTON) < 0 ||
			     pressingRotation && GetAsyncKeyState(VK_RBUTTON) < 0)
		{
			// get the initial distance between the game object and the mouse
			Vector2 origDistance = mouseStartPos - Vector2(mSelectedObject->X(), mSelectedObject->Y());

			float origLength = origDistance.Length();

			// now get the distance between the current mouse position
			POINT currentMouse;
			GetCursorPos(&currentMouse);
			ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);
			Vector2 newDistance = Vector2(currentMouse.x, currentMouse.y) - Vector2(mSelectedObject->X(), mSelectedObject->Y());

			float newLength = newDistance.Length();
			float scale = newLength / origLength;

			if (pressingRotation && GetAsyncKeyState(VK_RBUTTON) < 0)
			{
				scale *= -1;
			}

			if (scale != 0)
			{
				mSelectedObject->SetRotationAngle(mSelectedObject->GetRotationAngle() + (scale * 0.01));
				mSelectedObject->Update(0);
			}
		}

		if (GetAsyncKeyState('R') >= 0)
		{
			pressingRotation = false;
			mouseStartPos = Vector2(0,0);
		}
		else if (GetAsyncKeyState('R') < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			// reset rotation to 0
			mSelectedObject->SetRotationAngle(0);
			mSelectedObject->Update(0);
		}
	}
}

void LevelEditor::CheckInput_TerrainEditing()
{
	list<shared_ptr<GameObject> > & gameObjects = GameObjectManager::Instance()->GetGameObjectList();

	static bool pressingSelect = false;

	if (mSelectedLineStrip)
	{
		if (!pressingSelect && GetAsyncKeyState(VK_RBUTTON) < 0)
		{
			SolidLineStrip * obj = GetSolidLineStripClickedOn(gameObjects);

			if (obj)
			{
				if (obj == mSelectedLineStrip)
				{
					mSelectedLineStrip->ShowDebugText(false);
					mSelectedLineStrip->SetLevelEditShowSelected(false);
					mSelectedLineStrip = nullptr;
				}
				else
				{
					mSelectedLineStrip->ShowDebugText(false);
					mSelectedLineStrip->SetLevelEditShowSelected(false);
					mSelectedLineStrip = obj;
					mSelectedLineStrip->ShowDebugText(true);
					mSelectedLineStrip->SetLevelEditShowSelected(true);
				}

				pressingSelect = false;
			}
		}
	}
	else
	{
		if (!pressingSelect && GetAsyncKeyState(VK_RBUTTON) < 0)
		{
			mSelectedLineStrip = GetSolidLineStripClickedOn(gameObjects);

			if (mSelectedLineStrip)
			{
				mSelectedLineStrip->ShowDebugText(true);
				mSelectedLineStrip->SetLevelEditShowSelected(true);
			}

			pressingSelect = true;
		}
	}

	if (GetAsyncKeyState(VK_RBUTTON) >= 0)
	{
		pressingSelect = false;
	}

	CheckForSavePressed();
}

void LevelEditor::CheckInput_Regular()
{
	list<shared_ptr<GameObject> > & gameObjects = GameObjectManager::Instance()->GetGameObjectList();

	static bool pressingSelect = false;
	static bool pressingPlace = false;

	if (mSelectedObject)
	{
		if (!pressingSelect && GetAsyncKeyState(VK_RBUTTON) < 0 && GetAsyncKeyState('R') >= 0)
		{
			GameObject * obj = GetGameObjectClickedOn(gameObjects);

			if (obj)
			{
				if (obj == mSelectedObject)
				{
					// deselect
					mSelectedObject->ShowDebugText(false);
					mSelectedObject->SetLevelEditShowSelected(false);
					mSelectedObject = nullptr;
				}
				else
				{
					mSelectedObject->ShowDebugText(false);
					mSelectedObject->SetLevelEditShowSelected(false);
					mSelectedObject = obj;
					mSelectedObject->ShowDebugText(true);
					mSelectedObject->SetLevelEditShowSelected(true);
				}
			}

			pressingSelect = true;
		}

		if (mSelectedObject)
		{
			if (GetAsyncKeyState(VK_LBUTTON) < 0 && GetAsyncKeyState('X') >= 0 && GetAsyncKeyState('C') >= 0 && GetAsyncKeyState('R') >= 0)
			{
				// drag
				// get the mouse in world coordinates and set the game objects position
				POINT currentMouse;
				GetCursorPos(&currentMouse);
				ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);

				// the backbuffer may be larger in size than the the window (Windows scaling) so scale accordingly
				float scaleX = Graphics::GetInstance()->BackBufferWidth() / DXWindow::GetInstance()->GetWindowDimensions().X;
				float scaleY = Graphics::GetInstance()->BackBufferHeight() / DXWindow::GetInstance()->GetWindowDimensions().Y;

				Vector2 worldPos = Utilities::ScreenToWorld(Vector2(currentMouse.x * scaleX, currentMouse.y * scaleY));

				mSelectedObject->SetX(worldPos.X);
				mSelectedObject->SetY(worldPos.Y);

				Sprite * sprite = GetAsSprite(mSelectedObject);
				if (sprite)
				{
					sprite->ApplyChange(Graphics::GetInstance()->Device());
					sprite->Update(0);
				}
				MovingSprite * movingSprite = GetAsMovingSprite(mSelectedObject);
				if (movingSprite)
				{
					movingSprite->SetVelocityXYZ(0, 0, 0);
				}

				ScrollingSprite * scrollingSprite = GetAsScrollingSprite(mSelectedObject);
				if (scrollingSprite)
				{
					scrollingSprite->SetLerpStartPos(worldPos);
				}

				pressingPlace = true;
			}
		}
	}
	else
	{
		if (!pressingSelect && GetAsyncKeyState(VK_RBUTTON) < 0 && GetAsyncKeyState('R') >= 0)
		{
			mSelectedObject = GetGameObjectClickedOn(gameObjects);

			if (mSelectedObject)
			{
				mSelectedObject->ShowDebugText(true);
				mSelectedObject->SetLevelEditShowSelected(true);
			}

			pressingSelect = true;
		}
	}

	if (GetAsyncKeyState(VK_RBUTTON) >= 0)
	{
		pressingSelect = false;
	}
	if (GetAsyncKeyState(VK_LBUTTON) >= 0)
	{
		pressingPlace = false;
	}

	CheckForZChange();

	CheckForSpriteScaling();

	CheckForCollisionBoxScaling();

	CheckFlippingSprite();

	CheckForSettingNativeRes();

	CheckForRotating();

	CheckForDeleting();

	CheckForSavePressed();

	CheckForCopy();
}

void LevelEditor::Draw()
{
	if (mTerrainEditing)
	{
		Graphics::GetInstance()->DrawDebugText("Terrain Edit Mode", 100, 100);
	}

	Vector2 mousePos = GetMouseWorldPos();

	Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Mouse X,Y: %f %f", mousePos.X, mousePos.Y).c_str(), 100, 150);
}

SolidLineStrip * LevelEditor::GetSolidLineStripClickedOn(list<shared_ptr<GameObject> > & gameObjects)
{
	Vector2 worldPosClicked = GetMouseWorldPos();

	for (auto & g : gameObjects)
	{
		SolidLineStrip * solidLineStrip = dynamic_cast<SolidLineStrip*>(g.get());

		if (!solidLineStrip)
		{
			continue;
		}

		if (solidLineStrip->IsLevelEditLocked())
		{
			continue;
		}
		
		float left = solidLineStrip->Position().X - (solidLineStrip->Dimensions().X * 0.5f);
		float right = solidLineStrip->Position().X + (solidLineStrip->Dimensions().X * 0.5f);
		float top = solidLineStrip->Position().Y + (solidLineStrip->Dimensions().Y * 0.5f);
		float bottom = solidLineStrip->Position().Y - (solidLineStrip->Dimensions().Y * 0.5f);

		if (!(worldPosClicked.X > left &&
			worldPosClicked.X < right &&
			worldPosClicked.Y > bottom &&
			worldPosClicked.Y < top))
		{
			continue;
		}

		const vector<SolidLineStrip::SolidLinePoint> points = solidLineStrip->GetLinePoints();
		for (auto & p : points)
		{
			float pointLeft = solidLineStrip->Position().X - 50;
			float pointRight = solidLineStrip->Position().X + 50;
			float pointTop = solidLineStrip->Position().Y + 50;
			float pointBottom = solidLineStrip->Position().Y - 50;

			if (!(worldPosClicked.X > pointLeft &&
				worldPosClicked.X < pointRight &&
				worldPosClicked.Y > pointBottom &&
				worldPosClicked.Y < pointTop))
			{
				continue;
			}

			LOG_INFO("Clicking on a solid line point");
		}
	}

	return nullptr;
}

Vector2 LevelEditor::GetMouseWorldPos()
{
	POINT currentMouse;
	GetCursorPos(&currentMouse);
	ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);

	// the backbuffer may be larger in size than the the window (Windows scaling) so scale accordingly
	float scaleX = Graphics::GetInstance()->BackBufferWidth() / DXWindow::GetInstance()->GetWindowDimensions().X;
	float scaleY = Graphics::GetInstance()->BackBufferHeight() / DXWindow::GetInstance()->GetWindowDimensions().Y;

	Vector2 worldPos = Utilities::ScreenToWorld(Vector2(currentMouse.x * scaleX, currentMouse.y * scaleY));

	return worldPos;
}

#endif
