#include "EffectBase.h"
#include "Client_Calculation.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

CEffectBase::CEffectBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlendObject{ pDevice, pContext }
{

}

CEffectBase::CEffectBase(const CEffectBase& Prototype)
	: CBlendObject( Prototype )
	, m_KeyFrames(Prototype.m_KeyFrames)
{

}

HRESULT CEffectBase::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffectBase::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		MSG_BOX("이펙트는 DESC가 꼭 필요해용");
		return E_FAIL;
	}

	DESC* pDesc = static_cast<DESC*>(pArg);

	m_pSocketMatrix = pDesc->pSocketMatrix;

	m_iTileX = pDesc->iTileX;
	m_iTileY = pDesc->iTileY;
	m_fTileSize.x = 1.0f / _float(m_iTileX);
	m_fTileSize.y = 1.0f / _float(m_iTileY);
	m_iTileCnt = m_iTileX * m_iTileY;
	m_fTickPerSecond = 60.f; // 60프레임으로 재생
	m_bBillboard = pDesc->bBillboard;
	m_iShaderPass = pDesc->iShaderPass;
	m_bAnimation = pDesc->bAnimation;
	m_bTool = pDesc->bTool;

	for (_uint i = 0; i < TU_END; i++)
	{
		m_TextureTag[i] = L"";
	}


	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CEffectBase::Priority_Update(_float fTimeDelta)
{

}

void CEffectBase::Update(_float fTimeDelta)
{
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
	if (m_fCurrentTrackPosition >= static_cast<_float>(m_iTileCnt))
		m_fCurrentTrackPosition = m_iTileCnt - 1.f;

	Update_Keyframes();

	if (m_bBillboard)
		m_pTransformCom->BillboardToCameraFull(XMLoadFloat4(m_pGameInstance->Get_CamPosition()));

	m_iTileIdx = static_cast<_int>(m_fCurrentTrackPosition);
	m_fOffset.x = (m_iTileIdx % m_iTileX) * m_fTileSize.x;
	m_fOffset.y = (m_iTileIdx / m_iTileX) * m_fTileSize.y;
	m_fTickAcc = 0.f;
}

void CEffectBase::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffectBase::Render()
{
	return S_OK;
}

void CEffectBase::Update_Tool(_float fTimeDelta, _float fCurFrame)
{
	m_fCurrentTrackPosition = fCurFrame;
	if (m_fCurrentTrackPosition > static_cast<_float>(m_iDuration))
		m_fCurrentTrackPosition = 0;



	Update_Keyframes();

	if (m_bBillboard)
		m_pTransformCom->BillboardToCameraFull(CCamera_Manager::Get_Instance()->GetPureCamPos());

	if (m_bAnimation)
		m_iTileIdx = static_cast<_int>(m_fCurrentTrackPosition);
	else
		m_iTileIdx = 0;

	if (m_iTileX == 0)
		m_iTileX = 1;
	if (m_iTileY == 0)
		m_iTileY = 1;

	m_fTileSize.x = 1.0f / _float(m_iTileX);
	m_fTileSize.y = 1.0f / _float(m_iTileY);
	m_fOffset.x = (m_iTileIdx % m_iTileX) * m_fTileSize.x;
	m_fOffset.y = (m_iTileIdx / m_iTileX) * m_fTileSize.y;
	//m_fTickAcc = 0.f;
}


_float CEffectBase::Interpolate_Ratio(_float fRatio)
{
	switch (m_KeyFrames[m_iCurKeyFrameIndex].eInterpolationType)
	{
	case Client::INTERPOLATION_LERP:
		return fRatio;
	case Client::INTERPOLATION_EASEOUTBACK:
		return EaseOutBack(fRatio);
	case Client::INTERPOLATION_EASEOUTCUBIC:
		return EaseOutCubic(fRatio);
	case Client::INTERPOLATION_EASEINQUAD:
		return EaseInQuad(fRatio);
	case Client::INTERPOLATION_EASEOUTQUAD:
		return EaseOutBack(fRatio);
	default:
		return fRatio;
	}
}

