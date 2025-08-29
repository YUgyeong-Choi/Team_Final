#pragma once

#include "Base.h"
#include "PhysX_ContactReport.h"

NS_BEGIN(Engine)

class CPhysX_Manager final : public CBase
{
private:
	typedef struct TriggerStay
	{
		CPhysXActor* pMe;
		CPhysXActor* pOther;
	}TIGGERSTAY_DESC;
private:
	CPhysX_Manager();
	virtual ~CPhysX_Manager() = default;

public:
	HRESULT Initialize();
	void Shutdown();

	PxPhysics* GetPhysics() const { return m_pPhysics; }
	PxScene* Get_Scene() const { return m_pScene; }
	PxControllerManager* Get_ControllerManager() const { return m_pControllerManager; }
	PxMaterial* GetMaterial(const wstring& name);

	void Simulate(float fDeltaTime);

	/* 
	TriangleMesh : RayCast, TriggerShape, �浹����(Static����) �� ����
	ConvexMesh : RayCast, TriggerShape, Overlap, Sweep, DynamicActor, KinematicActor, �浹���� ����
	*/

	// ���� ������Ʈ�� ����� �޽� �ݶ��̴� (PxTriangleMesh) �� ����
	PxTriangleMeshGeometry CookTriangleMesh(const PxVec3* vertices, PxU32 vertexCount, const PxU32* indices, PxU32 triangleCount, PxMeshScale geomScale);

	// ���� ������Ʈ�� ����� �޽� �ݶ��̴� (ConvexMesh)���� - triangle�� �ʹ� ���̵�
	PxConvexMeshGeometry CookConvexMesh(const PxVec3* vertices, PxU32 vertexCount, PxMeshScale geomScale);
	
	// ���� ���������� AABB�� ����� ��
	PxBoxGeometry CookBoxGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float geomScale);

	// ���� AABB�� ����� �ڵ�
	PxBoxGeometry CookBoxGeometry(const PxVec3& halfExtents);

	// ���� ���������� ĸ�� ũ�⸦ �˾Ƽ� �������
	PxCapsuleGeometry CookCapsuleGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float geomScale);

	// ���� ĸ�� ����� �ڵ�
	PxCapsuleGeometry CookCapsuleGeometry(_float fRadius, _float fCapsuleHeight);

	// ���� ���������� Sphere�� ����� ��
	PxSphereGeometry CookSphereGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float fScale);

	// ���� ��ü ����� �ڵ�
	PxSphereGeometry CookSphereGeometry(_float fRadius);

	// ���� �Ž� ����
	//CPhysXStaticActor* Create_Terrain(const PxVec3* pVertices, PxU32 vertexCount, const PxU32* pIndices, PxU32 triangleCount);

	void Insert_TriggerEnterActor(CPhysXActor* pMe, CPhysXActor* pOther);
	void Remove_TriggerExitActor(CPhysXActor* pMe, CPhysXActor* pOther);
	void Remove_TriggerRemoveActor(CPhysXActor* pMe, unordered_set<CPhysXActor*> pTriggerEnterOthers);
	void Update_OnTriggerStay();

	void Remove_OnStayTrigger();
public:
	static CPhysX_Manager* Create();
	virtual void Free() override;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	SpriteBatch* m_pBatch = { nullptr };

private:
	PxFoundation* m_pFoundation = { nullptr };
	PxPhysics* m_pPhysics = { nullptr };
	PxScene* m_pScene = { nullptr };
	PxControllerManager* m_pControllerManager = { nullptr };

	unordered_map<wstring, PxMaterial*> m_Materials;
	PxDefaultCpuDispatcher* m_pDispatcher = { nullptr };

	PxDefaultAllocator m_Allocator;
	PxDefaultErrorCallback m_ErrorCallback;

	CPhysX_ContactReport* m_pContactCallback = { nullptr };
private:
	vector<TIGGERSTAY_DESC> m_ActorsForTriggerStay;
};

NS_END
