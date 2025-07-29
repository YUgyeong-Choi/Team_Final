#pragma once

#pragma warning (disable : 4251)

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>

#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/Effects.h"
#include "DirectXTK/ScreenGrab.h"

#include "Fx11/d3dx11effect.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include <d3dcompiler.h>
using namespace DirectX;

#include <string>
#include <map>
#include <list>
#include <vector>
#include <set>
#include <unordered_map>
#include <cmath>
using namespace std;

#include <fstream>
#include <filesystem>
using namespace std::filesystem;
#include <PxPhysicsAPI.h>
using namespace physx;

#include "Engine_Typedef.h"
#include "Engine_Macro.h"
#include "Engine_Enum.h"
#include "Engine_Struct.h"
#include "Engine_Function.h"


#include "json.hpp"
using json = nlohmann::json;


namespace Engine
{
	const _wstring	g_strTransformTag = TEXT("Com_Transform");
	const _uint g_iMaxNumBones = 512;

	//const unsigned int g_iMaxWidth = 16384;
	//const unsigned int g_iMaxHeight = 9216;	

	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;


	const unsigned int g_iMiddleWidth = 3200;
	const unsigned int g_iMiddleHeight = 1800;

	extern _bool g_bSceneChanging;

}

using namespace Engine;
namespace fs = std::filesystem;




#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif
#endif



