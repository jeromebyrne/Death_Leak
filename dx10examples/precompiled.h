#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#include <windows.h>
#include <Mmsystem.h>
#include <typeinfo.h>
#include "resource.h"
#include "dxut\dxut.h"
#include "dxut\SDKmesh.h"
#include <string>
#include "graphics.h"
#include "Timing.h"
#include "VertexTypes.h"
#include <map>
#include <vector>
#include <list>
#include <math.h>
#define PI_VAL 3.14159265
using namespace std;
#include "TextureManager.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#include "TinyXml\tinyxml.h"
#include "Utilities.h"
#include "XmlDocument.h"
#include "gameobjectManager.h"
#include "xmlutilities.h"
#include "effectmanager.h"
#include "Environment.h"
#include "Camera2D.h"

const int kLargestInt = 2147483646;

// used for sprite drawing
enum EffectTypesEnum {EFFECT_BASIC, EFFECT_LIGHT_TEXTURE, EFFECT_VERTEX_WOBBLE, EFFECT_REFLECT, EFFECT_PARTICLE_SPRAY, EFFECT_BUMP, EFFECT_NOISE, EFFECT_PIXEL_WOBBLE };

//AUDIO
#include <irrKlang.h>
using namespace irrklang;

// UI event types
enum UIEventTypesEnum { UI_PRESS_DOWN, UI_PRESS_UP };

struct EventStruct
{
public:
	string EventName;
	list<string> EventParams;
};

#endif