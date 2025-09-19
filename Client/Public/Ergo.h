#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CErgo : public CGameObject
{
protected:
	CErgo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CErgo(const CErgo& Prototype);
	virtual ~CErgo() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Find_Lamp();

	//플레이어한테, 흡수 되어지다.
	void Absorbed();

private:
	//_float4x4 m_StartHorizontalMatrix = {};
	//_float4x4 m_HorizontalMatrix = {};
	_float4x4 m_StartVerticalMatrix = {};
	_float4x4 m_VerticalMatrix = {};
private:
	_bool m_bAbsorbed = { false };
	_bool m_bReadyDeath = { false };
	_float m_fDeathInterval = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	//플레이어 램프
	class CPlayerLamp* m_pLamp = { nullptr };

	//램프의 월드 행렬
	const _float4x4* m_pLampMatrix = { nullptr };
	
	enum class EFFECTS {/* HORIZONTAL,*/ VERTICAL, END };
	class CSwordTrailEffect* m_pTrailEffect[ENUM_CLASS(EFFECTS::END)] = {nullptr};

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Effect();
public:
	static CErgo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END