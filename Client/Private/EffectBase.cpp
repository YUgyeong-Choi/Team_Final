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
	m_bTool = pDesc->bTool;

	m_iTileX = pDesc->iTileX;
	m_iTileY = pDesc->iTileY;
	m_fTickPerSecond = 60.f; // 60프레임으로 재생
	m_bBillboard = pDesc->bBillboard;
	m_iShaderPass = pDesc->iShaderPass;
	m_bAnimation = pDesc->bAnimation;
	m_isLoop = pDesc->isLoop;

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
	m_fLifeTime += fTimeDelta;
	if (m_fCurrentTrackPosition >= static_cast<_float>(m_iTileCnt))
	{
		if (m_isLoop)
			m_fCurrentTrackPosition = 0.f;
		else
			m_bDead = true;
	}

	Update_Keyframes();

	if (m_bBillboard)
		m_pTransformCom->BillboardToCameraFull(XMLoadFloat4(m_pGameInstance->Get_CamPosition()));

	m_iTileIdx = static_cast<_int>(m_fCurrentTrackPosition);
	// 이부분 AnimationSpeed로 별개로 관리하자 
	m_fOffset.x = (m_iTileIdx % m_iTileX) * m_fTileSize.x;
	m_fOffset.y = (m_iTileIdx / m_iTileX) * m_fTileSize.y;
	//m_fTickAcc = 0.f;
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

	_vector			vScale, vRotation, vPosition, vColor, fIntensity;

	if (m_fCurrentTrackPosition >= LastKeyFrame.fTrackPosition)	
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vPosition = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
		vColor = XMLoadFloat4(&LastKeyFrame.vColor);
		fIntensity = XMLoadFloat(&LastKeyFrame.fIntensity);
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
		_vector			vSourIntensity, vDestIntensity;

		vSourScale = XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex].vScale);
		vDestScale = XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vScale);

		vSourRotation = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex].vRotation);
		vDestRotation = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vRotation);

		vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex].vTranslation), 1.f);
		vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vTranslation), 1.f);

		vSourColor = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex].vColor);
		vDestColor = XMLoadFloat4(&m_KeyFrames[m_iCurKeyFrameIndex + 1].vColor);

		vSourIntensity = XMLoadFloat(&m_KeyFrames[m_iCurKeyFrameIndex].fIntensity);
		vDestIntensity = XMLoadFloat(&m_KeyFrames[m_iCurKeyFrameIndex + 1].fIntensity);

		vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);
		vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);
		vPosition = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);
		vColor = XMVectorLerp(vSourColor, vDestColor, fRatio);
		fIntensity = XMVectorLerp(vSourIntensity, vDestIntensity, fRatio);
	}

	// TransformationMatrix = XMMatrixScaling() * XMMatrixRotationQuaternion() * XMMatrixTranslation();
	TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vPosition);
	_float4x4 resWorldMat = {};
	XMStoreFloat4x4(&resWorldMat, TransformationMatrix);
	m_pTransformCom->Set_WorldMatrix(resWorldMat);
	XMStoreFloat4(&m_vColor, vColor);
	XMStoreFloat(&m_fIntensity, fIntensity);

}

#ifdef USE_IMGUI
HRESULT CEffectBase::Change_Texture(_wstring strTextureName, TEXUSAGE eTex)
{
	_wstring Tag;
	switch (eTex)
	{
	case Client::CEffectBase::TU_DIFFUSE:
		Tag = TEXT("Com_Texture");
		break;
	case Client::CEffectBase::TU_MASK1:
		Tag = TEXT("Com_TextureMask1");
		break;
	case Client::CEffectBase::TU_MASK2:
		Tag = TEXT("Com_TextureMask2");
		break;
	case Client::CEffectBase::TU_MASK3:
		Tag = TEXT("Com_TextureMask3");
		break;
	default:
		return E_FAIL;
	}

	Safe_Release(m_pTextureCom[eTex]);
	_wstring strTextureTag = L"Prototype_Component_Texture_" + strTextureName;
	if (FAILED(Replace_Component(ENUM_CLASS(LEVEL::CY), strTextureTag.c_str(),
		Tag, reinterpret_cast<CComponent**>(&m_pTextureCom[eTex]))))
		return E_FAIL;
	m_TextureTag[eTex] = strTextureName;
	m_bTextureUsage[eTex] = true;
	return S_OK;
}

