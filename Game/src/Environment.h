#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class Material;

// class which holds level environment data
class Environment
{
public:
	Environment(void);
	~Environment(void);
	static Environment* Instance();
	void Initialise(int groundLevel);
	inline int GroundLevel()
	{
		return m_groundLevel;
	}
	inline Material * GroundMaterial()
	{
		return m_groundMaterial;
	}

private:

	static Environment * m_instance;
	int m_groundLevel;
	Material * m_groundMaterial;
};

#endif
