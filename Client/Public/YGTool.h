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

	CCameraSequence* m_CameraSequence;
	_int m_iCurrentFrame = 0;
	_bool m_bExpanded = true;
	_int m_iSelected = -1;
	_int m_iFirstFrame = 0;

	_int m_iEndFrame = 10;

	CAMERA_FRAMEDATA m_CameraDatas;
	CCameraSequence::CAMERA_KEY* m_pSelectedKey = nullptr;

	_int m_iEditKey = -1;
	CAMERA_WORLDFRAME m_EditPosKey = {};
	CAMERA_ROTFRAME m_EditRotKey = {};
	CAMERA_FOVFRAME m_EditFovKey = {};
	_int m_iChangeKeyFrame = 0;
	CUTSCENE_TYPE m_eCutSceneType = CUTSCENE_TYPE::ONE;
public:
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

