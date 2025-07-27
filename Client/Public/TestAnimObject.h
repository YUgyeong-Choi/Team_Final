#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CTestAnimObject : public CGameObject
{
private:
	CTestAnimObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestAnimObject(const CTestAnimObject& Prototype);
	virtual ~CTestAnimObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	void Input_Test(_float fTimeDelta);

private:
	CModel* m_pModelCom = nullptr; // �� ������Ʈ
	CShader* m_pShaderCom = nullptr; // ���̴� ������Ʈ
	CAnimator* m_pAnimator = nullptr; // �ִϸ�����

public:
	static CTestAnimObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

