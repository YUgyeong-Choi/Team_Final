#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CEffectContainer final : public CGameObject
{
public:
	typedef struct tagEffectContainerDesc : public CGameObject::GAMEOBJECT_DESC
	{
		json		j;
		_float3		vPresetPosition = { 0.f,0.f,0.f };
		_float4x4*	pSocketMatrix = { nullptr }; // 소켓 매트릭스
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
	const _float4x4*	m_pSocketMatrix = { nullptr };


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