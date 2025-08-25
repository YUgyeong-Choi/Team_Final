#include "Navigation.h"

#include "Cell.h"
#include "GameInstance.h"

_float4x4		CNavigation::m_WorldMatrix = {};

CNavigation::CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }		
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

CNavigation::CNavigation(const CNavigation& Prototype)
	: CComponent ( Prototype )
	, m_Cells { Prototype.m_Cells }
	, m_iIndex { Prototype.m_iIndex } 	
#ifdef _DEBUG
	, m_pShader { Prototype.m_pShader }
#endif
{	 

	for (auto& pCell : m_Cells)
		Safe_AddRef(pCell);

#ifdef _DEBUG
	Safe_AddRef(m_pShader);
#endif

}


HRESULT CNavigation::Initialize_Prototype(const _tchar* pNavigationDataFile)
{
	/*_ulong	dwByte = {};
	HANDLE	hFile = CreateFile(pNavigationDataFile, GENERIC_READ, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (0 == hFile)
		return E_FAIL;

	while (true)
	{
		_float3		vPoints[3] = {};

		ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);

		if (0 == dwByte)
			break;

		CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, static_cast<_int>(m_Cells.size()));
		if (nullptr == pCell)
			return E_FAIL;

		m_Cells.push_back(pCell);
	}

	CloseHandle(hFile);*/

	ifstream ifs(pNavigationDataFile);
	if (!ifs.is_open())
		return E_FAIL;

	json j;
	try
	{
		ifs >> j;
	}
	catch (const exception& e)
	{
		cerr << "JSON parse error: " << e.what() << "\n";
		return E_FAIL;
	}

	if (!j.contains("cells") || !j["cells"].is_array())
		return E_FAIL;

	for (auto& cellJson : j["cells"])
	{
		if (!cellJson.contains("points") || !cellJson["points"].is_array())
			continue;

		if (cellJson["points"].size() != 3)
			continue; // 꼭 3개의 점만 허용

		_float3 vPoints[3] = {};
		for (int i = 0; i < 3; ++i)
		{
			vPoints[i].x = cellJson["points"][i].value("x", 0.0f);
			vPoints[i].y = cellJson["points"][i].value("y", 0.0f);
			vPoints[i].z = cellJson["points"][i].value("z", 0.0f);
		}

		CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, static_cast<_int>(m_Cells.size()));
		if (nullptr == pCell)
			return E_FAIL;

		m_Cells.push_back(pCell);
	}

	if (FAILED(SetUp_Neighbors()))
		return E_FAIL;


#ifdef _DEBUG
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

#endif

	return S_OK;
}

HRESULT CNavigation::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return S_OK;

	NAVIGATION_DESC* pDesc = static_cast<NAVIGATION_DESC*>(pArg);

	m_iIndex = pDesc->iIndex;

	return S_OK;
}

void CNavigation::Update(_fmatrix WorldMatrix)
{
	XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
}

_bool CNavigation::isMove(_fvector vWorldPos)
{
	_vector		vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	_int		iNeighborIndex = { -1 };

	if (true == m_Cells[m_iIndex]->isIn(vLocalPos, &iNeighborIndex))
		return true;

	else
	{
		if(-1 == iNeighborIndex)
		/* 이웃이 없다면 */
			return false;

		else
		{
			while (true)
			{
				if (true == m_Cells[iNeighborIndex]->isIn(vLocalPos, &iNeighborIndex))
					break;

				if (-1 == iNeighborIndex)
					return false;
			}

			m_iIndex = iNeighborIndex;

			/* 이웃이 있다면 */
			return true;
		}
	}	
}

_vector CNavigation::SetUp_Height(_fvector vWorldPos)
{
	_vector		vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	vLocalPos = XMVectorSetY(vLocalPos, m_Cells[m_iIndex]->Compute_Height(vLocalPos));

	return XMVector3TransformCoord(vLocalPos, XMLoadFloat4x4(&m_WorldMatrix));
}

