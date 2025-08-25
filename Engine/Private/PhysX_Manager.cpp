#include "PhysX_Manager.h"
//#include <cuda_runtime.h>
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

	const _bool k0 = PxFilterObjectIsKinematic(attributes0);
	const _bool k1 = PxFilterObjectIsKinematic(attributes1);

	// 트리거 판정
	const bool t0 = PxFilterObjectIsTrigger(attributes0);
	const bool t1 = PxFilterObjectIsTrigger(attributes1);
	const bool isTriggerPair = (t0 || t1);
	if (isTriggerPair)
	{
		// 트리거는 persist 이벤트 지원 안 함 (PhysX 5.x)
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT
			| PxPairFlag::eNOTIFY_TOUCH_FOUND
			| PxPairFlag::eNOTIFY_TOUCH_LOST;
		return PxFilterFlag::eDEFAULT;
	}


	if (k0 && k1) // 둘 다 Kinematic이면 충돌 없음
	{
		pairFlags = PxPairFlag::eDETECT_DISCRETE_CONTACT
			| PxPairFlag::eNOTIFY_TOUCH_FOUND
			| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
			| PxPairFlag::eNOTIFY_TOUCH_LOST
			| PxPairFlag::eNOTIFY_CONTACT_POINTS;
		return PxFilterFlag::eDEFAULT;
	}

	pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
	pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
	pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
	pairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;

	return PxFilterFlag::eDEFAULT;
}

HRESULT CPhysX_Manager::Initialize()
{
	// 1. Foundation 생성
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
	if (!m_pFoundation)
		return E_FAIL;

	// 2. CUDA Context Manager 생성 (GPU 연동용)
	//PxCudaContextManagerDesc cudaDesc;
	//cudaDesc.graphicsDevice = m_pDevice; // DirectX11 디바이스 연결
	//m_pCudaContextManager = PxCreateCudaContextManager(*m_pFoundation, cudaDesc);
	//if (!m_pCudaContextManager || !m_pCudaContextManager->contextIsValid())
	//{
	//	OutputDebugStringA(" PhysX GPU 사용 불가: CUDA Context 유효하지 않음\n");
	//	if (m_pCudaContextManager)
	//	{
	//		m_pCudaContextManager->release(); 
	//		m_pCudaContextManager = nullptr;
	//	}
	//}
	//PrintCudaDeviceInfo();

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

	// GPU dispatcher와 GPU dynamics 활성화
	//if (m_pCudaContextManager && m_pCudaContextManager->contextIsValid())
	//{
	//	sceneDesc.cudaContextManager = m_pCudaContextManager;
	//	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	//	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	//}

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


	// 컨트롤러 매니저 추가
	m_pControllerManager = PxCreateControllerManager(*m_pScene);
	if (!m_pControllerManager)
		return E_FAIL;

	return S_OK;
}

