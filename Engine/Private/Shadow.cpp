#include "Shadow.h"

CShadow::CShadow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CShadow::Ready_Light_For_Shadow(const SHADOW_DESC& Desc, SHADOW eShadow)
{
	_uint					iNumViewports = { 1 };
	D3D11_VIEWPORT			ViewportDesc{};
	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	m_AspectRatio = ViewportDesc.Width / ViewportDesc.Height;
	/* 광원의 정보를 대신할 카메라로 바라본 장면을 그려주기위한 뷰, 투영행렬 만들었다. */
	XMStoreFloat4x4(&m_LightViewMatrix[ENUM_CLASS(eShadow)], XMMatrixLookAtLH(XMLoadFloat4(&Desc.vEye), XMLoadFloat4(&Desc.vAt), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&m_LightProjMatrix[ENUM_CLASS(eShadow)], XMMatrixPerspectiveFovLH(Desc.fFovy, m_AspectRatio, Desc.fNear, Desc.fFar));

	return S_OK;
}


CShadow* CShadow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CShadow(pDevice, pContext);
}

void CShadow::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
