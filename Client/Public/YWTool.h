#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

NS_BEGIN(Client)

class CYWTool abstract : public CGameObject
{
protected:
	CYWTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYWTool(const CYWTool& Prototype);
	virtual ~CYWTool() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render() override;
	virtual HRESULT	Render_ImGui();
	virtual HRESULT Save(const _char* Map); //void Arg·Î ¹Ù²ÜÁö´Â ¾ÆÁ÷ °í¹Î
	virtual HRESULT Load(const _char* Map);
public:
	virtual void Free() override;

};

NS_END