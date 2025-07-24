#include "PhysX_Manager.h"

CPhysX_Manager::CPhysX_Manager()
{
}

static PxFilterFlags CustomFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{

	if ((filterData0.word0 & filterData1.word1) == 0 &&
		(filterData1.word0 & filterData0.word1) == 0)
		return PxFilterFlag::eKILL; 

	pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
	pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
	pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;

	return PxFilterFlag::eDEFAULT;
}

HRESULT CPhysX_Manager::Initialize()
{
	// 1. Foundation 생성
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
	if (!m_pFoundation)
		return E_FAIL;

	// 3. Physics 객체 생성
	PxTolerancesScale scale;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, scale, true, nullptr);
	if (!m_pPhysics)
		return E_FAIL;

	// 4. Scene 생성
	PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	m_pDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	// 충돌 관련 콜백 함수 설정
	m_pContactCallback = new CPhysX_ContactReport();
	sceneDesc.simulationEventCallback = m_pContactCallback;
	sceneDesc.filterShader = CustomFilterShader;

	sceneDesc.kineKineFilteringMode = PxPairFilteringMode::eKEEP;
	sceneDesc.staticKineFilteringMode = PxPairFilteringMode::eKEEP;

	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;

	m_pScene = m_pPhysics->createScene(sceneDesc);
	if (!m_pScene)
		return E_FAIL;

	// 5. 기본 머티리얼
	PxMaterial* pMaterial = m_pPhysics->createMaterial(0.6f, 0.6f, 0.05f); //정적 마찰, 동적 마찰, 탄성 계수
	if (!pMaterial)
		return E_FAIL;
	m_Materials.emplace(make_pair(L"Default", pMaterial));

	return S_OK;
}

void CPhysX_Manager::Shutdown()
{
	if (m_pScene)
	{
		m_pScene->release();
		m_pScene = nullptr;
	}

	for (auto& material : m_Materials) {
		material.second->release();
		material.second = nullptr;
	}



	if (m_pDispatcher)
	{
		m_pDispatcher->release(); 
		m_pDispatcher = nullptr;
	}

	if (m_pPhysics)
	{
		m_pPhysics->release();
		m_pPhysics = nullptr;
	}

	if (m_pFoundation)
	{
		m_pFoundation->release();
		m_pFoundation = nullptr;
	}
}

PxMaterial* CPhysX_Manager::GetMaterial(const wstring& name)
{
	auto iter = m_Materials.find(name);
	if (iter != m_Materials.end()) {
		return iter->second;
	}
	else {
		return nullptr;
	}

}

void CPhysX_Manager::Simulate(float fDeltaTime)
{
	if (m_pScene)
	{
		m_pScene->simulate(fDeltaTime);
		m_pScene->fetchResults(true);
	}
}

PxTriangleMeshGeometry CPhysX_Manager::CookTriangleMesh(const PxVec3* vertices, PxU32 vertexCount, const PxU32* indices, PxU32 triangleCount, PxMeshScale geomScale)
{
	PxTolerancesScale scale = m_pPhysics->getTolerancesScale();
	PxCookingParams cookingParams(scale);
	cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = vertexCount;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = vertices;

	meshDesc.triangles.count = triangleCount;
	meshDesc.triangles.stride = sizeof(PxU32) * 3;
	meshDesc.triangles.data = indices;

	PxDefaultMemoryOutputStream writeBuffer;
	if (!PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer))
		return PxTriangleMeshGeometry();

	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

	PxTriangleMesh* pMesh = m_pPhysics->createTriangleMesh(readBuffer);
	PxTriangleMeshGeometry geom(pMesh, geomScale);
	 
	return geom;
}

