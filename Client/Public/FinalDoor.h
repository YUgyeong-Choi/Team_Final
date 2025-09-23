#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CFinalDoor : public CGameObject
{
protected:
	CFinalDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalDoor(const CFinalDoor& Prototype);
	virtual ~CFinalDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void DoorOpen();
	HRESULT Bind_ShaderResources();
protected:
	HRESULT Ready_Components(void* pArg);
protected: /* [ 초기화 변수 ] */
	const _tchar* m_szMeshID = { nullptr };
	LEVEL			m_eMeshLevelID = { LEVEL::END };
	_bool			m_bDoOnce = {};
protected:
	// render & anim 관련
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CTexture* m_pEmissiveCom = { nullptr };

	class CPlayer* m_pPlayer = { nullptr };

	_bool m_bOpen = false;
public:
	static CFinalDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END