_vector CNavigation::GetSlideDirection(_fvector vPosition, _fvector vDir)
{
	/* 현재 셀의 3개의 선분과 나가려고하는 위치를 전부 내적해본다. */
	NavigationEdge* NaviEdge = m_Cells[m_iIndex]->FindEdge(vPosition);

	if (NaviEdge == nullptr)
		return vDir;

	/* 나간 경계 선분의 법선 벡터와 내 이동 방향을 내적한다 */
	_vector vLineNormal = XMVector3Normalize(NaviEdge->vNormal);
	_vector vMyDirection = vDir;

	_float fDot = XMVectorGetX(XMVector3Dot(vLineNormal, vMyDirection));

	/* 내 이동 벡터 - 법선벡터 * 스칼라 값 */
	return vMyDirection - vLineNormal * fDot;
}

//이거 업데이트 하지마시오 전체 셀 순회함
HRESULT CNavigation::Select_Cell(_fvector vWorldPos)
{
	//x,z 평면상 내부에 있으면, 저장해뒀다가 가장 가까운 셀로 선택하게 한다.

	_float fMinDist = {FLT_MAX};
	_int	iIndex = { -1 };

	//월드 포지션을 던지면 모든 셀을 안에 있는지 확인하고 그 셀의 인덱스를 던져주자
	for (CCell* pCell : m_Cells)
	{
		_float fDist = {};
		if (pCell->isIn(XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix))), nullptr, &fDist))
		{
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				iIndex = pCell->Get_Index();
			}
		}
	}

	if(iIndex == -1)
		return E_FAIL;

	m_iIndex = iIndex;

	return S_OK;
}

HRESULT CNavigation::Snap(_float3* vWorldPos, _float fSnapThreshold)
{
	//처음 그리는 셀이면 스냅 기능 끄기
	if (m_Cells.size() == 0)
	{
		return E_FAIL;
	}

	_float fMinDist = FLT_MAX;
	_vector fMinDistPoint = {};

	for (CCell* pCell : m_Cells)
	{
		for (_int i = 0; i < CCell::POINT_END; ++i)
		{
			_vector vPoint = pCell->Get_Point(static_cast<CCell::POINT>(i));

			_float fDist = XMVectorGetX(XMVector3Length(XMLoadFloat3(vWorldPos) - vPoint));

			if (fDist < fSnapThreshold && fDist < fMinDist)
			{
				fMinDist = fDist;
				fMinDistPoint = vPoint;
			}
		}
	}

	if (fMinDist != FLT_MAX)
	{
		XMStoreFloat3(vWorldPos, fMinDistPoint);

		return S_OK;
	}


	return E_FAIL;
}

_float CNavigation::Compute_NavigationY(const _vector pTransform)
{
	_matrix		WorldMatrixInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));

	_vector		vPosition = XMVector3TransformCoord(pTransform, WorldMatrixInv);

	_float		fHeight = m_Cells[m_iIndex]->Compute_Height(vPosition);

	vPosition = XMVectorSetY(vPosition, fHeight);

	return XMVectorGetY(XMVector3TransformCoord(vPosition, XMLoadFloat4x4(&m_WorldMatrix)));
}

HRESULT CNavigation::Add_Cell(const _float3* pPoints)
{
	CCell* pCell = CCell::Create(m_pDevice, m_pContext, pPoints, static_cast<_int>(m_Cells.size()));
	if (nullptr == pCell)
		return E_FAIL;
	m_Cells.push_back(pCell);

	if (FAILED(SetUp_Neighbors()))
		return E_FAIL;

	return S_OK;
}

HRESULT CNavigation::Delete_Cell()
{
	if (m_iIndex == -1)
		return S_OK;

	Safe_Release(m_Cells[m_iIndex]);

	m_Cells.erase(m_Cells.begin() + m_iIndex);
	//삭제된 셀의 뒤 쪽 셀들의 인덱스를 앞당겨야한다.
	for (auto iter = m_Cells.begin() + m_iIndex; iter != m_Cells.end(); ++iter)
	{
		_int iIndex = (*iter)->Get_Index();
		(*iter)->Set_Index(--iIndex);
	}

	m_iIndex = -1;

	if (FAILED(SetUp_Neighbors()))
		return E_FAIL;

	return S_OK;
}

