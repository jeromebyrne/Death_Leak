#include "precompiled.h"
#include "LevelEditor.h"
#include "dxwindow.h"
#include "parallaxlayer.h"
#include "AudioObject.h"
#include "ScrollingSprite.h"
#include "Graphics.h"
#include "SolidLineStrip.h"
#include "DrawUtilities.h"
#include "MaterialManager.h"
#include "NPCTrigger.h"
#include "UIManager.h"

LevelEditor::LevelEditor(void):
	mSelectedObject(nullptr),
	mTerrainEditing(false),
	mSelectedLineStrip(nullptr),
	mSelectedLinePointIndex(-1)
{
}

LevelEditor::~LevelEditor(void)
{
}

void LevelEditor::Reset()
{
	mTerrainEditing = false;

	mSelectedObject = nullptr;

	mSelectedLineStrip = nullptr;

	mSelectedLinePointIndex = -1;
}

void LevelEditor::UpdateAnimationPreview()
{
	if (mAnimationPreviewTargetObject == nullptr)
	{
		// grab the target animation - TODO: can make this configurable
		auto object = GameObjectManager::Instance()->GetObjectByID(1);

		if (object == nullptr)
		{
			return;
		}

		mAnimationPreviewTargetObject = dynamic_cast<Sprite*>(object.get());

		if (mAnimationPreviewTargetObject == nullptr)
		{
			return;
		}
	}

	Animation * anim = mAnimationPreviewTargetObject->GetAnimation();

	if (anim == nullptr)
	{
		return;
	}

	//anim->GetSkeletonForCurrentSequence
}

