#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "UI_Button_Script.h"

NS_BEGIN(Client)
class CWego : public CUnit
{
private:
	enum WEGOTALKTYPE {ONE, TWO};
public:
	typedef struct tagWegoDesc : public CUnit::tagUnitDesc
	{
	}WEGO_DESC;
protected:
	CWego(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWego(const CWego& Prototype);
	virtual ~CWego() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
	HRESULT Ready_Trigger();

	void LoadNpcTalkData(string filePath);

	void LoadAnimDataFromJson();

private:
	void Update_Button();

private:
	CPhysXDynamicActor* m_pPhysXTriggerCom = { nullptr };

	_bool m_bInTrigger = false;
	_bool m_bTalkActive = false;


	WEGOTALKTYPE m_curTalkType = WEGOTALKTYPE::ONE;
	_int m_curTalkIndex = 0;

	string m_strNpcName = {};
	unordered_map<WEGOTALKTYPE, vector<string>> m_NpcTalkData;
	_bool m_bAutoTalk = {};
	
	_bool m_bFinish = false;

	// 버튼에 필요한 변수들?
	vector<CUI_Button_Script*> m_pSelectButtons;
	_int m_iButtonActiveIndex = {-1};
	_int m_iSelectButtonIndex = {};
	vector<_int> m_NextIndex = {};
	vector<_bool> m_CheckButtonSelct = {};

public:
	static CWego* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

