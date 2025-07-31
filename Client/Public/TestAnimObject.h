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

	PxRigidActor* Get_Actor() { return m_pControllerCom->Get_Actor(); }
private:
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	void Input_Test(_float fTimeDelta);

	HRESULT Ready_Collider();
	void SyncTransformWithController();

private:/* [ ĳ�����̵� �����Լ� ] */
	HRESULT UpdateShadowCamera();
private: /* [ �̵����� ] */
	void SetMoveState(_float fTimeDelta);
	void SetPlayerMoveState(_float fTimeDelta);

	void RayCast();

private:
	PxVec3 m_vRayHitPos = {};
	_bool m_bRayHit = false;

private:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };

private:
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private:
	CModel* m_pModelCom = nullptr; // �� ������Ʈ
	CShader* m_pShaderCom = nullptr; // ���̴� ������Ʈ
	CAnimator* m_pAnimator = nullptr; // �ִϸ�����
	CPhysXController* m_pControllerCom = { nullptr };

	//�߷¶���
private:
	bool m_bOnGround = false;
	PxVec3 m_vVelocity = PxVec3(0.f, 0.f, 0.f);
	XMFLOAT3 m_vGravityVelocity = { 0.f, 0.f, 0.f };  // ����� ����
public:
	static CTestAnimObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

