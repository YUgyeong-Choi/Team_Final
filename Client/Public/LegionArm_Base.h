#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)

class CModel;
class CAnimator;

NS_END


NS_BEGIN(Client)

class CLegionArm_Base abstract : public CGameObject
{
public:
	typedef struct eArmDesc : public GAMEOBJECT_DESC
	{
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };

		const _float4x4* pSocketMatrix = { nullptr };
		const _float4x4* pParentWorldMatrix = { nullptr };


	}ARM_DESC;

	_wstring Get_Prototag() { return m_strPrototag; }

protected:
	CLegionArm_Base(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLegionArm_Base(const CLegionArm_Base& Prototype);
	virtual ~CLegionArm_Base() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	// �� ��� ȿ�� �����
	virtual void Activate() {}

protected:              /* [ ������Ʈ ] */
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator = { nullptr };

protected:
	const _float4x4* m_pParentWorldMatrix = { nullptr };
	const _float4x4* m_pSocketMatrix = { nullptr };
	_float4x4			m_CombinedWorldMatrix = {};

protected:
	_wstring m_strPrototag = {};
	
/* [ ���� �� ������ ] */
	_float  m_fLegionArmEnergy;
	_float  m_fMaxLegionArmEnergy;


	

public:
	virtual CGameObject* Clone(void* pArg = nullptr) = 0;
	virtual void Free() override;
};

NS_END