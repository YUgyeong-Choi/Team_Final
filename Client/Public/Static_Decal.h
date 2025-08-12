#pragma once
#include "Decal.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_VolumeMesh;
NS_END

NS_BEGIN(Client)

class CStatic_Decal final : public CDecal
{
private:
	CStatic_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStatic_Decal(const CStatic_Decal& Prototype);
	virtual ~CStatic_Decal() = default;

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
	static CStatic_Decal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END