#pragma once

#include "Client_Defines.h"
#include "ParticleEffect.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CMeshEmitter final : public CParticleEffect
{
public:
	typedef struct tagMeshEmitterDesc : public CParticleEffect::DESC
	{
		class CGameObject* pOwner = { nullptr };
	}DESC;

private:
	CMeshEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshEmitter(const CMeshEmitter& Prototype);
	virtual ~CMeshEmitter() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual _float Ready_Death();
	virtual void Pause() { m_isActive = false; }

public:
	void Spawn_Particles();
	//void 

private:
	CModel*			m_pModelCom = { nullptr };


private:
	virtual HRESULT Ready_Components() override;
	virtual HRESULT Bind_ShaderResources() override;

private:
	void Create_CDF();
	_float CalcTriangleArea(const _float3& v0, const _float3& v1, const _float3& v2);

public:
	static CMeshEmitter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize()override;
	virtual void Deserialize(const json& j)override;
};

NS_END
