#pragma once
#include "Decal.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_VolumeMesh;
NS_END

NS_BEGIN(Client)

class CDecalToolObject final : public CDecal
{
private:
	CDecalToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecalToolObject(const CDecalToolObject& Prototype);
	virtual ~CDecalToolObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;

private:
	virtual HRESULT Ready_Components() override;
	HRESULT Bind_ShaderResources();

public:
	static CDecalToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END