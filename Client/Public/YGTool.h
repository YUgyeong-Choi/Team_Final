#pragma once
#include "GameObject.h"

#include "Client_Defines.h"
#include "CameraSequence.h"
NS_BEGIN(Client)

class CYGTool final : public CGameObject
{
private:
	CYGTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGTool(const CYGTool& Prototype);
	virtual ~CYGTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
private:
	json SaveCameraFrameData(const CAMERA_FRAMEDATA& data);
	void SaveToJsonFile(const std::string& filePath, const CAMERA_FRAMEDATA& data);
	CAMERA_FRAMEDATA LoadCameraFrameData(const json& j);

	HRESULT Render_CameraTool();
	HRESULT Render_CameraFrame();
	HRESULT Render_CameraSequence();
	void Render_SetInfos();

	/* [ 시퀀스 관련 ] */
	CCameraSequence* m_CameraSequence;
	_int m_iCurrentFrame = 0;
	_bool m_bExpanded = true;
	_int m_iSelected = -1;
	_int m_iFirstFrame = 0;

	_int m_iEndFrame = 10;

	CAMERA_FRAMEDATA m_CameraDatas;
	CCameraSequence::CAMERA_KEY* m_pSelectedKey = nullptr;

	_int m_iEditKey = -1;
	CAMERA_WORLDFRAME m_EditMatrixPosKey = {};
	CAMERA_POSFRAME m_EditOffSetPosKey = {};
	CAMERA_ROTFRAME m_EditOffSetRotKey = {};
	CAMERA_FOVFRAME m_EditFovKey = {};
	CAMERA_TARGETFRAME m_EditTargetKey = {};
	_int m_iChangeKeyFrame = 0;
	CUTSCENE_TYPE m_eCutSceneType = CUTSCENE_TYPE::WAKEUP;

	_float m_fPitch = {};
	_float m_fYaw = {};

	bool bStopCamera = false; // 상태 저장용
	int iStopFrame = 0;

	_int   curveType = 0;                         // 0=Linear, 1=EaseIn, 2=EaseOut, 3=EaseInOut, 4=Custom5
	_float curveY[5] = { 0.f, 0.25f, 0.5f, 0.75f, 1.f }; // Custom5 전용 y포인트(고정 x=[0,.25,.5,.75,1])
public:
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

