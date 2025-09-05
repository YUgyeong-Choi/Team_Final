#include "Picking.h"
#include "GameInstance.h"


CPicking::CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);

}

HRESULT CPicking::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);


	D3D11_TEXTURE2D_DESC		TextureDesc{};
	TextureDesc.Width = static_cast<_uint>(ViewportDesc.Width);
	TextureDesc.Height = static_cast<_uint>(ViewportDesc.Height);
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture)))
		return E_FAIL;

	m_pWorldPostions = new _float4[TextureDesc.Width * TextureDesc.Height];
	ZeroMemory(m_pWorldPostions, sizeof(_float4) * TextureDesc.Width * TextureDesc.Height);

	m_pIDs = new _float4[TextureDesc.Width * TextureDesc.Height];
	ZeroMemory(m_pIDs, sizeof(_float4) * TextureDesc.Width * TextureDesc.Height);

	m_iWidth = TextureDesc.Width;
	m_iHeight = TextureDesc.Height;

	return S_OK;
}

void CPicking::Update()
{
	//if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_PickPos"), m_pTexture)))
	//	return;
	//
	//D3D11_MAPPED_SUBRESOURCE		SubResource{};
	//
	//m_pContext->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &SubResource);
	//
	//memcpy(m_pWorldPostions, SubResource.pData, sizeof(_float4) * m_iWidth * m_iHeight);
	//
	//m_pContext->Unmap(m_pTexture, 0);	
}

_bool CPicking::Picking(_float4* pOut)
{
	POINT			ptMouse{};

	GetCursorPos(&ptMouse);
	ScreenToClient(m_hWnd, &ptMouse);

	// ���콺�� Ŭ���̾�Ʈ ���� ���̸� ����
	if (ptMouse.x < 0 || ptMouse.y < 0 || ptMouse.x >= static_cast<_long>(m_iWidth) || ptMouse.y >= static_cast<_long>(m_iHeight))
		return false;

	if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_PBR_Depth"), m_pTexture)))
		return false;
	
	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	
	m_pContext->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &SubResource);
	
	memcpy(m_pWorldPostions, SubResource.pData, sizeof(_float4) * m_iWidth * m_iHeight);
	
	m_pContext->Unmap(m_pTexture, 0);	

	_uint			iIndex = ptMouse.y * m_iWidth + ptMouse.x;

	//���� ���� �ʱⰪ�̸� ��ŷ �ȵȰ����� �Ǵ�.
	if (XMScalarNearEqual(m_pWorldPostions[iIndex].y, 1.f, ai_epsilon))
		return false;

#pragma region Depth ������ ���� ������ �߷�(����Ÿ�� ��� ��¿ �� ���� CPU�� �����)	
	//���� ������ ����������, �̰����� ���带 �߷��غ���(Ŭ���� �ε����� ���� �������� ���Ѵ�)
	_matrix InvProj = m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::PROJ);
	_matrix InvView = m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::VIEW);

	_float2 UV;
	UV.x = (ptMouse.x) / static_cast<_float>(m_iWidth);
	UV.y = (ptMouse.y) / static_cast<_float>(m_iHeight);

	_float zOverW = m_pWorldPostions[iIndex].x; // z/w
	_float viewZ = m_pWorldPostions[iIndex].y; // �佺���̽� �� ����(Near/Far ~ 1)

	// 1. UV -> NDC
	_float ndcX = (UV.x - 0.5f) * 2.f;
	_float ndcY = (UV.y - 0.5f) * -2.f;
	_float ndcZ = zOverW;

	// 2. Ŭ�� ���� ��ǥ (z/w�� �״�� �ְ� w=1)
	_vector clipPos = XMVectorSet(ndcX, ndcY, ndcZ, 1.f);

	// 3. NDC -> �� ����
	_vector viewPos = XMVector4Transform(clipPos, InvProj);
	viewPos /= XMVectorGetW(viewPos);

	// z�� �츮�� ���� viewZ�� ��ü ����
	XMVectorSetZ(viewPos, viewZ);

	// 4. �� ���� -> ���� ����
	_vector worldPos = XMVector4Transform(viewPos, InvView);

	XMStoreFloat4(&m_pWorldPostions[iIndex], worldPos);
