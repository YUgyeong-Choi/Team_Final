#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CVIBuffer_SwordTrail;
NS_END

NS_BEGIN(Client)

class CSwordTrailEffect : public CEffectBase
{
public:
	typedef struct tagTrailEffectDesc : public CEffectBase::DESC
	{
		_float4x4* pParentCombinedMatrix = { nullptr }; // 부모 모델의 월드 매트릭스
		_float4x4* pInnerSocketMatrix = { nullptr };
		_float4x4* pOuterSocketMatrix = { nullptr };
	}DESC;

protected:
	CSwordTrailEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSwordTrailEffect(const CSwordTrailEffect& Prototype);
	virtual ~CSwordTrailEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void Set_TrailActive(_bool bActive);
	void Release_Matrices() {
		m_pParentCombinedMatrix = { nullptr };
		m_pInnerSocketMatrix = { nullptr };
		m_pOuterSocketMatrix = { nullptr };
	}
	
protected:
	CVIBuffer_SwordTrail*		m_pVIBufferCom = { nullptr };
	_float4x4*					m_pParentCombinedMatrix = { nullptr };
	_float4x4*					m_pInnerSocketMatrix	= { nullptr };
	_float4x4*					m_pOuterSocketMatrix	= { nullptr };

	_float3						m_vInnerPos = { 0.f, 0.f, 0.f }; // 안쪽 위치
	_float3						m_vOuterPos = { 0.f, 0.f, 0.f }; // 바깥쪽 위치

	_bool						m_bTrailActive = true;
	_wstring					m_strBufferTag;

protected:
	virtual HRESULT Ready_Components() override;
	virtual HRESULT Bind_ShaderResources() override;

public:
	static CSwordTrailEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize()override;
	virtual void Deserialize(const json& j)override;
};

NS_END