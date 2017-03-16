#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

class InventoryManager
{
public:

	InventoryManager(void);
	~InventoryManager(void);

	static InventoryManager * GetInstance();

	void Initialise();

private:

	static InventoryManager * m_instance;
};

#endif
