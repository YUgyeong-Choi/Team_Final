#pragma once

#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CSoundController;
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

		//영향을 줄 네브메쉬 이름
		wstring wsNavName = {};

		//푸오코 보스 기둥만 매커니즘이 좀 달라서 이렇게 처리해버려야겠다. 새로운 클래스 파기 너무 번거로울 듯
		_bool bFireEaterBossPipe = { false }
		;
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
public:
	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	//virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	//virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
private:
	void Break();

private:
	HRESULT Render_Model();
	HRESULT Render_PartModels();

	HRESULT Find_Player();
	void IgnorePlayerCollider(CPhysXDynamicActor* pActor);

private:
	void Invisible();


private:
	//한 점이 xz 평면상에서 AABB에 포함되어있는지
	bool IsPointInsideXZ(const PxVec3& point, const PxBounds3& bounds);

	//자신의 aabb 만큼의 네브 인덱스를 가져온다.
	void Store_NavIndices();

	void Set_Active_StoreCells(_bool bActive);

private:
	//모델 이름
	wstring m_wsModelName = {};

private:
	//자신이 가지고 있는 네비 셀 인덱스들, 자신이 부서지면 활성화 시킨다.
	vector<_int> m_NavIndices = {};

private:
	//조각들 초기행렬 저장
	vector<_float4x4> m_PartInitWorldMatrixs = {};

private:
	//파트 모델 갯수
	_uint m_iPartModelCount = 0;

private:
	_bool m_bBreakTriggered = { false }; //무너진다는 트리거
	_bool m_bIsBroken = { false }; //이미 무너진 상태인지 확인하는 코드

private:
	//몇초뒤 렌더링과, 콜라이더도 빼주자
	const _float m_fTime_Invisible = { 8.f };
	_float m_fTimeAcc = { 0.f };
	_bool m_bInvisible = { false };

private:
	class CPlayer* m_pPlayer = { nullptr };

private:
	//푸오코 보스 기둥만 매커니즘이 좀 달라서 이렇게 처리해버려야겠다. 새로운 클래스 파기 너무 번거로울 듯
	_bool m_bFireEaterBossPipe = { false };

private:    

	/* [ 컴포넌트 ] */
	vector<CModel*> m_pPartModelComs = {};
	vector<CPhysXDynamicActor*> m_pPartPhysXActorComs = {};
	vector<CTransform*> m_pPartTransformComs = {};

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	//영향을 줄 네비게이션
	CNavigation* m_pNaviCom = { nullptr };

	//사운드
	CSoundController* m_pSoundCom = { nullptr };

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