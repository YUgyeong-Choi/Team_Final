#pragma once
#include "Unit.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CMonster_Test : public CUnit
{
public:
	enum class STATE_MONSTER {IDLE, WALK, DEAD, END};

protected:
	CMonster_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster_Test(const CMonster_Test& Prototype);
	virtual ~CMonster_Test() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private: /* [ Setup ÇÔ¼ö ] */
	HRESULT Ready_Components();
	void	Add_PartObject();
	void	RootMotionActive(_float fTimeDelta);
	void	Update_State();

	void    LoadAnimDataFromJson();


private:

	CGameObject*	m_pTarget = { nullptr };
	STATE_MONSTER	m_eCurrentState = {};

	vector<class CPartObject*> m_PartObjects;


public:
	static CMonster_Test* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

