#ifndef BASIC_LIGHT_H
#define BASIC_LIGHT_H

class BasicLight
{
private:
	
public:
	BasicLight(D3DXVECTOR4 direction, D3DXVECTOR4 color);
	~BasicLight(void);

	D3DXVECTOR4 Direction; // direction 
    D3DXVECTOR4 Color; // color
};

#endif
