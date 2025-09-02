#pragma once

#include "Engine_Defines.h"

namespace Client
{
	typedef struct tagIsMenuOpen {
		bool	ShowTerrainMenu = false;
		bool	ShowObjectMenu = false;
		bool	ShowConvertMenu = false;
	}IMGUIWINDATA;

	struct EELEMENTCONDITION
	{
		_bool		bIsActive = {};
		_float		fDuration = {};
		_float		fElementWeight = {};
		EELEMENT	eElement = { EELEMENT::END };
	};

#pragma region GL

	// �����ߵ�?

	// �� ui ��ɿ� �ʿ��� ������ ��Ƽ� �����ϰ�, json���� �ҷ����� ����

	// tool���� ����ϰ�, �̰� ������� seqeunce�� ���� �����ϱ�
	// ������ type���� �ʿ��ѰŸ� �����ϰ�, �ҷ�����
	typedef struct tagFeatureUITool{
		_int iType{};
		unsigned int color{};
		string strTypeTag{}; // 
		_int iStartFrame{}, iEndFrame{};
		_bool  isLoop{};

		// uv��
		_float2 fOffsetUV{};
		_float2 fStartUV{};

		// fade��
		_float			fStartAlpha{};
		_float			fEndAlpha{};

		_float2 fStartPos{};
		_float2 fEndPos{};

		// scale
		_float2			fStartScale{};
		_float2			fEndScale{};

		// color
		_float4			fStartColor{};
		_float4			fEndColor{};

		// rotation
		_float			fStartRotation{};
		_float			fEndRotation{};
		// ��� �߽����� ������
		_float2			fRotationPos{};
		_float2			fInitPos{};
		
	}UI_FEATURE_TOOL_DESC;

	// ��ü ���� ����� ����ü
	typedef struct tagFeatureUI{
		string strProtoTag{}; // � feature component�� �����ð���
		_int iStartFrame{}, iEndFrame{}; // ���� ������ �� ������
		_bool  isLoop{}; // ���� ����

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

		_float2 fStartScale{}; // 0 ���� ��������?
		_float2 fEndScale{};

	}UI_FEATURE_SCALE_DESC;

	typedef struct tagRotationFeature : public tagFeatureUI {

		_float			fStartRotation{};
		_float			fEndRotation{};
		// ��� �߽����� ������
		_float2			fRotationPos{};
		_float2			fInitPos{};

	}UI_FEATURE_ROTATION_DESC;

	typedef struct tagColorFeature : public tagFeatureUI {

		_float4 fStartColor{}; 
		_float4 fEndColor{};

	}UI_FEATURE_COLOR_DESC;

	// �������?

	// �������� ���� ����ü

	typedef struct tagItemDesc {
		_wstring strPrototag;
		_bool    isSelect;
		_bool    isUsable;
		_bool    isConsumable;
		_int     iUseCount;
		_int     iItemIndex;
	}ITEM_DESC;


	typedef struct tagSkillDesc {
		_int  iSkillType; // 0 ���� 1 ��� 2 ��ƿ
		_float  fManaCost; // �ѹ��� �����
		_bool isCombo;
		_int  iCountCombo; // �� �޺����� �Ǵ���
		
	}SKILL_DESC;

#pragma endregion

#pragma region YG
	struct CAMERA_WORLDFRAME
	{
		_int iKeyFrame;
		_matrix WorldMatrix;
		INTERPOLATION_CAMERA interpMatrixPos;
	};

	struct CAMERA_POSFRAME
	{
		_int iKeyFrame;
		XMFLOAT3 offSetPos;
		INTERPOLATION_CAMERA interpOffSetPos;
	};
	struct CAMERA_ROTFRAME
	{
		_int iKeyFrame;
		XMFLOAT3 offSetRot;
		INTERPOLATION_CAMERA interpOffSetRot;
	};
	struct CAMERA_FOVFRAME
	{
		_int iKeyFrame;
		_float fFov;
		INTERPOLATION_CAMERA interpFov;
	};

	struct CAMERA_TARGETFRAME
	{
		_int iKeyFrame;
		TARGET_CAMERA eTarget;
		_float fPitch;
		_float fYaw;
		_float fDistance;
	};

	struct CAMERA_FRAMEDATA
	{
		_int iEndFrame;
		_bool bOrbitalToSetOrbital;
		_bool bReadyCutSceneOrbital;
		_float fPitch;
		_float fYaw;
		vector<CAMERA_WORLDFRAME> vecWorldMatrixData;
		vector<CAMERA_POSFRAME> vecOffSetPosData;
		vector<CAMERA_ROTFRAME> vecOffSetRotData;
		vector<CAMERA_FOVFRAME> vecFovData;
		vector<CAMERA_TARGETFRAME> vecTargetData;
	};

#pragma endregion
}