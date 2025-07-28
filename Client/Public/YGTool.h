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
	_bool m_bUseLerp = true;

	int selectedFrameIndex = -1;
	XMFLOAT3 editedPos{}, editedRot{};
	float editedDuration = 0.f;
	_bool editedLerp = false;
	int lastSelectedIndex = -1;
public:
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END