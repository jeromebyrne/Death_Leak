#ifndef SOLIDLINESTRIP_H
#define SOLIDLINESTRIP_H

#include "solidmovingsprite.h"
#include <set>

class SolidLineStrip : public SolidMovingSprite
{
public:

	struct SolidLinePoint
	{
		Vector2 LocalPosition;
		Vector2 WorldPosition;
	};

	SolidLineStrip(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~SolidLineStrip(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void OnCollision(SolidMovingSprite * object) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	virtual void DebugDraw(ID3D10Device *  device) override;

	const vector<SolidLinePoint> GetLinePoints()  const { return mPoints; }

private:

	struct SolidLine
	{
		SolidLinePoint StartPoint;
		SolidLinePoint EndPoint;
		Vector2 Normal;
		Vector2 BoundingBox;
		Vector2 LineDirection;
		float Length;
		Vector2 MidPointWorld;
	};

	void CalculateLines();

	bool Intersect(SolidLine & solidLine, Vector2 & otherStart, Vector2 & otherEnd, Vector2 & intersectPointOut);

	bool BoxHitCheck(SolidLine & solidLine, SolidMovingSprite * object);

	vector<SolidLinePoint> mPoints;
	vector<SolidLine> mLines;
};

#endif
