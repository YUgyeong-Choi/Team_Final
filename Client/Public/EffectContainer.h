#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CEffectContainer final : public CGameObject
{
public:
	typedef struct tagEffectContainerDesc : public CGameObject::GAMEOBJECT_DESC
	{
		json				j;
		_float4x4			PresetMatrix = {};
		const _float4x4*	pSocketMatrix = { nullptr };         // 우선적으로 사용되는 부모 행렬 (e.g. 뼈소켓매트릭스, 혹은 단순히 부모 행렬 - 이 경우 웬만하면 EC의 트랜스폼을 조정하시오 - )
		const _float4x4*	pParentMatrix = { nullptr }; // 위의 행렬이 별개로 부모 행렬을 또 들어야 하는 경우 사용할 것.. (뼈 소켓일 경우 -> 부모 객체의 월드 매트릭스)

	}DESC;

private:
	CEffectContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffectContainer(const CEffectContainer& Prototype);
	virtual ~CEffectContainer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Set_Loop(_bool bLoop) { m_bLoop = bLoop; }
	void End_Effect();

private:
	vector<class CEffectBase*>        m_Effects;

	// 시퀀스 재생용
	_float		m_fCurFrame = {};
	_int		m_iCurFrame = {};
	_int		m_iFirstFrame = { 0 };
	_float		m_fTickPerSecond = { 60.f };

	_float		m_fTimeAcc = {};

	_int		m_iMaxFrame = {};
	_bool		m_bLoop = { true };
	_float		m_fLifeTimeAcc = {};
	_float		m_fLifeTime = {};

private:
	_float				m_fFrame = { };
	_bool				m_bReadyDeath = { false };
	_float				m_fDeadInterval = { 0.f };
	_float				m_fDeadTimeAcc = {};

	const _float4x4*	m_pSocketMatrix = { nullptr };
	const _float4x4*	m_pParentMatrix = { nullptr };
	_float4x4			m_CombinedWorldMatrix = {};

private:
	//HRESULT Load_JsonFiles(const _wstring strJsonFilePath); //쓰지마
	HRESULT Load_JsonFiles(const json& j);
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	//HRESULT Add_Effect(class CEffectBase* pEffect);

public:
	static CEffectContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END