HRESULT CNavigation::Save(const _char* Map)
{
	json Json;
	Json["cells"] = json::array();
	
	for (auto& pCell : m_Cells)
	{
		json CellJson;
		CellJson["points"] = json::array();
		for (int i = 0; i < CCell::POINT_END; ++i)
		{
			_float3 Point = { XMVectorGetX(pCell->Get_Point(static_cast<CCell::POINT>(i))),
							  XMVectorGetY(pCell->Get_Point(static_cast<CCell::POINT>(i))),
							  XMVectorGetZ(pCell->Get_Point(static_cast<CCell::POINT>(i))) };
			CellJson["points"].push_back({ {"x", Point.x}, {"y", Point.y}, {"z", Point.z} });
		}
		Json["cells"].push_back(CellJson);
	}

	string NavPath = string("../Bin/Save/NavTool/Nav_") + Map + ".json";
	ofstream ofs(NavPath);
	if (!ofs.is_open())
		return E_FAIL;

	ofs << Json.dump(4); // 4 spaces for indentation

	ofs.close();

	return S_OK;
}

#ifdef _DEBUG
HRESULT CNavigation::Render()
{	
	m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW));
	m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ));

	_float4		vColor = {};

	m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix);
	vColor = _float4(0.f, 1.f, 0.f, 1.f);
	m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));
	m_pShader->Begin(m_iShaderPass);

	//모두다 초록색으로 출력
	for (auto& pCell : m_Cells)
	{
		//선택된 거랑 같으면 제외
		if (pCell->Get_Index() == m_iIndex)
		{
			pCell->Get_NeighborIndices();
			continue;
		}

		pCell->Render();
	}

	if (m_iIndex != -1)
	{
		//이웃 파란색으로
		_int* pNeighborIndices = m_Cells[m_iIndex]->Get_NeighborIndices();

		for (_int i = 0; i < 3; ++i)
		{
			if (pNeighborIndices[i] != -1)
			{
				vColor = _float4(0.f, 0.f, 1.f, 1.f);
				m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));
				m_pShader->Begin(m_iShaderPass);
				m_Cells[pNeighborIndices[i]]->Render();

			}
		}

		/*_float4x4		WorldMatrix = m_WorldMatrix;
		WorldMatrix.m[3][1] += 0.1f;

		m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix);*/

		//선택된것만 빨간색으로 
		vColor = _float4(1.f, 0.f, 0.f, 1.f);

		m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));

		m_pShader->Begin(m_iShaderPass);

		m_Cells[m_iIndex]->Render();
	}

	return S_OK;
}
#endif


HRESULT CNavigation::SetUp_Neighbors()
{
	for (auto& pSourCell : m_Cells)
	{
		pSourCell->Clear_Neighbors();

		for (auto& pDestCell : m_Cells)
		{
			if (pSourCell == pDestCell)
				continue;

			if (true == pDestCell->Compare(pSourCell->Get_Point(CCell::POINT_A), pSourCell->Get_Point(CCell::POINT_B)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_AB, pDestCell);
			}

			if (true == pDestCell->Compare(pSourCell->Get_Point(CCell::POINT_B), pSourCell->Get_Point(CCell::POINT_C)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_BC, pDestCell);
			}

			if (true == pDestCell->Compare(pSourCell->Get_Point(CCell::POINT_C), pSourCell->Get_Point(CCell::POINT_A)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_CA, pDestCell);
			}
		}
	}

	return S_OK;
}

CNavigation* CNavigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile)
{
	CNavigation* pInstance = new CNavigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pNavigationDataFile)))
	{
		MSG_BOX("Failed to Created : CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CNavigation::Clone(void* pArg)
{
	CNavigation* pInstance = new CNavigation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

	for (auto& pCell : m_Cells)
		Safe_Release(pCell);

#ifdef _DEBUG
	Safe_Release(m_pShader);
#endif	
}