HRESULT CEffectBase::Delete_Texture(TEXUSAGE eTex)
{
	_wstring Tag;
	switch (eTex)
	{
	case Client::CEffectBase::TU_DIFFUSE:
		Tag = TEXT("Com_Texture");
		break;
	case Client::CEffectBase::TU_MASK1:
		Tag = TEXT("Com_TextureMask1");
		break;
	case Client::CEffectBase::TU_MASK2:
		Tag = TEXT("Com_TextureMask2");
		break;
	case Client::CEffectBase::TU_MASK3:
		Tag = TEXT("Com_TextureMask3");
		break;
	default:
		return E_FAIL;
	}
	Safe_Release(m_pTextureCom[eTex]);
	Remove_Component(Tag);
	m_bTextureUsage[eTex] = false;
	m_TextureTag[eTex] = L"";
	return S_OK;
}


#endif USE_IMGUI

HRESULT CEffectBase::Ready_Textures_Prototype()
{
	_wstring TextureFilePath = TEXT("../Bin/Resources/Textures/Effect/");
	_wstring TextureTag = TEXT("Prototype_Component_Texture_");

	if (m_bTextureUsage[TU_DIFFUSE] == true && m_TextureTag[TU_DIFFUSE].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_DIFFUSE],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_DIFFUSE] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true && m_TextureTag[TU_MASK1].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK1],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_MASK1] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true && m_TextureTag[TU_MASK2].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK2],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_MASK2] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true && m_TextureTag[TU_MASK3].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK3],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_MASK3] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEffectBase::Ready_Textures_Prototype_Tool()
{
	_wstring TextureFilePath = TEXT("../Bin/Resources/Textures/Effect/");
	_wstring TextureTag = TEXT("Prototype_Component_Texture_");

	if (m_bTextureUsage[TU_DIFFUSE] == true && m_TextureTag[TU_DIFFUSE].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TextureTag + m_TextureTag[TU_DIFFUSE],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_DIFFUSE] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
		Change_Texture(m_TextureTag[TU_DIFFUSE], TU_DIFFUSE);
	}
	if (m_bTextureUsage[TU_MASK1] == true && m_TextureTag[TU_MASK1].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TextureTag + m_TextureTag[TU_MASK1],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_MASK1] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
		Change_Texture(m_TextureTag[TU_MASK1], TU_MASK1);
	}
	if (m_bTextureUsage[TU_MASK2] == true && m_TextureTag[TU_MASK2].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TextureTag + m_TextureTag[TU_MASK2],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_MASK2] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
		Change_Texture(m_TextureTag[TU_MASK2], TU_MASK2);
	}
	if (m_bTextureUsage[TU_MASK3] == true && m_TextureTag[TU_MASK3].size() != 0)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TextureTag + m_TextureTag[TU_MASK3],
			CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + m_TextureTag[TU_MASK3] + TEXT(".dds")).c_str(), 1))))
			return E_FAIL;
		Change_Texture(m_TextureTag[TU_MASK3], TU_MASK3);
	}


	return S_OK;
}

void CEffectBase::Free()
{
	__super::Free();	

	Safe_Release(m_pShaderCom);
	for (_uint i = 0; i < TU_END; i++)
	{
		Safe_Release(m_pTextureCom[i]);
	}

}

json CEffectBase::Serialize()
{
	json j;

	// Basic Effect Preferences
	j["LifeTime"] = m_fLifeTime;
	j["Billboard"] = m_bBillboard;
	j["Animation"] = m_bAnimation;
	j["ShaderPass"] = m_iShaderPass;
	j["Loop"] = m_isLoop;

	// Colors
	j["Color"] = { m_vColor.x, m_vColor.y, m_vColor.z, m_vColor.w };
	j["Threshold"] = m_fThreshold;
	j["CenterColor"] = { m_vCenterColor.x, m_vCenterColor.y, m_vCenterColor.z, m_vCenterColor.w };

	// Texture Usage
	json textureUsage = json::array();
	json textureTags = json::array();

	for (int i = 0; i < TU_END; ++i)
	{
		textureUsage.push_back(m_bTextureUsage[i]);
		textureTags.push_back(WStringToString(m_TextureTag[i]));
	}

	j["TextureUsage"] = textureUsage;
	j["TextureTags"] = textureTags;

	// Track Positions
	j["Duration"] = m_iDuration;
	j["StartTrack"] = m_iStartTrackPosition;
	j["EndTrack"] = m_iEndTrackPosition;
	j["TickPerSecond"] = m_fTickPerSecond;

	// KeyFrames
	j["NumKeyFrames"] = m_KeyFrames.size();

	json keyFramesJson = json::array();
	for (auto& key : m_KeyFrames)
		keyFramesJson.push_back(key.Serialize()); // EFFKEYFRAME에 Serialize() 함수 필요
	j["KeyFrames"] = keyFramesJson;

	// UV Grid
	j["TileX"] = m_iTileX;
	j["TileY"] = m_iTileY;
	j["FlipUV"] = m_bFlipUV;

	return j;
}

