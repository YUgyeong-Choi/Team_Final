#include "PlayerLamp.h"

#include "GameInstance.h"
#include "Effect_Manager.h"
#include "SoundController.h"
#include "PhysX_IgnoreSelfCallback.h"

#include "Player.h"

CPlayerLamp::CPlayerLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pShaderCom(nullptr)
{
}
CPlayerLamp::CPlayerLamp(const CPlayerLamp& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom(Prototype.m_pShaderCom)
{
	Safe_AddRef(m_pShaderCom);
}
HRESULT CPlayerLamp::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayerLamp::Initialize(void* pArg)
{
	PLAYERLAMP_DESC* pDesc = static_cast<PLAYERLAMP_DESC*>(pArg);
	m_pParentWorldMatrix = pDesc->pParentWorldMatrix;
	m_pSocketMatrix = pDesc->pSocketMatrix;

	m_fSpeedPerSec = pDesc->fSpeedPerSec;
	m_fRotationPerSec = pDesc->fRotationPerSec;

	m_InitPos = pDesc->InitPos;
	m_InitScale = pDesc->InitScale;
	m_szMeshID = pDesc->szMeshID;
	m_iRender = pDesc->iRender;
	m_szName = pDesc->szName;
	m_pOwner = dynamic_cast<CPlayer*>(pDesc->pOwner);
	m_eMeshLevelID = pDesc->eMeshLevelID;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;
	
	m_pTransformCom->Rotation(_fvector{0.f,0.f,1.f,0.f}, XMConvertToRadians(90.f));
	m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);

	m_eTargetLevel = LEVEL::KRAT_CENTERAL_STATION;
	m_bDebug = false;

	if (FAILED(Ready_Light()))
		return E_FAIL;

	SetRange(6.f);
	SetColor(_float4(1.f, 0.7f, 0.4f, 1.f));
	SetIntensity(1.f);
	m_pSoundCom->Set3DState(0.f, 12.f);
	m_pSoundCom->SetVolume(0.8f);
	return S_OK;
}

void CPlayerLamp::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
		m_pGameInstance->Remove_Light(ENUM_CLASS(LEVEL::DH), m_pLight);

}

void CPlayerLamp::Update(_float fTimeDelta)
{
}

void CPlayerLamp::Late_Update(_float fTimeDelta)
{
	if (!m_bIsVisible)
	{
		m_fSoundElapsed += fTimeDelta;
		if (m_fSoundElapsed >= m_fSoundInterval)
		{
			m_fSoundElapsed = 0.f;

			if (m_pSoundCom)
				m_pSoundCom->Play(m_strLampSound);
		}

		if (m_pSoundCom)
		{
			_float3 vPos{ 30.933064f, 1.5f, 0.672125f };
			m_pSoundCom->Update3DPosition(vPos);
		}
		return;
	}



	_matrix		SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);

	for (size_t i = 0; i < 3; i++)
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);

	/*무기 월드 1.f, 소켓 월드, 부모 월드 0.02f */
	XMStoreFloat4x4(&m_CombinedWorldMatrix,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()) *
		SocketMatrix *
		XMLoadFloat4x4(m_pParentWorldMatrix));

	if (m_bIsUse)
	{
		_vector vPosition = m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vLook = m_pOwner->Get_TransfomCom()->Get_State(STATE::LOOK);
		vPosition += -vLook * 1.5f;
		vPosition = XMVectorAdd(vPosition, XMVectorSet(0.f, 1.f, 0.f, 0.f));

		XMStoreFloat4(&m_pLight->Get_LightDesc()->vPosition, vPosition);
	}


	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CPlayerLamp::Render()
{
	if (!m_bIsVisible)
		return S_OK;

	if (FAILED(Bind_Shader()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		_bool bIsDiffuse = true;
		_bool bIsNormal = true;
		_bool bIsARM = true;
		_bool bIsEmissive = true;

		_float fEmissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
			return E_FAIL;
		_float fGlass = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fGlass", &fGlass, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			bIsDiffuse = false;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			bIsNormal = false;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
			bIsARM = false;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
			bIsEmissive = false;

		_bool bIsGlass = m_pModelCom->HasTexture(i, aiTextureType_AMBIENT);
		
		/* [ 디퓨즈 , 이미시브, 글래스 다 없으면 생략하라 ] */
		if (!bIsDiffuse && !bIsEmissive && !bIsGlass)
			continue;

		/* [ 이미시브 맵이 있다면 사용하라 ] */
		if (bIsEmissive)
		{
			if (m_bIsUse)
			{
				_float fEmissive = 1.f;
				if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
					return E_FAIL;
			}
			else if (!m_bIsUse)
			{
				_float fEmissive = 0.f;
				if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
					return E_FAIL;
			}
		}


		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	// 빛 디버깅 용
	if (!m_bDebug)
		return S_OK;

	/* [ 픽킹 아이디 넘기기 ] */
	_float fID = static_cast<_float>(m_iID);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fID", &fID, sizeof(_float))))
		return E_FAIL;

	_uint		iNumMeshLight = m_pLightModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshLight; i++)
	{
		if (FAILED(m_pLightModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(1);

		m_pLightModelCom->Render(i);
	}

	return S_OK;

}

HRESULT CPlayerLamp::Bind_Shader()
{
	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	_bool bUseTiling = false;
	//타일링을 사용 하는가? 인스턴스된 애들은 타일링 하기 번거롭겠다.
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &bUseTiling, sizeof(_bool))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayerLamp::Ready_Components()
{

	if (m_szMeshID == nullptr)
		return S_OK;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), _wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Model_PointLight")),
		TEXT("Com_Model_Light"), reinterpret_cast<CComponent**>(&m_pLightModelCom))))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_LampAmb"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayerLamp::Ready_Light()
{
	LIGHT_DESC			LightDesc{};


	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);

	LightDesc.fAmbient = 0.2f;
	LightDesc.fIntensity = 0.1f;
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.fFogDensity = 0.f;
	LightDesc.fFogCutoff = 15.f;
	LightDesc.bIsVolumetric = true;
	LightDesc.bIsPlayerFar = true;

	if (FAILED(m_pGameInstance->Add_LevelLightDataReturn(ENUM_CLASS(m_eTargetLevel), LightDesc, &m_pLight)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayerLamp::Play_Absorbe_Effect()
{
	//여기서 흡수 이펙트 재생하면됨

	return S_OK;
}

CPlayerLamp* CPlayerLamp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPlayerLamp* pInstance = new CPlayerLamp(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayerLamp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CPlayerLamp::Clone(void* pArg)
{
	CPlayerLamp* pInstance = new CPlayerLamp(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayerLamp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerLamp::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pSoundCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pLightModelCom);
}
