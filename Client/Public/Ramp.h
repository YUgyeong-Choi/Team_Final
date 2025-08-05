#pragma once

#include "Client_Defines.h"
#include "Item.h"


// �ʿ��ϸ� �߰�, ��Ʈ�� �޷��ִ°� ǥ���ؾ� �ɵ�?
NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END


NS_BEGIN(Client)

class CRamp final : public CItem
{
private:
	CRamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRamp(const CRamp& Prototype);
	virtual ~CRamp() = default;

public:
	_bool	Get_isLight() { return m_isLight; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	// ��� ȿ�� ��ӹ޾Ƽ� �����ϱ�
	virtual void Activate();

	HRESULT Ready_Components();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
private:
	//
	_bool		m_isLight = { false };

public:
	static CRamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END