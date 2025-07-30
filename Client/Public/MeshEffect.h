#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CMeshEffect : public CEffectBase
{
public:
	typedef struct tagMeshEffectDesc : public CEffectBase::DESC
	{

	}DESC;

protected:
	CMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshEffect(const CMeshEffect& Prototype);
	virtual ~CMeshEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	CModel*			m_pModelCom = { nullptr };

protected:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CMeshEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize();
	virtual void Deserialize(const json& j);
};

NS_END
