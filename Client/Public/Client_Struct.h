#pragma once

#include "Engine_Defines.h"

namespace Client
{
	typedef struct tagIsMenuOpen {
		bool	ShowTerrainMenu = false;
		bool	ShowObjectMenu = false;
		bool	ShowConvertMenu = false;
	}IMGUIWINDATA;


#pragma region GL
	// 각 ui 기능에 필요한 정보를 담아서 저장하고, json으로 불러오기 위해


	// tool에서 사용하고, 이거 기반으로 seqeunce를 만들어서 조작하기
	// 저장은 type따라서 필요한거만 저장하고, 불러오기
	typedef struct tagFeatureUITool{
		_int iType{};
		unsigned int color{};
		string strTypeTag{}; // 
		_int iStartFrame{}, iEndFrame{};
		_bool  isLoop{};

		// uv용
		_float2 fOffsetUV{};
		_float2 fStartUV{};

		// fade용
		_float			fStartAlpha{};
		_float			fEndAlpha{};

		_float2 fStartPos{};
		_float2 fEndPos{};

		// scale
		_float2			fStartScale{};
		_float2			fEndScale{};
		
	}UI_FEATURE_TOOL_DESC;


	// 객체 에서 사용할 구조체
	typedef struct tagFeatureUI{
		string strProtoTag{}; // 어떤 feature component를 가져올건지
		_int iStartFrame{}, iEndFrame{}; // 시작 프레임 끝 프레임
		_bool  isLoop{}; // 루프 적용

	}UI_FEATRE_DESC;

	typedef struct tagUVFeature : public tagFeatureUI{
	
		_float2 fOffsetUV{};
		_float2 fStartUV{};


	}UI_FEATURE_UV_DESC;

	typedef struct tagFadeFeature : public tagFeatureUI {

		
		_float			fStartAlpha{};
		_float			fEndAlpha{};

	}UI_FEATURE_FADE_DESC;

	typedef struct tagPosFeature : public tagFeatureUI {

		_float2 fStartPos{};
		_float2 fEndPos{};
	
	}UI_FEATURE_POS_DESC;

	typedef struct tagScaleFeature : public tagFeatureUI {

		_float2 fStartScale{}; // 0 들어가면 오류날듯?
		_float2 fEndScale{};

	}UI_FEATURE_SCALE_DESC;

	// 옵저버에 던질 구조체

	typedef struct tagItemDesc {
		_wstring strPrototag;
		_bool    isSelect;
		_bool    isUsable;
		_bool    isConsumable;
		_int     iUseCount;
		_int     iItemIndex;
	}ITEM_DESC;


	typedef struct tagSkillDesc {
		_int  iSkillType; // 0 공격 1 방어 2 유틸
		_int  iManaCost; // 한번에 사용할
		_bool isCombo;
		_int  iCountCombo; // 몇 콤보까지 되는지
		
	}SKILL_DESC;

#pragma endregion

#pragma region YG
	struct CAMERA_WORLDFRAME
	{
		_int keyFrame;
		_matrix WorldMatrix;
		INTERPOLATION_CAMERA interpPosition;
	};

	struct CAMERA_POSFRAME
	{
		_int keyFrame;
		XMFLOAT3 offSetPos;
		INTERPOLATION_CAMERA interpPosition;
	};
	struct CAMERA_ROTFRAME
	{
		_int keyFrame;
		XMFLOAT3 offSetRot;
		INTERPOLATION_CAMERA interpRotation;
	};
	struct CAMERA_FOVFRAME
	{
		_int keyFrame;
		_float fFov;
		INTERPOLATION_CAMERA interpFov;
	};

	struct CAMERA_FRAMEDATA
	{
		_int iEndFrame;
		_bool bOrbitalToSetOrbital;
		_bool bReadyCutSceneOrbital;
		_float fPitch;
		_float fYaw;
		vector<CAMERA_WORLDFRAME> vecWorldMatrixData;
		vector<CAMERA_ROTFRAME> vecRotData;
		vector<CAMERA_FOVFRAME> vecFovData;
	};

#pragma endregion
}