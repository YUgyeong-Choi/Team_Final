#pragma once

#include "Transform.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{
		_tchar		szName[MAX_PATH];
	}GAMEOBJECT_DESC;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	CComponent* Get_Component(const _wstring& strComponentTag);
	CTransform* Get_TransfomCom() const {
		return m_pTransformCom;
	}
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow() { return S_OK; }

public:
	void Compute_ViewZ(const _vector* pPos);
public:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}

	// Ray로 인항 충돌을 하면 On_Hit를 호출함 (HitPos & HitNormal을 가지고 올 수 있음)
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
public:
	virtual void Set_bDead() { m_bDead = true; }
	virtual _bool Get_bDead();

	void Set_isActive(_bool isActive) { m_isActive = isActive; }
	_bool Get_isActive() { return m_isActive; }

	virtual _float Get_TimeScale() { return m_fTimeScale; }

	void PrintMatrix(const char* szName, const _matrix& mat);

	wstring Get_Name() { return wstring(m_szName); }
protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	class CGameInstance*		m_pGameInstance = { nullptr };

protected:
	_tchar										m_szName[MAX_PATH] = {};
	map<const _wstring, class CComponent*>		m_Components;
	class CTransform*							m_pTransformCom = { nullptr };

	_bool m_bDead = {};
	
	_float	m_fViewZ = {};
	_bool m_bCloned = {};

	_bool m_isActive = { true };

	float m_fTimeScale = 1.f; // 오브젝트 별 업데이트 속도
protected:
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);

	HRESULT Replace_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);
	void Remove_Component(const _wstring& strComponentTag);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END