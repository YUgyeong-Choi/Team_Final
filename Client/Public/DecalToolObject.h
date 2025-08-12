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
	friend class CDecalTool;

public:
	typedef struct tagDecalToolObjectDesc : public DECAL_DESC
	{
		wstring FilePath[ENUM_CLASS(TEXTURE_TYPE::END)] = {};
	}DECALTOOLOBJECT_DESC;

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

public:
	HRESULT Set_Texture(TEXTURE_TYPE eType, string TexturePath, string FileName);

private:
	wstring m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::END)] = {};
	wstring m_FilePath[ENUM_CLASS(TEXTURE_TYPE::END)] = {};

private:
	virtual HRESULT Ready_Components(void* Arg) override;
	HRESULT Bind_ShaderResources();


public:
	static CDecalToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END