void CPhysX_Manager::Shutdown()
{
	if (m_pControllerManager)
	{
		m_pControllerManager->purgeControllers(); // 컨트롤러 모두 정리
		m_pControllerManager->release();
		m_pControllerManager = nullptr;
	}

	if (m_pScene)
	{
		m_pScene->release();
		m_pScene = nullptr;
	}

	for (auto& material : m_Materials) {
		material.second->release();
		material.second = nullptr;
	}

	//if (m_pCudaContextManager)
	//{
	//	m_pCudaContextManager->release();  
	//	m_pCudaContextManager = nullptr;
	//}

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
	m_pScene->simulate(fDeltaTime);
	m_pScene->fetchResults(true);
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

PxBoxGeometry CPhysX_Manager::CookBoxGeometry(const PxVec3& halfExtents)
{
	return PxBoxGeometry(halfExtents);
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

PxSphereGeometry CPhysX_Manager::CookSphereGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float fScale)
{
	if (pVertices == nullptr || vertexCount == 0)
		return PxSphereGeometry();

	// 1. AABB 계산
	PxVec3 vMin = pVertices[0];
	PxVec3 vMax = pVertices[0];

	for (PxU32 i = 1; i < vertexCount; ++i)
	{
		vMin = vMin.minimum(pVertices[i]);
		vMax = vMax.maximum(pVertices[i]);
	}

	// 2. 중심 계산 (AABB 중심 사용)
	PxVec3 vCenter = (vMin + vMax) * 0.5f;

	// 3. 반지름 계산 (가장 먼 점으로부터의 거리)
	float fMaxDistSq = 0.f;
	for (PxU32 i = 0; i < vertexCount; ++i)
	{
		PxVec3 vDiff = pVertices[i] - vCenter;
		float distSq = vDiff.magnitudeSquared(); // 거리 제곱
		if (distSq > fMaxDistSq)
			fMaxDistSq = distSq;
	}

	float radius = sqrtf(fMaxDistSq) * fScale;

	// 4. 구체 지오메트리 생성
	return PxSphereGeometry(radius);
}

PxSphereGeometry CPhysX_Manager::CookSphereGeometry(_float fRadius)
{
	return PxSphereGeometry(fRadius);
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

//
//void CPhysX_Manager::PrintCudaDeviceInfo()
//{
//	if (m_pCudaContextManager && m_pCudaContextManager->contextIsValid())
//	{
//		printf(" CUDA Context Valid\n");
//
//		int deviceCount = 0;
//		cudaError_t err = cudaGetDeviceCount(&deviceCount);
//		if (err != cudaSuccess)
//		{
//			printf(" cudaGetDeviceCount failed\n");
//			printf(cudaGetErrorString(err));
//			return;
//		}
//
//		if (deviceCount == 0)
//		{
//			printf(" No CUDA devices found\n");
//			return;
//		}
//
//		cudaDeviceProp prop;
//		err = cudaGetDeviceProperties(&prop, 0); // 첫 번째 디바이스 정보 조회
//		if (err != cudaSuccess)
//		{
//			printf(" cudaGetDeviceProperties failed\n");
//			printf(cudaGetErrorString(err));
//			return;
//		}
//
//		printf(" CUDA Device: ");
//		printf(prop.name);
//		printf("\n");
//	}
//	else
//	{
//		printf(" CUDA Context Invalid\n");
//	}
//}

void CPhysX_Manager::Insert_TriggerEnterActor(CPhysXActor* pMe, CPhysXActor* pOther)
{
	TIGGERSTAY_DESC desc;
	desc.pMe = pMe;
	desc.pOther = pOther;
	m_ActorsForTriggerStay.push_back(desc);
}

void CPhysX_Manager::Remove_TriggerExitActor(CPhysXActor* pMe, CPhysXActor* pOther)
{
	if (pMe->Get_Owner()->Get_bDead())
		return;

	if (pOther->Get_Owner()->Get_bDead())
		return;

	for (auto it = m_ActorsForTriggerStay.begin(); it != m_ActorsForTriggerStay.end(); )
	{
		if (it->pMe == pMe && it->pOther == pOther)
			it = m_ActorsForTriggerStay.erase(it); 
		else
			++it;
	}
}

void CPhysX_Manager::Remove_TriggerRemoveActor(CPhysXActor* pMe, unordered_set<CPhysXActor*> pTriggerEnterOthers)
{
	for (auto it = m_ActorsForTriggerStay.begin(); it != m_ActorsForTriggerStay.end(); )
	{
		bool eraseMe = false;
		if (it->pMe == pMe)
			eraseMe = (pTriggerEnterOthers.find(it->pOther) != pTriggerEnterOthers.end());
		else if (it->pOther == pMe)
			eraseMe = (pTriggerEnterOthers.find(it->pMe) != pTriggerEnterOthers.end());

		if (eraseMe)
			it = m_ActorsForTriggerStay.erase(it);
		else
			++it;
	}
}

void CPhysX_Manager::Update_OnTriggerStay()
{
	for (auto& pair : m_ActorsForTriggerStay)
	{
		CPhysXActor* pMe = pair.pMe;
		CPhysXActor* pOther = pair.pOther;

		if (pMe && pOther && pOther->Get_Owner())
		{
			pOther->Get_Owner()->On_TriggerStay(pMe->Get_Owner(), pMe->Get_ColliderType());
		}
	}
}

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