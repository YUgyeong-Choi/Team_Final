#include "PreviewObject.h"

#include "GameInstance.h"

CPreviewObject::CPreviewObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CPreviewObject::CPreviewObject(const CPreviewObject& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CPreviewObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPreviewObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;


	if (FAILED(Ready_DepthStencilView(g_iWinSizeX, g_iWinSizeY)))
		return E_FAIL;
	

	//미리보기용 렌더타겟
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Preview"), static_cast<_uint>(g_iWinSizeX), static_cast<_uint>(g_iWinSizeY), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.f, 0.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Preview"), TEXT("Target_Preview"))))
		return E_FAIL;

	m_pCameraTransformCom->Set_RotationPerSec(1.f);
	m_pCameraTransformCom->Set_SpeedPerSec(1.f);

	m_pTransformCom->Set_RotationPerSec(3.f);

	Reset_CameraWorldMatrix();

	return S_OK;
}

void CPreviewObject::Priority_Update(_float fTimeDelta)
{

}

void CPreviewObject::Update(_float fTimeDelta)
{
	if(m_bRotate)
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);

}

void CPreviewObject::Late_Update(_float fTimeDelta)
{
	if (m_pModelCom == nullptr)
		return;

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CPreviewObject::Render()
{
	//중간에 프리뷰용 타겟 변경
	m_pGameInstance->End_MRT();

	m_pGameInstance->Begin_MRT(TEXT("MRT_Preview"), m_pDSV, true, true);

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);

		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		m_pShaderCom->Begin(3);

		m_pModelCom->Render(i);
	}

	m_pGameInstance->End_MRT();

	//타겟 복원
	m_pGameInstance->Begin_MRT(TEXT("MRT_GameObjects"), nullptr, false);

//#ifdef _DEBUG
//	if (m_pGameInstance->Get_RenderCollider()) 
//	{
//		m_pGameInstance->Add_DebugComponent(m_pPhysXActorTriangleCom);
//		m_pGameInstance->Add_DebugComponent(m_pPhysXActorConvexCom);
//	}
//#endif

	return S_OK;
}

void CPreviewObject::Reset_CameraWorldMatrix()
{
	//_float fHeight = 10.f;

	_vector vEye = XMVectorSet(0.f, 0.f, 10.f, 1.f);
	_vector vAt = XMVectorSet(0.f, 0.f, 0.f, 1.f); // 고정 타겟
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);
	m_pCameraTransformCom->Set_WorldMatrix(matView);
}

HRESULT CPreviewObject::Change_Model(wstring ModelPrototypeTag)
{
	//현재 들고 있는 모델을 바꾼다.
	m_ModelPrototypeTag = ModelPrototypeTag;

	Safe_Release(m_pModelCom);

	//컴포넌트 리무브
	Remove_Component(TEXT("Com_Model"));

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), m_ModelPrototypeTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPreviewObject::Ready_Components(void* pArg)
{

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_CameraTransformCom */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Transform"),
		TEXT("Com_CameraTransformCom"), reinterpret_cast<CComponent**>(&m_pCameraTransformCom))))
		return E_FAIL;


	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX1"), reinterpret_cast<CComponent**>(&m_pPhysXActorTriangleCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorConvexCom))))
		return E_FAIL;
	

	return S_OK;
}

HRESULT CPreviewObject::Ready_Collider()
{
	if (m_pModelCom)
	{
		// 피오나 몸체가 2번째 메쉬라서
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(0);
		_uint numIndices = m_pModelCom->Get_Mesh_NumIndices(0);

		vector<PxVec3> physxVertices;
		physxVertices.reserve(numVertices);

		const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(0);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// 2. 인덱스 복사
		const _uint* pIndices = m_pModelCom->Get_Mesh_pIndices(0);
		vector<PxU32> physxIndices;
		physxIndices.reserve(numIndices);

		for (_uint i = 0; i < numIndices; ++i)
			physxIndices.push_back(static_cast<PxU32>(pIndices[i]));

		// 3. Transform에서 S, R, T 분리
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

		// 3-1. 스케일, 회전, 위치 변환
		PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
		PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

		PxTransform pose(positionVec, rotationQuat);
		PxMeshScale meshScale(scaleVec);

		PxTriangleMeshGeometry  geom = m_pGameInstance->CookTriangleMesh(physxVertices.data(), numVertices, physxIndices.data(), numIndices / 3, meshScale);
		m_pPhysXActorTriangleCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorTriangleCom->Set_ShapeFlag(true, false, true);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorTriangleCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorTriangleCom->Set_QueryFilterData(filterData);
		m_pPhysXActorTriangleCom->Set_Owner(this);
		m_pPhysXActorTriangleCom->Set_ColliderType(COLLIDERTYPE::B);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorTriangleCom->Get_Actor());



		PxConvexMeshGeometry  geom2 = m_pGameInstance->CookConvexMesh(physxVertices.data(), numVertices, meshScale);
		m_pPhysXActorConvexCom->Create_Collision(m_pGameInstance->GetPhysics(), geom2, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorConvexCom->Set_ShapeFlag(true, false, true);

		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorConvexCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorConvexCom->Set_QueryFilterData(filterData);
		m_pPhysXActorConvexCom->Set_Owner(this);
		m_pPhysXActorConvexCom->Set_ColliderType(COLLIDERTYPE::B);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorConvexCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

HRESULT CPreviewObject::Ready_DepthStencilView(_uint iWidth, _uint iHeight)
{
	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	/* 깊이 버퍼의 픽셀은 백버퍼의 픽셀과 갯수가 동일해야만 깊이 텍스트가 가능해진다. */
	/* 픽셀의 수가 다르면 아에 렌더링을 못함. */
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;


	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);

	return S_OK;
}

HRESULT CPreviewObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	_float4x4 CameraWorldInverse = {};
	XMStoreFloat4x4(&CameraWorldInverse, m_pCameraTransformCom->Get_WorldMatrix_Inverse());

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &CameraWorldInverse)))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

CPreviewObject* CPreviewObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPreviewObject* pInstance = new CPreviewObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPreviewObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPreviewObject::Clone(void* pArg)
{
	CPreviewObject* pInstance = new CPreviewObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPreviewObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPreviewObject::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pCameraTransformCom);
	Safe_Release(m_pDSV);

	Safe_Release(m_pPhysXActorConvexCom);
	Safe_Release(m_pPhysXActorTriangleCom);

	if (m_bCloned)
	{
		m_pGameInstance->Delete_RenderTarget(TEXT("Target_Preview"));
		m_pGameInstance->Delete_MRT(TEXT("MRT_Preview"));
	}

	//if (nullptr != m_pGameInstance->Find_RenderTarget(TEXT("Target_Preview")))
	//{
	//	m_pGameInstance->Delete_RenderTarget(TEXT("Target_Preview"));
	//	m_pGameInstance->Delete_MRT(TEXT("MRT_Preview"));
	//}
}
