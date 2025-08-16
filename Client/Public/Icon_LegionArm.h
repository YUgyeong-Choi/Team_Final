#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"


NS_BEGIN(Client)

class CIcon_LegionArm : public CDynamic_UI
{
private:
	CIcon_LegionArm(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CIcon_LegionArm(const CIcon_LegionArm& Prototype);
	virtual ~CIcon_LegionArm() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Update_ICon(_wstring& strTextureTag);

	HRESULT Bind_ShaderResources();


private:


public:
	static CIcon_LegionArm* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END