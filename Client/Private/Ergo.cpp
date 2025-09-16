#include "Ergo.h"
#include "GameInstance.h"

#include "Player.h"
#include "PlayerLamp.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "SwordTrailEffect.h"


CErgo::CErgo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CErgo::CErgo(const CErgo& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CErgo::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CErgo::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);
	pDesc->fSpeedPerSec = 10.f;
	//pDesc->fRotationPerSec = 10.f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(Find_Lamp()))
		return E_FAIL;
	
	//XMStoreFloat4x4(
	//	&m_StartHorizontalMatrix,
	//	XMMatrixTranslation(-0.5f, 0.f, 0.f) // (x=0, y=1, z=0)
	//);
	//XMStoreFloat4x4(
	//	&m_HorizontalMatrix,
	//	XMMatrixTranslation(0.5, 0.f, 0.f) // (x=0, y=1, z=0)
	//);

	XMStoreFloat4x4(
		&m_StartVerticalMatrix,
		XMMatrixTranslation(0.f, -0.5f, 0.0f) // (x=0, y=1, z=0)
	);
	XMStoreFloat4x4(
		&m_VerticalMatrix,
		XMMatrixTranslation(0.f, 0.5f, 0.0f) // (x=0, y=1, z=0)
	);

	if (FAILED(Ready_Effect()))
		return E_FAIL;

	for (_uint i = 0; i < ENUM_CLASS(EFFECTS::END); ++i)
	{
		m_pTrailEffect[i]->Set_TrailActive(true);
	}

	Absorbed();

	return S_OK;
}

void CErgo::Priority_Update(_float fTimeDelta)
{
	//if (m_pGameInstance->Key_Down(DIK_L))
	//{
	//	Absorbed();
	//}
}

void CErgo::Update(_float fTimeDelta)
{
	if (m_bAbsorbed)
	{
		_float3 vPos = { m_pLampMatrix->_41, m_pLampMatrix->_42, m_pLampMatrix->_43 };
		m_pTransformCom->Go_Target(fTimeDelta, XMVectorSetW(XMLoadFloat3(&vPos), 1.f), 0.01f);

		_vector vDiff = m_pTransformCom->Get_State(STATE::POSITION) - XMLoadFloat3(&vPos);
		_float fLength = XMVectorGetX(XMVector3Length(vDiff));
		if (fLength < 0.1f)
		{
			m_pTrailEffect[ENUM_CLASS(EFFECTS::VERTICAL)]->Set_TrailActive(false);
			m_pTrailEffect[ENUM_CLASS(EFFECTS::VERTICAL)]->Set_bDead();
			Set_bDead();
		}
	}
}

void CErgo::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CErgo::Render()
{
	if (FAILED(Bind_ShaderResources()))
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

		m_pShaderCom->Begin(0);
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CErgo::Find_Lamp()
{
	CPlayer* pPlayer = static_cast<CPlayer*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player")));

	if (pPlayer == nullptr)
		return E_FAIL;

	//램프의 월드행렬 획득
	m_pLampMatrix = pPlayer->Get_PlayerLamp()->Get_CombinedWorldMatrix();

	if (m_pLampMatrix == nullptr)
		return E_FAIL;

	return S_OK;
}

void CErgo::Absorbed()
{
	m_bAbsorbed = true;

	//// 기준 위치 추출
	//_float3 vLampPos = { m_pLampMatrix->_41, m_pLampMatrix->_42 , m_pLampMatrix->_43 };

	//// 랜덤 방향
	//_float3 vDir;
	//vDir.x = m_pGameInstance->Compute_Random(-1.f, 1.f);
	//vDir.y = m_pGameInstance->Compute_Random(-1.f, 1.f);
	//vDir.z = m_pGameInstance->Compute_Random(-1.f, 1.f);
	//_vector vRandDir = XMVector3Normalize(XMLoadFloat3(&vDir));

	//// 랜덤 거리 (0 ~ 5)
	//_float fDist = m_pGameInstance->Compute_Random(5.f, 10.f);

	//// 최종 위치
	//_vector vTargetPos = XMLoadFloat3(&vLampPos) + vRandDir * fDist;

	//// 적용
	//m_pTransformCom->Set_State(STATE::POSITION, vTargetPos);

	//m_bAbsorbed = !m_bAbsorbed;
}

HRESULT CErgo::Ready_Components(void* pArg)
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(m_pGameInstance->GetCurrentLevelIndex(), _wstring(TEXT("Prototype_Component_Model_PlayerLamp")),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CErgo::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	_bool bTiling = false;
	//타일링을 사용 하는가? 인스턴스된 애들은 타일링 하기 번거롭겠다.
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &bTiling, sizeof(_bool))))
		return E_FAIL;

	return S_OK;
}

HRESULT CErgo::Ready_Effect()
{
	CSwordTrailEffect::DESC desc = {};
	desc.pParentCombinedMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	desc.iLevelID = m_iLevelID;

	//desc.pInnerSocketMatrix = &m_StartHorizontalMatrix; // 0000 identyiu
	//desc.pOuterSocketMatrix = &m_HorizontalMatrix; // 0101? 아무데나, 로컬 위치
	//m_pTrailEffect[ENUM_CLASS(EFFECTS::HORIZONTAL)] = dynamic_cast<CSwordTrailEffect*>(MAKE_SINGLEEFFECT(ENUM_CLASS(m_iLevelID), TEXT("TE_YW_ErgoTrail"), TEXT("Layer_Effect"), 0.f, 0.f, 0.f, &desc));

	desc.pInnerSocketMatrix = &m_StartVerticalMatrix; // 0000 identyiu
	desc.pOuterSocketMatrix = &m_VerticalMatrix; // 0101? 아무데나, 로컬 위치
	m_pTrailEffect[ENUM_CLASS(EFFECTS::VERTICAL)] = dynamic_cast<CSwordTrailEffect*>(MAKE_SINGLEEFFECT(ENUM_CLASS(m_iLevelID), TEXT("TE_YW_ErgoTrail"), TEXT("Layer_Effect"), 0.f, 0.f, 0.f, &desc));

	return S_OK;
}

CErgo* CErgo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CErgo* pGameInstance = new CErgo(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CErgo");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CErgo::Clone(void* pArg)
{
	CErgo* pGameInstance = new CErgo(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CErgo");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CErgo::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);


	//for (_uint i = 0; i < ENUM_CLASS(EFFECTS::END); ++i)
	//{
	//	Safe_Release(m_pTrailEffect[i]);
	//}


}
