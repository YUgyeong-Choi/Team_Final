#pragma once

#include "Base.h"
#include "PhysX_ContactReport.h"

NS_BEGIN(Engine)

class CPhysX_Manager final : public CBase
{
private:
	CPhysX_Manager();
	virtual ~CPhysX_Manager() = default;

public:
	HRESULT Initialize();
	void Shutdown();

	PxPhysics* GetPhysics() const { return m_pPhysics; }
	PxScene* GetScene() const { return m_pScene; }
	PxMaterial* GetDefaultMaterial() const { return m_pDefaultMaterial; }

	void Simulate(float fDeltaTime);

	/* 
	TriangleMesh : RayCast, TriggerShape, �浹����(Static����) �� ����
	ConvexMesh : RayCast, TriggerShape, Overlap, Sweep, DynamicActor, KinematicActor, �浹���� ����
	*/

	// ���� ������Ʈ�� ����� �޽� �ݶ��̴� (PxTriangleMesh) �� ����
	PxTriangleMeshGeometry CookTriangleMesh(const PxVec3* vertices, PxU32 vertexCount, const PxU32* indices, PxU32 triangleCount, PxMeshScale geomScale);

	// ���� ������Ʈ�� ����� �޽� �ݶ��̴� (ConvexMesh)���� - triangle�� �ʹ� ���̵�
	PxConvexMeshGeometry CookConvexMesh(const PxVec3* vertices, PxU32 vertexCount, PxMeshScale geomScale);
	
	PxBoxGeometry CookBoxGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float geomScale);

	// ���� �Ž� ����
	//CPhysXStaticActor* Create_Terrain(const PxVec3* pVertices, PxU32 vertexCount, const PxU32* pIndices, PxU32 triangleCount);

public:
	static CPhysX_Manager* Create();
	virtual void Free() override;

private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;
	SpriteBatch* m_pBatch = nullptr;

private:
	PxFoundation* m_pFoundation = nullptr;
	PxPhysics* m_pPhysics = nullptr;
	PxScene* m_pScene = nullptr;
	PxMaterial* m_pDefaultMaterial = nullptr;

	PxDefaultAllocator m_Allocator;
	PxDefaultErrorCallback m_ErrorCallback;

	CPhysX_ContactReport* m_pContactCallback = nullptr;

};

NS_END
