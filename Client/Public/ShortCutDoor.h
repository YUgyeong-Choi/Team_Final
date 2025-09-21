#pragma once
/* [ �߿� ������ �� �� key �ʿ��� ���� ] */
#include "Client_Defines.h"
#include "DefaultDoor.h"

NS_BEGIN(Client)

class CShortCutDoor : public CDefaultDoor
{
protected:
	CShortCutDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CShortCutDoor(const CShortCutDoor& Prototype);
	virtual ~CShortCutDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	void OpenDoor();
	void ActivateUnlock(); // �ڹ��� ���� �ִϸ��̼� ���

	void Move_Player(_float fTimeDelta);
	void Register_Events();
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT LoadFromJson();
private:
	HRESULT Render_Key();
	void Start_Effect(_float fTimeDelta);
	void Play_Sound(_float fTimeDelta) override;
private:
	_bool m_bCanOpen = false;

	// �÷��̾� move Ǯ�� ���� 
	_float m_fEscapeTime = 0.f;
	_bool m_bCanMovePlayer = false;

	// ����Ʈ 
	_bool m_bEffectActive = false;
	_float m_fEffectTime = 0.f;

	// �ڹ���
	class CModel* m_pModelComFrontKey = { nullptr };
	class CAnimator* m_pAnimatorFrontKey = { nullptr };

	class CModel* m_pModelComBackKey = { nullptr };
	class CAnimator* m_pAnimatorBackKey = { nullptr };

	_bool m_bSoundPlay[5] = {};
public:
	static CShortCutDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END