void CEffectBase::Deserialize(const json& j)
{
	// Basic Effect Preferences
	if (j.contains("LifeTime"))
		m_fLifeTime = j["LifeTime"].get<_float>();

	if (j.contains("Billboard"))
		m_bBillboard = j["Billboard"].get<_bool>();

	if (j.contains("Animation"))
		m_bAnimation = j["Animation"].get<_bool>();

	if (j.contains("ShaderPass"))
		m_iShaderPass = j["ShaderPass"].get<_uint>();

	if (j.contains("Loop"))
		m_isLoop = j["Loop"].get<_bool>();

	
	// Colors
	if (j.contains("Color") && j["Color"].is_array() && j["Color"].size() == 4)
		m_vColor = { j["Color"][0].get<_float>(), j["Color"][1].get<_float>(), j["Color"][2].get<_float>(), j["Color"][3].get<_float>() };

	if (j.contains("Threshold"))
		m_fThreshold = j["Threshold"].get<_float>();

	if (j.contains("CenterColor") && j["CenterColor"].is_array() && j["CenterColor"].size() == 4)
		m_vCenterColor = { j["CenterColor"][0].get<_float>(), j["CenterColor"][1].get<_float>(), j["CenterColor"][2].get<_float>(), j["CenterColor"][3].get<_float>() };

	// Texture Usage
	if (j.contains("TextureUsage") && j["TextureUsage"].is_array())
	{
		for (int i = 0; i < TU_END && i < j["TextureUsage"].size(); ++i)
			m_bTextureUsage[i] = j["TextureUsage"][i].get<_bool>();
	}

	if (j.contains("TextureTags") && j["TextureTags"].is_array())
	{
		for (int i = 0; i < TU_END && i < j["TextureTags"].size(); ++i)
			m_TextureTag[i] = StringToWString(j["TextureTags"][i].get<std::string>());
	}

	// Track Positions
	if (j.contains("Duration"))
		m_iDuration = j["Duration"].get<_int>();

	if (j.contains("StartTrack"))
		m_iStartTrackPosition = j["StartTrack"].get<_int>();

	if (j.contains("EndTrack"))
		m_iEndTrackPosition = j["EndTrack"].get<_int>();

	if (j.contains("TickPerSecond"))
		m_fTickPerSecond = j["TickPerSecond"].get<_float>();

	// KeyFrames
	if (j.contains("NumKeyFrames"))
		m_iNumKeyFrames = j["NumKeyFrames"].get<_uint>();

	if (j.contains("KeyFrames") && j["KeyFrames"].is_array())
	{
		m_KeyFrames.clear();
		for (const auto& keyJson : j["KeyFrames"])
		{
			tagEffectKeyFrame key;
			key.Deserialize(keyJson);
			m_KeyFrames.push_back(key);
		}
	}

	// UV Grid
	if (j.contains("TileX"))
		m_iTileX = j["TileX"].get<_int>();

	if (j.contains("TileY"))
		m_iTileY = j["TileY"].get<_int>();

	if (j.contains("FlipUV"))
		m_bFlipUV = j["FlipUV"].get<_bool>();
}

json CEffectBase::tagEffectKeyFrame::Serialize()
{
	json j;

	j["Scale"] = { vScale.x, vScale.y, vScale.z };
	j["Rotation"] = { vRotation.x, vRotation.y, vRotation.z, vRotation.w };
	j["Translation"] = { vTranslation.x, vTranslation.y, vTranslation.z };
	j["Color"] = { vColor.x, vColor.y, vColor.z, vColor.w };
	j["Intensity"] = fIntensity;
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

	if (j.contains("Intensity"))
		fIntensity = j["Intensity"].get<_float>();

	if (j.contains("TrackPosition"))
		fTrackPosition = j["TrackPosition"].get<_float>();

	if (j.contains("Interpolation"))
		eInterpolationType = static_cast<INTERPOLATION>(j["Interpolation"].get<int>());
}
