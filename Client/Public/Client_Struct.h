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

#pragma endregion
	typedef struct tagCutScene
	{
		_matrix worldMatrix;
		INTERPOLATION_CAMERA eInterp;
		_float fInterpDuration;
		_bool bZoom;
		_float fFov;
		_float fFovDuration;
	}CUTSCENE_DESC;

}