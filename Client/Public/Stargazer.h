#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CStargazer : public CGameObject
{
public:
	typedef struct tagStargazerDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}STARGAZER_DESC;

protected:
	CStargazer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStargazer(const CStargazer& Prototype);
	virtual ~CStargazer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void LoadAnimDataFromJson(CModel* pModel, CAnimator* pAnimator);
	void Find_Player();

private:
	enum class STARGAZER_STATE { DESTROYED, FUNCTIONAL, END };

private:
	STARGAZER_STATE m_eState = { STARGAZER_STATE::END };

private:
	class CPlayer* m_pPlayer = { nullptr };

private:    /* [ ÄÄÆ÷³ÍÆ® ] */
	CModel* m_pModelCom[ENUM_CLASS(STARGAZER_STATE::END)] = {nullptr};
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator[ENUM_CLASS(STARGAZER_STATE::END)] = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();

public:
	static CStargazer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END