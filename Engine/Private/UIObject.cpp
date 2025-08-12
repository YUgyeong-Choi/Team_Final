#include "UIObject.h"
#include "GameInstance.h"

json CUIObject::Serialize()
{
	json j;

	j["ProtoTag"] = WStringToString(m_strProtoTag);
	j["fX"] = m_fX;
	j["fY"] = m_fY;
	j["SizeX"] = m_fSizeX;
	j["SizeY"] = m_fSizeY;
	j["fOffset"] = m_fOffset;
	j["fAlpha"] = m_fCurrentAlpha;
	j["RotationZ"] = m_fRotation;
	j["Color"]["R"] = m_vColor.x;
	j["Color"]["G"] = m_vColor.y;
	j["Color"]["B"] = m_vColor.z;
	j["Color"]["A"] = m_vColor.w;


	return j;
}

void CUIObject::Deserialize(const json& j)
{
	string protoTag = j["ProtoTag"].get<string>();
	m_strProtoTag = StringToWStringU8(protoTag);
	m_fX = j["fX"].get<_float>();
	m_fY = j["fY"].get<_float>();
	m_fSizeX = j["SizeX"].get<_float>();
	m_fSizeY = j["SizeY"].get<_float>();
	m_fOffset = j["fOffset"].get<_float>();
	m_fCurrentAlpha = j["fAlpha"].get<_float>();
	m_fRotation = j["RotationZ"].get<_float>();

	m_vColor = { j["Color"]["R"].get<_float>() , j["Color"]["G"].get<_float>() , j["Color"]["B"].get<_float>() , j["Color"]["A"].get<_float>() ,};
}

CUIObject::CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CUIObject::CUIObject(const CUIObject& Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CUIObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUIObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (nullptr == pArg)
	{
		D3D11_VIEWPORT			ViewportDesc{};
		_uint					iNumViewports = { 1 };

		m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

		XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.0f, 1.f));

		XMStoreFloat4x4(&m_pTransformCom->Get_World4x4(), XMMatrixIdentity());

		return S_OK;
	}
		

	UIOBJECT_DESC* pDesc = static_cast<UIOBJECT_DESC*>(pArg);

	m_fX = pDesc->fX;
	m_fY = pDesc->fY;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;

	m_fOffset = pDesc->fOffset;
	
	m_fCurrentAlpha = pDesc->fAlpha;

	m_fRotation = pDesc->fRotation;

	

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);
	
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.0f, 1.f));

	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);

	m_pTransformCom->Rotation(0.f, 0.f, m_fRotation);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));

	

	m_strProtoTag = pDesc-> strProtoTag;

	m_vColor = pDesc->vColor;


	return S_OK;
}

void CUIObject::Priority_Update(_float fTimeDelta)
{
}

void CUIObject::Update(_float fTimeDelta)
{
}

void CUIObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CUIObject::Render()
{

	return S_OK;
}

void CUIObject::Update_Data()
{
	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.0f, 1.f));

	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);

	m_pTransformCom->Rotation(0.f, 0.f, XMConvertToRadians(m_fRotation));

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));
}

void CUIObject::Set_Position(_float fX, _float fY)
{

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(fX - ViewportDesc.Width * 0.5f, -fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));
}

void CUIObject::FadeStart(_float fStartAlpha, _float fEndAlpha, _float fTime)
{
	if (m_isFade)
		return;

	m_isFade = true;
	

	m_fStartAlpha = fStartAlpha;
	m_fEndAlpha = fEndAlpha;
	m_fFadeTime = fTime;

	m_fFadeElapsedTime = 0.f;

}

void CUIObject::Fade(float fTimeDelta)
{
	if (!m_isFade)
		return;

	m_fFadeElapsedTime += fTimeDelta;

	if (m_fFadeElapsedTime >= m_fFadeTime)
	{
		m_isFade = false;
		m_fFadeElapsedTime = 0.f;
		m_fCurrentAlpha = m_fEndAlpha;
		return;
	}

	_float t = std::clamp(m_fFadeElapsedTime / m_fFadeTime, 0.f, 1.f);
	m_fCurrentAlpha = LERP(m_fStartAlpha, m_fEndAlpha, t);
}


void CUIObject::Free()
{
	__super::Free();


}
