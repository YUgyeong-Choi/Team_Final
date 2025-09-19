#pragma once

#include "GameObject.h"
#include "Client_Defines.h"
NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CAnimatedProp : public CGameObject
{
public:
	typedef struct tagBossDoorMeshDesc : public CGameObject::GAMEOBJECT_DESC	
	{
		_bool       bCullNone{ false };
		_bool 		bUseSecondMesh{ false };
		const _tchar* szMeshID{ nullptr };
		const _tchar* szSecondMeshID{ nullptr };
		LEVEL			eMeshLevelID{ LEVEL::END };
		_float4x4		vSecondWorldMatrix = {};
	}ANIMTEDPROP_DESC;
protected:
	CAnimatedProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimatedProp(const CAnimatedProp& Prototype);
	virtual ~CAnimatedProp() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Register_Events();

	void NotifyPlayAnimation(_bool bPlay);

protected:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources(_bool bSecondModel);
protected:
	HRESULT LoadFromJson();
	HRESULT LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom);
	HRESULT LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator);

private:

	_bool    m_bDoOnce = false;
	_bool    m_bCullNone = false;
	_bool    m_bUseSecondMesh = false;
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimator = { nullptr };

	CModel* m_pSecondModelCom = { nullptr };
	CGameObject* m_pDummyObject = { nullptr };
	CAnimator* m_pSecondAnimator = { nullptr };
	CTransform* m_pSecondTransformCom = { nullptr };

	const _tchar* m_szMeshID = { nullptr };
	const _tchar* m_szSecondMeshID = { nullptr };
	LEVEL		m_eMeshLevelID = { LEVEL::END };
	
public:
	static CAnimatedProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END