#ifndef SOLIDLINE_H
#define SOLIDLINE_H

#include "solidmovingsprite.h"
#include <set>

class SolidLine : public SolidMovingSprite
{
public:

	SolidLine(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~SolidLine(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void OnCollision(SolidMovingSprite * object) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	virtual void DebugDraw(ID3D10Device *  device) override;

private:

	virtual void SetupDebugDraw() override;

	void CalculateVariables();

	bool Intersect(Vector2 & otherStart, Vector2 & otherEnd, Vector2 & intersectPointOut);

	Vector2 mStartPos;
	Vector2 mEndPos;
	float mLength;
	Vector2 mLineDirection;
	Vector2 mNormal;

	VertexPositionColor mDebugLineVertices[2];
	ID3D10Buffer* mDebugLineVBuffer;
};

#endif
