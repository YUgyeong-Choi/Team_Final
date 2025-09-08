#pragma once

#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBreakableMesh : public CGameObject
{
public:
	typedef struct tagBreakableMeshDesc : public CGameObject::GAMEOBJECT_DESC
	{
		//본 메쉬
		wstring ModelName = {};
		//파트 메쉬 갯수
		_uint iPartModelCount = { 0 };
		//파트 메쉬 이름들
		vector<wstring> PartModelNames = {};
		vector<_float3> vOffsets = {};
	}BREAKABLEMESH_DESC;

protected:
	CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBreakableMesh(const CBreakableMesh& Prototype);
	virtual ~CBreakableMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Reset() override;

public:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;

private:
	void Break();

private:
	HRESULT Render_Model();
	HRESULT Render_PartModels();

	HRESULT Find_Player();
	void IgnorePlayerCollider(CPhysXDynamicActor* pActor);

private:
	//조각들 초기행렬 저장
	vector<_float4x4> m_PartInitWorldMatrixs = {};

private:
	//파트 모델 갯수
	_uint m_iPartModelCount = 0;

	_bool m_bBreakTriggered = { false }; //무너진다는 트리거
	_bool m_bIsBroken = { false }; //이미 무너진 상태인지 확인하는 코드


	class CPlayer* m_pPlayer = { nullptr };
private:    

	/* [ 컴포넌트 ] */
	vector<CModel*> m_pPartModelComs = {};
	vector<CPhysXDynamicActor*> m_pPartPhysXActorComs = {};
	vector<CTransform*> m_pPartTransformComs = {};

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();
	HRESULT Ready_PartColliders();

public:
	static CBreakableMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END