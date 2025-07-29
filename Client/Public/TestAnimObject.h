#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXController;
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
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	void Input_Test(_float fTimeDelta);

	HRESULT Ready_Collider();
	void SyncTransformWithController();
private:
	CModel* m_pModelCom = nullptr; // 모델 컴포넌트
	CShader* m_pShaderCom = nullptr; // 셰이더 컴포넌트
	CAnimator* m_pAnimator = nullptr; // 애니메이터
	CPhysXController* m_pControllerCom = { nullptr };

	//중력땜시
private:
	bool m_bOnGround = false;
	PxVec3 m_vVelocity = PxVec3(0.f, 0.f, 0.f);
	XMFLOAT3 m_vGravityVelocity = { 0.f, 0.f, 0.f };  // 헤더에 선언
public:
	static CTestAnimObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

