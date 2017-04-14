#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

class InventoryManager
{
public:

	InventoryManager(void);
	~InventoryManager(void);

	static InventoryManager * GetInstance();

	void Initialise();

	bool HasKey(const std::string & keyId);

	void SetHasKey(const std::string & keyId, bool value);

private:

	static InventoryManager * m_instance;
};

#endif
