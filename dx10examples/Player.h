#ifndef PLAYER_H
#define PLAYER_H
#include "character.h"

class Player : public Character
{
private:

public:
	Player(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0);
	virtual ~Player(void);
	virtual void Update(float delta) override;
	void Initialise() override;
	void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	void OnCollision(SolidMovingSprite * object) override;
	virtual void OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode = true) override;
};

#endif
