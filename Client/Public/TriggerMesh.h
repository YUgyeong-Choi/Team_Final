#pragma once
/* [ 바닥에 있는 아이템들 같은 거 ] */

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
NS_END

NS_BEGIN(Client)

class CTriggerMesh : public CTriggerBox
{
public:
	typedef struct tagTriggerMeshDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		const _tchar* szMeshID;
		LEVEL			m_eMeshLevelID;
		_tchar		szModelPrototypeTag[MAX_PATH] = { 0 };
	}TRIGGERMESH_DESC;

protected:
	CTriggerMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerMesh(const CTriggerMesh& Prototype);
	virtual ~CTriggerMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	const _tchar*	m_szMeshFullID = { nullptr };
	LEVEL			m_eMeshLevelID = { LEVEL::END };
protected:

	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };
	CTexture*		m_pEmissiveCom = { nullptr };
	CPhysXStaticActor* m_pPhysXActorCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();

public:
	static CTriggerMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END