void CEffectBase::Update_Keyframes()
{
	if (1.0f >= m_fCurrentTrackPosition)
		m_iCurKeyFrameIndex = 0;

	_matrix			TransformationMatrix{};

	EFFKEYFRAME		LastKeyFrame = m_KeyFrames.back();

	_vector			vScale, vRotation, vPosition, vColor;

	if (m_fCurrentTrackPosition >= LastKeyFrame.fTrackPosition)	
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vPosition = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
		vColor = XMLoadFloat4(&LastKeyFrame.vColor);
	}
	else
	{
		if (m_fCurrentTrackPosition >= m_KeyFrames[m_iCurKeyFrameIndex + 1].fTrackPosition)
			++m_iCurKeyFrameIndex;

		_float			fRatio = (m_fCurrentTrackPosition - m_KeyFrames[m_iCurKeyFrameIndex].fTrackPosition) /
			(m_KeyFrames[m_iCurKeyFrameIndex + 1].fTrackPosition - m_KeyFrames[m_iCurKeyFrameIndex].fTrackPosition);

		fRatio = Interpolate_Ratio(fRatio);

		_vector			vSourScale, vDestScale;
		_vector			vSourRotation, vDestRotation;
		_vector			vSourTranslation, vDestTranslation;
		_vector			vSourColor, vDestColor;

		vSourScale = XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex].vScale);
		vDestScale = XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vScale);

		vSourRotation = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex].vRotation);
		vDestRotation = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vRotation);

		vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex].vTranslation), 1.f);
		vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vTranslation), 1.f);

		vSourColor = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex].vColor);
		vDestColor = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vColor);

		vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);
		vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);
		vPosition = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);
		vColor = XMVectorLerp(vSourColor, vDestColor, fRatio);
	}

	// TransformationMatrix = XMMatrixScaling() * XMMatrixRotationQuaternion() * XMMatrixTranslation();
	TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vPosition);
	_float4x4 resWorldMat = {};
	XMStoreFloat4x4(&resWorldMat, TransformationMatrix);
	m_pTransformCom->Set_WorldMatrix(resWorldMat);
	XMStoreFloat4(&m_vColor, vColor);
}

#ifdef USE_IMGUI
HRESULT CEffectBase::Change_Texture(_wstring strTextureName, TEXUSAGE eTex)
{
	Safe_Release(m_pTextureCom[eTex]);
	_wstring strTextureTag = L"Prototype_Component_Texture_" + strTextureName;
	if (FAILED(Replace_Component(ENUM_CLASS(LEVEL::CY), strTextureTag.c_str(),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[eTex]))))
		return E_FAIL;
	m_TextureTag[eTex] = strTextureName;
	return S_OK;
}
#endif USE_IMGUI

void CEffectBase::Free()
{
	__super::Free();	

	Safe_Release(m_pShaderCom);
	for (_uint i = 0; i < TU_END; i++)
	{
		Safe_Release(m_pTextureCom[i]);
	}

}

json CEffectBase::tagEffectKeyFrame::Serialize()
{
	json j;

	j["Scale"] = { vScale.x, vScale.y, vScale.z };
	j["Rotation"] = { vRotation.x, vRotation.y, vRotation.z, vRotation.w };
	j["Translation"] = { vTranslation.x, vTranslation.y, vTranslation.z };
	j["Color"] = { vColor.x, vColor.y, vColor.z, vColor.w };
	j["TrackPosition"] = fTrackPosition;
	j["Interpolation"] = static_cast<int>(eInterpolationType); // 정수 저장

	return j;
}

void CEffectBase::tagEffectKeyFrame::Deserialize(const json& j)
{
	if (j.contains("Scale") && j["Scale"].is_array() && j["Scale"].size() == 3)
	{
		vScale = { j["Scale"][0].get<_float>(), j["Scale"][1].get<_float>(), j["Scale"][2].get<_float>() };
	}

	if (j.contains("Rotation") && j["Rotation"].is_array() && j["Rotation"].size() == 4)
	{
		vRotation = { j["Rotation"][0].get<_float>(), j["Rotation"][1].get<_float>(), j["Rotation"][2].get<_float>(), j["Rotation"][3].get<_float>() };
	}

	if (j.contains("Translation") && j["Translation"].is_array() && j["Translation"].size() == 3)
	{
		vTranslation = { j["Translation"][0].get<_float>(), j["Translation"][1].get<_float>(), j["Translation"][2].get<_float>() };
	}

	if (j.contains("Color") && j["Color"].is_array() && j["Color"].size() == 4)
	{
		vColor = { j["Color"][0].get<_float>(), j["Color"][1].get<_float>(), j["Color"][2].get<_float>(), j["Color"][3].get<_float>() };
	}

	if (j.contains("TrackPosition"))
		fTrackPosition = j["TrackPosition"].get<_float>();

	if (j.contains("Interpolation"))
		eInterpolationType = static_cast<INTERPOLATION>(j["Interpolation"].get<int>());
}
