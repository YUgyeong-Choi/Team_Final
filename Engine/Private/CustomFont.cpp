#include "CustomFont.h"

CCustomFont::CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

HRESULT CCustomFont::Initialize(const _tchar* pFontFilePath)
{
	m_pFont = new SpriteFont(m_pDevice, pFontFilePath);

	return S_OK;
}

void CCustomFont::Draw(SpriteBatch* pBatch, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, _float fScale)
{
	m_pFont->DrawString(pBatch, pText, vPosition, vColor, fRotation, vOrigin, fScale);
}

void CCustomFont::Draw_Centered(SpriteBatch* pBatch, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, _float fScale)
{
	_float2 vFontDesc = {};
	XMStoreFloat2(&vFontDesc, m_pFont->MeasureString(pText));


	float textWidth = vFontDesc.x * fScale;
	float textHeight = vFontDesc.y * fScale;

	_vector vCenteredPos = XMLoadFloat2(&vPosition);

	vCenteredPos -= {textWidth * 0.5f, textHeight * 0.5f,0.f,0.f};

	_float2 vPos = {};

	XMStoreFloat2(&vPos, vCenteredPos);

	m_pFont->DrawString(pBatch, pText, vPos, vColor, fRotation, vOrigin, fScale);

}

_float2 CCustomFont::Calc_Draw_Range(const _tchar* pText)
{
	_float2 vFontDesc = {};
	XMStoreFloat2(&vFontDesc, m_pFont->MeasureString(pText));

	return vFontDesc;
}

CCustomFont* CCustomFont::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFontFilePath)
{
	CCustomFont* pInstance = new CCustomFont(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pFontFilePath)))
	{
		MSG_BOX("Failed to Created : CCustomFont");
		Safe_Release(pInstance);
	}

	return pInstance;
}



void CCustomFont::Free()
{
	__super::Free();

	Safe_Delete(m_pFont);
	
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