#pragma endregion

	*pOut = m_pWorldPostions[iIndex];

	return true;
}

_bool CPicking::PickByClick(_int* pOut)
{
	if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_PBR_Depth"), m_pTexture)))
		return false;

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &SubResource);

	memcpy(m_pIDs, SubResource.pData, sizeof(_float4) * m_iWidth * m_iHeight);

	m_pContext->Unmap(m_pTexture, 0);

	POINT			ptMouse{};

	GetCursorPos(&ptMouse);
	ScreenToClient(m_hWnd, &ptMouse);

	// ���콺�� Ŭ���̾�Ʈ ���� ���̸� ����
	if (ptMouse.x < 0 || ptMouse.y < 0 || ptMouse.x >= static_cast<_long>(m_iWidth) || ptMouse.y >= static_cast<_long>(m_iHeight))
		return false;

	_uint			iIndex = ptMouse.y * m_iWidth + ptMouse.x;

	*pOut = static_cast<_int>(m_pIDs[iIndex].w);

	if (static_cast<_int>(m_pIDs[iIndex].w) < 0.f)
		return false;

	return true;
}

_bool CPicking::PickInRect(const _float2& vStart, const _float2& vEnd, set<_int>* pOut)
{
	if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_PBR_Depth"), m_pTexture)))
		return false;

	D3D11_MAPPED_SUBRESOURCE SubResource{};
	if (FAILED(m_pContext->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &SubResource)))
		return false;

	memcpy(m_pIDs, SubResource.pData, sizeof(_float4) * m_iWidth * m_iHeight);
	m_pContext->Unmap(m_pTexture, 0);

	_long iMinX = (_long)min(vStart.x, vEnd.x);
	_long iMaxX = (_long)max(vStart.x, vEnd.x);
	_long iMinY = (_long)min(vStart.y, vEnd.y);
	_long iMaxY = (_long)max(vStart.y, vEnd.y);

	iMinX = max(0L, iMinX);
	iMaxX = min((_long)m_iWidth - 1, iMaxX);
	iMinY = max(0L, iMinY);
	iMaxY = min((_long)m_iHeight - 1, iMaxY);

	pOut->clear();

	for (_long y = iMinY; y <= iMaxY; ++y)
	{
		for (_long x = iMinX; x <= iMaxX; ++x)
		{
			_uint iIndex = y * m_iWidth + x;
			_int iID = static_cast<_int>(m_pIDs[iIndex].w);

			if (iID >= 0)
				pOut->insert(iID);
		}
	}

	return !pOut->empty(); // ���õ� �� ������ true
}
_bool CPicking::PickingToolMesh(_int* pOut)
{
	if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_PBR_Depth"), m_pTexture)))
		return false;

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &SubResource);

	memcpy(m_pIDs, SubResource.pData, sizeof(_float4) * m_iWidth * m_iHeight);

	m_pContext->Unmap(m_pTexture, 0);

	POINT			ptMouse{};

	GetCursorPos(&ptMouse);
	ScreenToClient(m_hWnd, &ptMouse);

	// ���콺�� Ŭ���̾�Ʈ ���� ���̸� ����
	if (ptMouse.x < 0 || ptMouse.y < 0 || ptMouse.x >= static_cast<_long>(m_iWidth) || ptMouse.y >= static_cast<_long>(m_iHeight))
		return false;

	_uint			iIndex = ptMouse.y * m_iWidth + ptMouse.x;

	*pOut = static_cast<_int>(m_pIDs[iIndex].w);

	if (static_cast<_int>(m_pIDs[iIndex].w) < 0.f)
		return false;

	return true;
}

CPicking* CPicking::Create(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPicking* pInstance = new CPicking(pDevice, pContext);

	if (FAILED(pInstance->Initialize(hWnd)))
	{
		MSG_BOX("Failed to Created : CPicking");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPicking::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pWorldPostions);
	Safe_Delete_Array(m_pIDs);

	Safe_Release(m_pGameInstance);

	Safe_Release(m_pTexture);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
