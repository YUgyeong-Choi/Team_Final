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
	HRESULT Render_CameraTool();
	HRESULT Render_CameraFrame();
	vector<CUTSCENE_DESC> m_vecCameraFrame;
	CUTSCENE_DESC m_CutSceneDesc = {};
	_float m_fInterpDuration = {};
	INTERPOLATION_CAMERA m_eInterpMode = INTERPOLATION_CAMERA::NONE;
	_bool m_bZoom = false;
	_float m_fFov = 60.f;
	_float m_fFovDuration = {};

	int m_iSelectedFrameIndex = -1;
	int m_iLastSelectedIndex = -1;
	XMFLOAT3 m_editedPos{}, m_editedRot{};
	float m_fEditedInterpDuration = 0.f;
	INTERPOLATION_CAMERA m_eEditInterpMode = INTERPOLATION_CAMERA::NONE;
	_bool m_bEditZoom = false;
	_float m_fEditFov = 60.f;
	_float m_fEditFovDuration = {};

	CCameraSequence* m_CameraSequence;
	int m_iSelectedIndex = 0;


	_int m_iEndFrame = {};
public:
	HRESULT Render_CameraSequence();
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END