void LevelEditor::Update()
{ 
	if (GameObjectManager::Instance()->GetCurrentLevelProperties().IsAnimationPreview())
	{
		UpdateAnimationPreview();
		CheckInput_AnimationPreview();
		return;
	}

	CheckForInvokeObjectEditor();

	if (UIManager::Instance()->IsObjectEditorDisplaying())
	{
		return;
	}

	static bool isPressingTerrainEdit = false;

	if (!isPressingTerrainEdit && GetAsyncKeyState('T') < 0)
	{
		isPressingTerrainEdit = true;

		if (mTerrainEditing)
		{
			mTerrainEditing = false;

			if (mSelectedLineStrip)
			{
				mSelectedLineStrip->ShowDebugText(false);
				mSelectedLineStrip->SetLevelEditShowSelected(false);
				mSelectedLineStrip = nullptr;
				mSelectedLinePointIndex = -1;
			}
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
		MovingSprite * mo = GetAsMovingSprite(obj.get());
		if (mo)
		{
			mo->SetVelocityXYZ(0, 0, 0);
			mo->StopYAccelerating();
			mo->StopXAccelerating();
		}
	}
}

void LevelEditor::CheckInput_AnimationPreview()
{
	if (!mAnimationPreviewTargetObject)
	{
		return;
	}

	// *** PRESSING RIGHT
	static bool isPressingRight = false;

	if (!isPressingRight && GetAsyncKeyState(VK_RIGHT) < 0)
	{
		isPressingRight = true;

		auto anim = mAnimationPreviewTargetObject->GetAnimation();
		if (anim)
		{
			anim->JumpToNextFrame("body");
			mAnimationPreviewTargetObject->UpdateAnimTexture("body");
		}
	}

	if (GetAsyncKeyState(VK_RIGHT) >= 0)
	{
		isPressingRight = false;
	}

	// *** PRESSING LEFT
	static bool isPressingLeft = false;

	if (!isPressingLeft && GetAsyncKeyState(VK_LEFT) < 0)
	{
		isPressingLeft = true;

		auto anim = mAnimationPreviewTargetObject->GetAnimation();
		if (anim)
		{
			anim->JumpToPreviousFrame("body");
			mAnimationPreviewTargetObject->UpdateAnimTexture("body");
		}
	}

	if (GetAsyncKeyState(VK_LEFT) >= 0)
	{
		isPressingLeft = false;
	}

	// *** PRESSING UP
	static bool isPressingUp = false;

	if (!isPressingUp && GetAsyncKeyState(VK_UP) < 0)
	{
		isPressingUp = true;

		auto anim = mAnimationPreviewTargetObject->GetAnimation();
		if (anim)
		{
			anim->JumpToNextSequence("body");
			mAnimationPreviewTargetObject->UpdateAnimTexture("body");
		}
	}

	if (GetAsyncKeyState(VK_UP) >= 0)
	{
		isPressingUp = false;
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
		if (paraLayer && paraLayer->FollowCamX())
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
				GameObjectManager::Instance()->AddGameObjectViaLevelEditor(newObject);
				
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

void LevelEditor::CheckForNewTerrainObject()
{
	static bool pressingNew = false;

	if (!pressingNew && GetAsyncKeyState('N') < 0)
	{
		pressingNew = true;

		Vector3 cameraPos = Camera2D::GetInstance()->Position();

		SolidLineStrip * solidLineStrip = new SolidLineStrip();

		solidLineStrip->SetXYZ(cameraPos.X, cameraPos.Y, 3);
		
		SolidLineStrip::SolidLinePoint startPoint;
		SolidLineStrip::SolidLinePoint endPoint;

		startPoint.WorldPosition = Vector2(solidLineStrip->Position().X, solidLineStrip->Position().Y);
		startPoint.LocalPosition = Vector2(0, 0);

		endPoint.WorldPosition = Vector2(solidLineStrip->Position().X + 200, solidLineStrip->Position().Y + 100);
		endPoint.LocalPosition = Vector2(0, 200);

		std::vector<SolidLineStrip::SolidLinePoint> pointVec;
		pointVec.reserve(2);

		pointVec.push_back(startPoint);
		pointVec.push_back(endPoint);

		solidLineStrip->RecalculateLines(pointVec);

		GameObjectManager::Instance()->AddGameObject(solidLineStrip, true);
	}

	if (GetAsyncKeyState('N') >= 0)
	{
		pressingNew = false;
	}
}

void LevelEditor::CheckForTerrainPointSelect()
{
	list<shared_ptr<GameObject> > & gameObjects = GameObjectManager::Instance()->GetGameObjectList();

	static bool pressingSelect = false;

	if (mSelectedLineStrip)
	{
		if (!pressingSelect && GetAsyncKeyState(VK_RBUTTON) < 0)
		{
			mSelectedLineStrip = GetSolidLineStripClickedOn(gameObjects);

			if (mSelectedLineStrip)
			{
				mSelectedLineStrip->ShowDebugText(true);
				mSelectedLineStrip->SetLevelEditShowSelected(true);
			}
			else
			{
				mSelectedLineStrip = nullptr;
				mSelectedLinePointIndex = -1;
			}

			pressingSelect = true;
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
}

void LevelEditor::CheckForTerrainPointMove()
{
	if (mSelectedLineStrip && mSelectedLinePointIndex > -1)
	{
		if (GetAsyncKeyState(VK_LBUTTON) >= 0)
		{
			return;
		}

		Vector2 mousePos = GetMouseWorldPos();

		std::vector<SolidLineStrip::SolidLinePoint> points = mSelectedLineStrip->GetLinePoints();

		GAME_ASSERT(points.size() > 0 && mSelectedLinePointIndex < points.size());

		points[mSelectedLinePointIndex].WorldPosition = mousePos;

		Vector2 LocalPos = Vector2(mousePos.X - mSelectedLineStrip->Position().X, mousePos.Y - mSelectedLineStrip->Position().Y);

		points[mSelectedLinePointIndex].LocalPosition = LocalPos;

		mSelectedLineStrip->RecalculateLines(points);
	}
}

void LevelEditor::CheckForTerrainNewPoint()
{
	static bool pressingNewPoint = false;

	if (mSelectedLineStrip && mSelectedLinePointIndex > -1)
	{
		if (!pressingNewPoint && GetAsyncKeyState('P') < 0)
		{
			pressingNewPoint = true;

			SolidLineStrip::SolidLinePoint newPoint;

			std::vector<SolidLineStrip::SolidLinePoint> points = mSelectedLineStrip->GetLinePoints();

			GAME_ASSERT(points.size() > 0 && mSelectedLinePointIndex < points.size());

			newPoint.WorldPosition = Vector2(points[mSelectedLinePointIndex].WorldPosition.X, points[mSelectedLinePointIndex].WorldPosition.Y + 150.0f);
			newPoint.LocalPosition = Vector2(points[mSelectedLinePointIndex].LocalPosition.X, points[mSelectedLinePointIndex].LocalPosition.Y + 150.0f);

			points.insert(points.begin() + (mSelectedLinePointIndex + 1), newPoint);
			++mSelectedLinePointIndex;

			mSelectedLineStrip->RecalculateLines(points);
		}
	}

	if (GetAsyncKeyState('P') >= 0)
	{
		pressingNewPoint = false;
	}
}

void LevelEditor::CheckForTerrainPointDelete()
{
	static bool pressingDelete = false;

	if (mSelectedLineStrip && mSelectedLinePointIndex > -1)
	{
		if (!pressingDelete && GetAsyncKeyState(VK_DELETE) < 0)
		{
			pressingDelete = true;

			std::vector<SolidLineStrip::SolidLinePoint> points = mSelectedLineStrip->GetLinePoints();

			if (points.size() < 3)
			{
				// should always be 2 points (1 line)
				return;
			}

			points.erase(points.begin() + mSelectedLinePointIndex);

			if (mSelectedLinePointIndex > 0)
			{
				--mSelectedLinePointIndex;
			}

			mSelectedLineStrip->RecalculateLines(points);
		}
	}

	if (GetAsyncKeyState(VK_DELETE) >= 0)
	{
		pressingDelete = false;
	}
}

void LevelEditor::CheckInput_TerrainEditing()
{
	CheckForTerrainPointSelect();

	CheckForTerrainPointMove();

	CheckForTerrainNewPoint();

	CheckForTerrainPointDelete();

	CheckForNewTerrainObject();

	CheckForMaterialAssign();

	CheckForSolidLineStripEdgeAssign();

	CheckForSolidLineSetDropDown();

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

				SolidLineStrip * solidLineStrip = GetAsSolidLineStrip(mSelectedObject);
				if (solidLineStrip)
				{
					vector<SolidLineStrip::SolidLinePoint> points = solidLineStrip->GetLinePoints();
					solidLineStrip->RecalculateLines(points);
				}

				ParticleSpray * particleSpray = GetAsParticleSpray(mSelectedObject);
				if (particleSpray)
				{
					particleSpray->Reset();
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

	CheckForLayerAssign();

	CheckForMaterialAssign();

	CheckForPixelMovement();

	for (auto & obj : gameObjects)
	{
		if (dynamic_cast<ParticleSpray*>(obj.get()))
		{
			obj->Update(Timing::Instance()->GetLastUpdateDelta());
		}
	}
}

SolidLineStrip * LevelEditor::GetAsSolidLineStrip(GameObject * object)
{
	SolidLineStrip * solidLineStrip = dynamic_cast<SolidLineStrip*>(object);

	return solidLineStrip;
}

ParticleSpray * LevelEditor::GetAsParticleSpray(GameObject * object)
{
	ParticleSpray * particleSpray = dynamic_cast<ParticleSpray*>(object);

	return particleSpray;
}

void LevelEditor::Draw()
{
	if (mTerrainEditing)
	{
		Graphics::GetInstance()->DrawDebugText("Terrain Edit Mode", 100, 100);

		if (mSelectedLineStrip && mSelectedLinePointIndex > -1)
		{
			std::vector<SolidLineStrip::SolidLinePoint> points = mSelectedLineStrip->GetLinePoints();

			GAME_ASSERT(points.size() > 0 && mSelectedLinePointIndex < points.size());

			DrawUtilities::DrawTexture(Vector3(points[mSelectedLinePointIndex].WorldPosition.X, points[mSelectedLinePointIndex].WorldPosition.Y, 3),
										Vector2(50, 50),
										"Media\\editor\\circle_selected.png");

			std::string materialName;
			if (mSelectedLineStrip->GetMaterial())
			{
				materialName = mSelectedLineStrip->GetMaterial()->GetMaterialName();
			}

			const int bufferSize = 100;
			char array[bufferSize];
			memset(array, 0, bufferSize);
			sprintf(array, "Selected Line Material: %s", materialName.empty() ? "none" : materialName.c_str());

			Graphics::GetInstance()->DrawDebugText(array, 100, 200);
		}
	}

	Vector2 mousePos = GetMouseWorldPos();

	Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Mouse X,Y: %f %f", mousePos.X, mousePos.Y).c_str(), 100, 150);

	if (GameObjectManager::Instance()->GetCurrentLevelProperties().IsAnimationPreview())
	{
		if (mAnimationPreviewTargetObject)
		{
			float animX = mAnimationPreviewTargetObject->Position().X - mousePos.X;
			float animY = mAnimationPreviewTargetObject->Position().Y - mousePos.Y;

			Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Animation Position X,Y: %f %f", animX, -animY).c_str(), 100, 175);

			auto animation = mAnimationPreviewTargetObject->GetAnimation();

			if (animation)
			{
				int currentFrame = animation->CurrentFrame("body");

				const string & currentSequenceName = animation->CurrentSequenceName("body");

				Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Current Sequence: %s", currentSequenceName.c_str()).c_str(), 100, 200);

				Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Animation Frame: %i", currentFrame).c_str(), 100, 225);
			}
			
		}	
	}
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
		unsigned index = 0;
		for (auto p : points)
		{
			float pointLeft = p.WorldPosition.X - 50;
			float pointRight = p.WorldPosition.X + 50;
			float pointTop = p.WorldPosition.Y + 50;
			float pointBottom = p.WorldPosition.Y - 50;

			if (!(worldPosClicked.X > pointLeft &&
				worldPosClicked.X < pointRight &&
				worldPosClicked.Y > pointBottom &&
				worldPosClicked.Y < pointTop))
			{
				++index;
				continue;
			}

			mSelectedLinePointIndex = index;
			return solidLineStrip;
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

void LevelEditor::CheckForLayerAssign()
{
	if (!mSelectedObject)
	{
		return;
	}

	if (GetAsyncKeyState('1') < 0)
	{
		mSelectedObject->SetParallaxMultiplierX(1.0f);
		mSelectedObject->SetParallaxMultiplierY(1.07f);
	}
	else if (GetAsyncKeyState('2') < 0)
	{
		mSelectedObject->SetParallaxMultiplierX(0.8f);
		mSelectedObject->SetParallaxMultiplierY(1.04f);
	}
	else if (GetAsyncKeyState('3') < 0)
	{
		mSelectedObject->SetParallaxMultiplierX(0.65f);
		mSelectedObject->SetParallaxMultiplierY(1.01f);
	}
	if (GetAsyncKeyState('0') < 0)
	{
		// reset
		/*for (auto & object : GameObjectManager::Instance()->GetGameObjectList())
		{
			if (object->GetParallaxMultiplierY() == 1.07f)
			{
				object->SetParallaxMultiplierY(1.0f);
			}
		}*/

		mSelectedObject->SetParallaxMultiplierX(1.0f);
		mSelectedObject->SetParallaxMultiplierY(1.0f);
	}
}

void LevelEditor::CheckForMaterialAssign()
{
	GameObject * object = mTerrainEditing ? mSelectedLineStrip : mSelectedObject;

	if (!object)
	{
		return;
	}

	if (GetAsyncKeyState('5') < 0)
	{
		Material * material = MaterialManager::Instance()->GetMaterial("grassground");
		GAME_ASSERT(material);

		if (material)
		{
			object->SetMaterial(material);
		}
	}
	else if (GetAsyncKeyState('6') < 0)
	{
		Material * material = MaterialManager::Instance()->GetMaterial("softwood");
		GAME_ASSERT(material);

		if (material)
		{
			object->SetMaterial(material);
		}
	}
	else if (GetAsyncKeyState('7') < 0)
	{
		Material * material = MaterialManager::Instance()->GetMaterial("caverock");
		GAME_ASSERT(material);

		if (material)
		{
			object->SetMaterial(material);
		}
	}
	else if (GetAsyncKeyState('8') < 0)
	{
		Material * material = MaterialManager::Instance()->GetMaterial("metal");
		GAME_ASSERT(material);

		if (material)
		{
			object->SetMaterial(material);
		}
	}
	else if (GetAsyncKeyState('9') < 0)
	{
		Material * material = MaterialManager::Instance()->GetMaterial("ethereal_walkway");
		GAME_ASSERT(material);

		if (material)
		{
			object->SetMaterial(material);
		}
	}
}

void LevelEditor::CheckForSolidLineStripEdgeAssign()
{
	GAME_ASSERT(mTerrainEditing);

	if (!mTerrainEditing)
	{
		return;
	}

	if (!mSelectedLineStrip)
	{
		return;
	}

	static bool pressingLeft = false;

	if (!pressingLeft && GetAsyncKeyState(VK_LEFT) < 0)
	{
		pressingLeft = true;

		if (mSelectedLineStrip->GetHasHardLeftEdge())
		{
			mSelectedLineStrip->SetHasHardLeftEdge(false);
			mSelectedLineStrip->SetHardLeftEdgeOffsetX(0.0f);
		}
		else
		{
			mSelectedLineStrip->SetHasHardLeftEdge(true);
			mSelectedLineStrip->SetHardLeftEdgeOffsetX(20.0f);
		}
	}

	if (GetAsyncKeyState(VK_LEFT) >= 0)
	{
		pressingLeft = false;
	}

	static bool pressingRight = false;

	if (!pressingRight && GetAsyncKeyState(VK_RIGHT) < 0)
	{
		pressingRight = true;

		if (mSelectedLineStrip->GetHasHardRightEdge())
		{
			mSelectedLineStrip->SetHasHardRightEdge(false);
			mSelectedLineStrip->SetHardRightEdgeOffsetX(0.0f);
		}
		else
		{
			mSelectedLineStrip->SetHasHardRightEdge(true);
			mSelectedLineStrip->SetHardRightEdgeOffsetX(20.0f);
		}
	}

	if (GetAsyncKeyState(VK_RIGHT) >= 0)
	{
		pressingRight = false;
	}
}

void LevelEditor::CheckForSolidLineSetDropDown()
{
	GAME_ASSERT(mTerrainEditing);

	if (!mTerrainEditing)
	{
		return;
	}

	if (!mSelectedLineStrip)
	{
		return;
	}

	static bool pressingDown = false;

	if (!pressingDown && GetAsyncKeyState(VK_DOWN) < 0)
	{
		pressingDown = true;

		if (mSelectedLineStrip->GetCanDropDown())
		{
			mSelectedLineStrip->SetCanDropDown(false);
		}
		else
		{
			mSelectedLineStrip->SetCanDropDown(true);
		}
	}

	if (GetAsyncKeyState(VK_DOWN) >= 0)
	{
		pressingDown = false;
	}
}

void LevelEditor::CheckForPixelMovement()
{
	GAME_ASSERT(!mTerrainEditing);

	if (mTerrainEditing)
	{
		return;
	}

	if (!mSelectedObject)
	{
		return;
	}

	if (GetAsyncKeyState(VK_UP))
	{
		mSelectedObject->SetY(mSelectedObject->Y() + 0.1f);
		mSelectedObject->Update(0);
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		mSelectedObject->SetX(mSelectedObject->X() - 0.1f);
		mSelectedObject->Update(0);
	}

	if (GetAsyncKeyState(VK_DOWN))
	{
		mSelectedObject->SetY(mSelectedObject->Y() - 0.1f);
		mSelectedObject->Update(0);
	}

	if (GetAsyncKeyState(VK_RIGHT))
	{
		mSelectedObject->SetX(mSelectedObject->X() + 0.1f);
		mSelectedObject->Update(0);
	}
}

void LevelEditor::CheckForInvokeObjectEditor()
{
	static bool pressingKey = false;

	if (!pressingKey && GetAsyncKeyState(VK_TAB) < 0)
	{
		pressingKey = true;

		if (UIManager::Instance()->IsObjectEditorDisplaying())
		{
			UIManager::Instance()->DismissObjectEditor();
		}
		else if (mSelectedObject)
		{
			UIManager::Instance()->DisplayObjectEditor(mSelectedObject);
		}
	}

	if (GetAsyncKeyState(VK_TAB) >= 0)
	{
		pressingKey = false;
	}
}
