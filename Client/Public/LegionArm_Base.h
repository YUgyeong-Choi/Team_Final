#pragma once

#include "Client_Defines.h"
#include "Weapon.h"

NS_BEGIN(Engine)

class CModel;
class CAnimator;

NS_END


NS_BEGIN(Client)

class CLegionArm_Base abstract : public CWeapon
{
public:
	typedef struct eArmDesc : public WEAPON_DESC
	{

	}ARM_DESC;


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

	// 팔 사용 효과 만들기
	virtual void Activate() {}

	_float& Get_fLegionEnergy() { return m_fLegionArmEnergy; }
	void Use_LegionEnergy(_float energy);

	virtual void Reset();
	


protected:              /* [ 컴포넌트 ], 나중에 필요할거 같아서 일단 넣어만 둠 */
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator = { nullptr };


protected:
	
/* [ 리전 암 내구도 ] */
	_float  m_fLegionArmEnergy;
	_float  m_fMaxLegionArmEnergy;


	

public:
	virtual CGameObject* Clone(void* pArg = nullptr) = 0;
	virtual void Free() override;
};

NS_END