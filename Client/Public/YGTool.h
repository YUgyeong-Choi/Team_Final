#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

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
	_float m_fDuration = {};
	INTERPOLATION_CAMERA m_eInterpMode = INTERPOLATION_CAMERA::NONE;

	int m_iSelectedFrameIndex = -1;
	XMFLOAT3 m_editedPos{}, m_editedRot{};
	float m_editedDuration = 0.f;
	int m_iLastSelectedIndex = -1;
	INTERPOLATION_CAMERA m_eEditInterpMode = INTERPOLATION_CAMERA::NONE;
public:
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END