#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <typeinfo>
#include <vector>

#include "tinyxml.h"
#include "Engine/Platform/MacDxCompat.h"
using namespace std;

#define PI_VAL 3.14159265

enum EffectTypesEnum {EFFECT_BASIC, EFFECT_LIGHT_TEXTURE, EFFECT_VERTEX_WOBBLE, EFFECT_REFLECT, EFFECT_PARTICLE_SPRAY, EFFECT_BUMP, EFFECT_NOISE, EFFECT_PIXEL_WOBBLE, EFFECT_FOLIAGE_SWAY};

enum UIEventTypesEnum { UI_PRESS_DOWN, UI_PRESS_UP };

struct EventStruct
{
public:
	string EventName;
	list<string> EventParams;
};

#include "Core/Vector2.h"
#include "Core/Vector3.h"
#include "Core/Vector4.h"
#include "Core/VertexTypes.h"
#include "Core/Timing.h"
#include "Utils/Utilities.h"
#include "Core/XmlDocument.h"
#include "Core/XmlUtilities.h"
#include "Core/Logger.h"
#include "TextureManager.h"
#include "Core/Graphics.h"
#include "GameObjects/GameObjectManager.h"
#include "Effects/EffectManager.h"
#include "Camera2D.h"
using namespace irrklang;

#ifndef GAME_ASSERT_ENABLED
	#ifdef _RELEASE
		#define GAME_ASSERT_ENABLED 0
	#else
		#define GAME_ASSERT_ENABLED 1
	#endif
#endif

#if GAME_ASSERT_ENABLED == 1
	#define GAME_ASSERT(CONDITION)\
		do\
		{\
			if (!(CONDITION))\
			{\
				std::fprintf(stderr, "*** GameAssert FAILED! ***: %s\nFile: %s, Line: %i\n", #CONDITION, __FILE__, __LINE__);\
				__builtin_trap();\
			}\
		} while (0)
#else
	#define GAME_ASSERT(CONDITION)
#endif

#else

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <memory>
#include <limits>
#include <Mmsystem.h>
#include <typeinfo>
#include "resource.h"
#include "dxut.h"
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
#include "tinyxml.h"
#include "Utilities.h"
#include "XmlDocument.h"
#include "gameobjectManager.h"
#include "xmlutilities.h"
#include "effectmanager.h"
#include "Camera2D.h"
#include "Logger.h"

// used for sprite drawing
enum EffectTypesEnum {EFFECT_BASIC, EFFECT_LIGHT_TEXTURE, EFFECT_VERTEX_WOBBLE, EFFECT_REFLECT, EFFECT_PARTICLE_SPRAY, EFFECT_BUMP, EFFECT_NOISE, EFFECT_PIXEL_WOBBLE, EFFECT_FOLIAGE_SWAY };

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

#ifndef GAME_ASSERT_ENABLED
	#ifdef _RELEASE
		#define GAME_ASSERT_ENABLED 0
	#else
		#define GAME_ASSERT_ENABLED 1
	#endif
#endif

#if GAME_ASSERT_ENABLED == 1
	
	#define GAME_ASSERT(CONDITION)\
		\
		do\
		{\
			if (!CONDITION)\
			{\
				LOG_ERROR("*** GameAssert FAILED! ***: %s \nFile: %s, Line: %i\n", #CONDITION, __FILE__, __LINE__ );\
				DebugBreak();\
			}\
		\
		} while (0)
#else
	#define GAME_ASSERT(CONDITION)
#endif

#endif

#endif
