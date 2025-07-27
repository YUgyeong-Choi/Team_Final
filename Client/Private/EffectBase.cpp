#include "EffectBase.h"
#include "Client_Calculation.h"
#include "GameInstance.h"

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


_float CEffectBase::Interpolate_Ratio(_float fRatio)
{
	switch (m_KeyFrames[m_iCurKeyFrameIndex].eInterpolationType)
	{
	case Client::CEffectBase::INTERPOLATION_LERP:
		return fRatio;
	case Client::CEffectBase::INTERPOLATION_EASEOUTBACK:
		return EaseOutBack(fRatio);
	case Client::CEffectBase::INTERPOLATION_EASEOUTCUBIC:
		return EaseOutCubic(fRatio);
	case Client::CEffectBase::INTERPOLATION_EASEINQUAD:
		return EaseInQuad(fRatio);
	case Client::CEffectBase::INTERPOLATION_EASEOUTQUAD:
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

void CEffectBase::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
