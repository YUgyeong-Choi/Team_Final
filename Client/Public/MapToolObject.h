#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CMapToolObject final : public CGameObject
{
public:
	typedef struct tagMapToolObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_tchar		szModelPrototypeTag[MAX_PATH];
	}MAPTOOLOBJ_DESC;

private:
	CMapToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapToolObject(const CMapToolObject& Prototype);
	virtual ~CMapToolObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	const wstring* Get_ModelPrototypeTag() const {
		return &m_ModelPrototypeTag;
	}

private:
	wstring m_ModelPrototypeTag = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

private:
	HRESULT Ready_Components();

	HRESULT Bind_ShaderResources();

public:
	static CMapToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END