#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CPreviewObject final : public CGameObject
{
private:
	CPreviewObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPreviewObject(const CPreviewObject& Prototype);
	virtual ~CPreviewObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void Reset_CameraWorldMatrix();

	HRESULT Change_Model(wstring ModelPrototypeTag);

	CTransform* Get_CameraTransformCom() const {
		return m_pCameraTransformCom;
	}

private:
	wstring m_ModelPrototypeTag = {};

private:
	ID3D11DepthStencilView* m_pDSV = { nullptr };
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CTransform* m_pCameraTransformCom = { nullptr };
private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_DepthStencilView(_uint iWidth, _uint iHeight);
	HRESULT Bind_ShaderResources();

public:
	static CPreviewObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END