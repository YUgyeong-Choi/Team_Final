#include "Camera_CutScene.h"
#include "GameInstance.h"

#include "Camera_Manager.h"
CCamera_CutScene::CCamera_CutScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CCamera_CutScene::CCamera_CutScene(const CCamera_CutScene& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CCamera_CutScene::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_CutScene::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_CutScene_DESC* pDesc = static_cast<CAMERA_CutScene_DESC*>(pArg);
	m_fMouseSensor = pDesc->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_CutScene::Priority_Update(_float fTimeDelta)
{
	if (m_bActive)
	{
		const CUTSCENE_DESC& curDesc = m_vecCameraFrame[m_iCurrentFrame];

		// 진행 비율 계산
		_float t = m_fElapsedTime / max(0.0001f, curDesc.fDuration);
		t = min(t, 1.f); // 과도한 t 방지

		if (curDesc.bUseLerp)
		{
			const CUTSCENE_DESC& nextDesc = m_vecCameraFrame[m_iCurrentFrame + 1];

			// 1. SRT 분해
			XMVECTOR scale1, rot1, trans1;
			XMMatrixDecompose(&scale1, &rot1, &trans1, curDesc.worldMatrix);

			XMVECTOR scale2, rot2, trans2;
			XMMatrixDecompose(&scale2, &rot2, &trans2, nextDesc.worldMatrix);

			// 2. 보간
			XMVECTOR lerpScale = XMVectorLerp(scale1, scale2, t);
			XMVECTOR lerpTrans = XMVectorLerp(trans1, trans2, t);
			XMVECTOR slerpRot = XMQuaternionSlerp(rot1, rot2, t);

			// 3. 조립
			XMMATRIX matInterpolated =
				XMMatrixScalingFromVector(lerpScale) *
				XMMatrixRotationQuaternion(slerpRot) *
				XMMatrixTranslationFromVector(lerpTrans);

			// 4. 적용
			m_pTransformCom->Set_WorldMatrix(matInterpolated);
		}
		else
		{
			// 즉시 전환
			m_pTransformCom->Set_WorldMatrix(curDesc.worldMatrix);
		}

		// 시간 누적 및 프레임 전환
		m_fElapsedTime += fTimeDelta;
		if (m_fElapsedTime >= curDesc.fDuration)
		{
			m_fElapsedTime = 0.f;
			++m_iCurrentFrame;

			// 마지막 전 프레임이 Lerp라면
			if (m_iCurrentFrame == m_vecCameraFrame.size() - 1)
			{
				const CUTSCENE_DESC& preDesc = m_vecCameraFrame[m_iCurrentFrame-1];
				if (preDesc.bUseLerp)
				{
					m_bActive = false;
					m_fElapsedTime = 0.f;
					m_iCurrentFrame = -1;
					CCamera_Manager::Get_Instance()->SetFreeCam();
					return;
				}
			}

			// 마지막 프레임이 끝났다면
			if (m_iCurrentFrame == m_vecCameraFrame.size()) {
				m_bActive = false;
				m_fElapsedTime = 0.f;
				m_iCurrentFrame = -1;
				CCamera_Manager::Get_Instance()->SetFreeCam();
				return;
			}
		}
	}

	__super::Priority_Update(fTimeDelta);
}

void CCamera_CutScene::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CCamera_CutScene::Late_Update(_float fTimeDelta)
{
}

HRESULT CCamera_CutScene::Render()
{
	return S_OK;
}


CCamera_CutScene* CCamera_CutScene::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_CutScene* pGameInstance = new CCamera_CutScene(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCamera_CutScene");
		Safe_Release(pGameInstance);
	}
	return pGameInstance;
}


CGameObject* CCamera_CutScene::Clone(void* pArg)
{
	CCamera_CutScene* pGameInstance = new CCamera_CutScene(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CCamera_CutScene");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CCamera_CutScene::Free()
{
	__super::Free();

}
