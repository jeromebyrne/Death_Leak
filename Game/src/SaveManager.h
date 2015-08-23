#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

class SaveManager
{	
public:
	static SaveManager * GetInstance();

	void ReadSaveFile();

	void WriteSaveFile();

private:

	static SaveManager * mInstance;

	SaveManager();
	~SaveManager(void) {}

};

#endif