PxConvexMeshGeometry CPhysX_Manager::CookConvexMesh(const PxVec3* vertices, PxU32 vertexCount, PxMeshScale geomScale)
{
    PxTolerancesScale scale = m_pPhysics->getTolerancesScale();
    PxCookingParams cookingParams(scale);
	cookingParams.meshPreprocessParams = PxMeshPreprocessingFlags(
		PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH |
		PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
	cookingParams.areaTestEpsilon = 0.0001f; // 더 작게 하면 더 세밀한 면 허용
	cookingParams.planeTolerance = 0.0001f; // 동일평면 간주 기준 낮추기
	cookingParams.gaussMapLimit = 256; // 너무 낮으면 퀄리티 떨어짐

    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = vertexCount;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = vertices;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX; // 자동 볼록 껍질

    PxDefaultMemoryOutputStream writeBuffer;
    if (!PxCookConvexMesh(cookingParams, convexDesc, writeBuffer))
        return nullptr;

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	PxConvexMesh* pMesh = m_pPhysics->createConvexMesh(readBuffer);

	PxConvexMeshGeometry geom(pMesh, geomScale);
	return geom;
}

PxBoxGeometry CPhysX_Manager::CookBoxGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float fScale)
{
	if (pVertices == nullptr || vertexCount == 0)
		return PxBoxGeometry();

	// 1. AABB 계산
	PxVec3 vMin = pVertices[0];
	PxVec3 vMax = pVertices[0];

	for (PxU32 i = 1; i < vertexCount; ++i)
	{
		vMin = vMin.minimum(pVertices[i]);
		vMax = vMax.maximum(pVertices[i]);
	}

	// 2. Extents 계산
	PxVec3 extents = (vMax - vMin) * 0.5f * fScale;
	PxBoxGeometry boxGeom(extents);
	return boxGeom;
}

PxCapsuleGeometry CPhysX_Manager::CookCapsuleGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float geomScale)
{
	if (pVertices == nullptr || vertexCount == 0)
		return PxCapsuleGeometry();

	// 1. AABB 계산
	PxVec3 vMin = pVertices[0];
	PxVec3 vMax = pVertices[0];

	for (PxU32 i = 1; i < vertexCount; ++i)
	{
		vMin = vMin.minimum(pVertices[i]);
		vMax = vMax.maximum(pVertices[i]);
	}

	// 2. 캡슐 축 방향은 Y축 기준 (수직 캡슐)
	_float height = (vMax.y - vMin.y) * geomScale;

	// 3. 반지름 계산 (X/Z방향 평균)
	_float radiusX = (vMax.x - vMin.x) * 0.5f * geomScale;
	_float radiusZ = (vMax.z - vMin.z) * 0.5f * geomScale;
	_float radius = max(radiusX, radiusZ); // 또는 평균도 가능

	// 4. 캡슐의 height는 양 끝 구체를 제외한 실린더 부분의 길이
	// 따라서 실제 물리 height = 전체 길이 - 2 * radius
	_float capsuleHeight = max(0.f, height - 2.f * radius);

	return PxCapsuleGeometry(radius, capsuleHeight);
}

PxCapsuleGeometry CPhysX_Manager::CookCapsuleGeometry(_float fRadius, _float fCapsuleHeight)
{
	return PxCapsuleGeometry(fRadius, fCapsuleHeight);
}

//CPhysXStaticActor* CPhysX_Manager::Create_Terrain(const PxVec3* pVertices, PxU32 vertexCount, const PxU32* pIndices, PxU32 triangleCount)
//{
//	PxTriangleMesh* pMesh = CookTriangleMesh(m_pPhysics, pVertices, vertexCount, pIndices, triangleCount);
//	if (!pMesh)
//		return nullptr;
//
//	PxTriangleMeshGeometry geom(pMesh);
//	PxTransform pose = PxTransform(PxVec3(0.f)); // 기본 월드 위치
//
//	CPhysXStaticActor* pActor = CPhysXStaticActor::Create(m_pPhysics, geom, pose, m_pDefaultMaterial, WorldFilter::STATIC);
//	if (m_pScene && pActor)
//		m_pScene->addActor(*pActor->Get_Actor());
//
//	return pActor;
//}

CPhysX_Manager* CPhysX_Manager::Create()
{
	CPhysX_Manager* pInstance = new CPhysX_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create: CPhysX_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPhysX_Manager::Free()
{
	Safe_Delete(m_pContactCallback);

	__super::Free();
}