#include "Camera_CutScene.h"
#include "GameInstance.h"
#include "Client_Calculation.h"

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
		_float t = m_fElapsedTime / max(0.0001f, curDesc.fInterpDuration);
		t = min(t, 1.f); // 과도한 t 방지

		if (curDesc.eInterp == INTERPOLATION_CAMERA::LERP)
		{
			CUTSCENE_DESC nextDesc{};
			// 예외 처리
			if (m_iCurrentFrame + 1 == m_vecCameraFrame.size())
				nextDesc = curDesc;
			else
				nextDesc = m_vecCameraFrame[m_iCurrentFrame + 1];

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
		}else if (curDesc.eInterp == INTERPOLATION_CAMERA::CATMULLROM)
		{
			CUTSCENE_DESC prevDesc;
			CUTSCENE_DESC nextDesc;
			CUTSCENE_DESC nextNextDesc;

			// 예외 처리
			if (m_iCurrentFrame - 1 == -1)
				prevDesc = curDesc;
			else
				prevDesc = m_vecCameraFrame[m_iCurrentFrame - 1];

			if (m_iCurrentFrame + 1 >= m_vecCameraFrame.size())
			{
				nextDesc = curDesc;
			}
			else {
				nextDesc = m_vecCameraFrame[m_iCurrentFrame + 1];
			}

			if (m_iCurrentFrame + 2 >= m_vecCameraFrame.size())
			{
				nextNextDesc = curDesc;
			}
			else {
				nextNextDesc = m_vecCameraFrame[m_iCurrentFrame + 2];
			}

			// 각각의 위치값 추출
			XMVECTOR pos0 = XMVector3TransformCoord(XMVectorZero(), prevDesc.worldMatrix);
			XMVECTOR pos1 = XMVector3TransformCoord(XMVectorZero(), curDesc.worldMatrix);
			XMVECTOR pos2 = XMVector3TransformCoord(XMVectorZero(), nextDesc.worldMatrix);
			XMVECTOR pos3 = XMVector3TransformCoord(XMVectorZero(), nextNextDesc.worldMatrix);

			// 쿼터니언 회전 추출
			XMVECTOR scale1, rot1, _;
			XMMatrixDecompose(&scale1, &rot1, &_, curDesc.worldMatrix);

			XMVECTOR scale2, rot2, __;
			XMMatrixDecompose(&scale2, &rot2, &__, nextDesc.worldMatrix);

			// 위치는 CatmullRom 보간
			XMVECTOR interpolatedPos = CatmullRom(pos0, pos1, pos2, pos3, t);

			// 회전은 SLERP로 유지 (CatmullRom은 회전에 잘 안 맞음)
			XMVECTOR interpolatedRot = XMQuaternionSlerp(rot1, rot2, t);

			// 스케일도 선형 보간
			XMVECTOR interpolatedScale = XMVectorLerp(scale1, scale2, t);

			// 최종 행렬 조립
			XMMATRIX resultMat =
				XMMatrixScalingFromVector(interpolatedScale) *
				XMMatrixRotationQuaternion(interpolatedRot) *
				XMMatrixTranslationFromVector(interpolatedPos);

			m_pTransformCom->Set_WorldMatrix(resultMat);
		}
		else
		{
			// 즉시 전환
			m_pTransformCom->Set_WorldMatrix(curDesc.worldMatrix);
		}

		// 시간 누적 및 프레임 전환
		m_fElapsedTime += fTimeDelta;
		if (m_fElapsedTime >= curDesc.fInterpDuration)
		{
			m_fElapsedTime = 0.f;
			++m_iCurrentFrame;

			// 마지막 전 프레임이 Lerp라면
			if (m_iCurrentFrame == m_vecCameraFrame.size() - 1)
			{
				const CUTSCENE_DESC& preDesc = m_vecCameraFrame[m_iCurrentFrame-1];
				if (preDesc.eInterp == INTERPOLATION_CAMERA::LERP)
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


		if (m_iCurrentFrame + 1 != m_vecCameraFrame.size())
		{
			CUTSCENE_DESC nextDesc = m_vecCameraFrame[m_iCurrentFrame + 1];
			if (nextDesc.bZoom)
			{
				_float startFov = curDesc.fFov;
				_float endFov = nextDesc.fFov;
				m_fFov = XMConvertToRadians(startFov + (endFov - startFov) * t);
			}
		}

		if (curDesc.bZoom)
		{
			CUTSCENE_DESC nextDesc{};
			if (m_iCurrentFrame + 1 == m_vecCameraFrame.size())
				nextDesc = curDesc;
			else
				nextDesc = m_vecCameraFrame[m_iCurrentFrame + 1];

			_float  startFov = curDesc.fFov;
			_float  endFov = nextDesc.fFov;
			m_fFov = XMConvertToRadians(startFov + (endFov - startFov) * t);
		}


		if (m_pGameInstance->Key_Down(DIK_M))
			m_bStartSpecialRotate = true;

		if (m_bStartSpecialRotate)
		{
			bool bFinished = m_pTransformCom->Rotate_Special(fTimeDelta, 1.0f, m_pTransformCom->Get_State(STATE::LOOK), 30.f);
			if (bFinished)
				m_bStartSpecialRotate = false;
		}

		if (m_pGameInstance->Key_Down(DIK_N))
			m_bStartSpecialRotate2 = true;

		if (m_bStartSpecialRotate2)
		{
			bool bFinished = m_pTransformCom->Rotate_Special(fTimeDelta, 1.0f, m_pTransformCom->Get_State(STATE::LOOK), -30.f);
			if (bFinished)
				m_bStartSpecialRotate2 = false;
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
