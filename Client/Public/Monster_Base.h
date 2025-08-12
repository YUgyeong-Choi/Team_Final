#pragma once
#include "Unit.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CMonster_Base : public CUnit
{
public:
	enum class STATE_MONSTER { IDLE, WALK, RUN, TURN, DEAD, HIT, GROGGY, FATAL, ATTACK, END };
	enum class MONSTER_DIR { F, B, L, R, END };

protected:
	CMonster_Base(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster_Base(const CMonster_Base& Prototype);
	virtual ~CMonster_Base() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private: /* [ Setup �Լ� ] */
	HRESULT Ready_Components();
	void	Add_PartObject();
	void	RootMotionActive(_float fTimeDelta);
	void	Update_State();

	void    LoadAnimDataFromJson();

	void    Update_Collider();

	


private:

	CGameObject* m_pTarget = { nullptr };
	STATE_MONSTER	m_eCurrentState = {};

	vector<class CPartObject*> m_PartObjects; // hp ��, �ʿ��ϸ� ���� �ֱ�

	// Ư�� ���¿��� ��� �ٶ󺸰� �ؾ�?
	_bool			m_isLookAt = {};

	// �ൿ ������ ���� ������
	MONSTER_DIR		m_eDir = { MONSTER_DIR::END };
	_int			m_iStamina = {100};
	_int			m_iGroggyThreshold = {};
	_bool			m_isCanGroggy = {};



public:
	static CMonster_Base* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

