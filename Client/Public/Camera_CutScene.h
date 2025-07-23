#pragma once

#include "Client_Defines.h"
#include "Camera.h"


NS_BEGIN(Engine)
class CGameObject;
NS_END


NS_BEGIN(Client)

class CCamera_CutScene final : public CCamera
{
public:
	typedef struct tagCameraCutSceneDesc : public CCamera::CAMERA_DESC
	{
		_float			fMouseSensor = {};
	}CAMERA_CutScene_DESC;

private:
	CCamera_CutScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_CutScene(const CCamera_CutScene& Prototype);
	virtual ~CCamera_CutScene() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void	Set_FOV(_float FOV) { m_fFov = FOV; }

private:
	_float			m_fMouseSensor = { };
	_float			m_fPanSpeed = { };

	CGameObject* m_pPlayer = { nullptr };

public:
	static CCamera_CutScene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

