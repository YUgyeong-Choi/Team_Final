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

	if (FAILED(InitDatas()))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(m_CutSceneDatas[CUTSCENE_TYPE::TWO].vecWorldMatrixData.front().WorldMatrix);
	m_pGameInstance->Set_Transform(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));

	return S_OK;
}

void CCamera_CutScene::Priority_Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	if (m_bActive)
	{
		if (!m_bOrbitalToSetOrbital)
		{
			_bool bFinish = ReadyToOrbitalWorldMatrix(fTimeDelta);
			m_bOrbitalToSetOrbital = bFinish;
		}

		if (m_bOrbitalToSetOrbital && !m_bReadyCutScene)
		{
			// 목표 행렬
			_matrix targetMat = m_CameraDatas.vecWorldMatrixData.front().WorldMatrix;
			// 현재 행렬
			_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

			_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);
			m_bReadyCutScene = bFinish;
		}

		if (m_bReadyCutScene && !m_bReadyCutSceneOrbital)
		{
			m_fElapsedTime += fTimeDelta;

			// 시간 누적 → 프레임 단위 변환
			_int iNewFrame = static_cast<_int>(m_fElapsedTime * m_fFrameSpeed);

			if (iNewFrame != m_iCurrentFrame)
			{
				m_iCurrentFrame = iNewFrame;

				Interp_WorldMatrixOnly(m_iCurrentFrame);
				Interp_Fov(m_iCurrentFrame);
				Interp_OffsetRot(m_iCurrentFrame);

				// 종료 조건
				if (m_iCurrentFrame > m_CameraDatas.iEndFrame)
				{
					m_bReadyCutSceneOrbital = true;
				}
			}
		}

		if (m_bReadyCutSceneOrbital)
		{
			m_initOrbitalMatrix = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_OrbitalWorldMatrix(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
			// 목표 행렬
			_matrix targetMat = m_initOrbitalMatrix;
			// 현재 행렬
			_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

			_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);

			if (bFinish)
			{
				m_bActive = false;
				m_bOrbitalToSetOrbital = false;
				m_bReadyCutScene = false;
				m_bReadyCutSceneOrbital = false;
				m_fElapsedTime = 0.f;
				m_iCurrentFrame = -1;
				m_initOrbitalMatrix = {};
				CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_InitCam();
				CCamera_Manager::Get_Instance()->SetOrbitalCam();
			}
		}
	}
	__super::Priority_Update(fTimeDelta);
}

void CCamera_CutScene::Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;
	__super::Update(fTimeDelta);
}

void CCamera_CutScene::Late_Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;
}

HRESULT CCamera_CutScene::Render()
{
	return S_OK;
}

void CCamera_CutScene::Set_CameraFrame(const CAMERA_FRAMEDATA CameraFrameData)
{
	m_CameraDatas = CameraFrameData;
	m_bOrbitalToSetOrbital = CameraFrameData.bOrbitalToSetOrbital;
	if (m_bOrbitalToSetOrbital)
		m_pTransformCom->Set_WorldMatrix(m_CameraDatas.vecWorldMatrixData.front().WorldMatrix);
	m_bReadyCutSceneOrbital = CameraFrameData.bReadyCutSceneOrbital;
	m_bReadyCutScene = false;

	m_initOrbitalMatrix = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_OrbitalWorldMatrix(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
	CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_TransfomCom()->Set_WorldMatrix(m_initOrbitalMatrix);
	CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_PitchYaw(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
}

void CCamera_CutScene::Set_CutSceneData(CUTSCENE_TYPE cutSceneType)
{
	m_CameraDatas = m_CutSceneDatas[cutSceneType];
	m_bOrbitalToSetOrbital = m_CameraDatas.bOrbitalToSetOrbital;
	if (m_bOrbitalToSetOrbital)
		m_pTransformCom->Set_WorldMatrix(m_CameraDatas.vecWorldMatrixData.front().WorldMatrix);
	m_bReadyCutSceneOrbital = m_CameraDatas.bReadyCutSceneOrbital;
	m_bReadyCutScene = false;

	m_initOrbitalMatrix = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_OrbitalWorldMatrix(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
	CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_TransfomCom()->Set_WorldMatrix(m_initOrbitalMatrix);
	CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_PitchYaw(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
}

void CCamera_CutScene::Interp_WorldMatrixOnly(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecWorldMatrixData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.keyFrame && curFrame <= b.keyFrame)
		{
			const float t = float(curFrame - a.keyFrame) / float(max(1, b.keyFrame - a.keyFrame));
			const INTERPOLATION_CAMERA interp = a.interpPosition;

			// Decompose A
			XMVECTOR sA, rA, tA;
			XMMatrixDecompose(&sA, &rA, &tA, a.WorldMatrix);

			// Decompose B
			XMVECTOR sB, rB, tB;
			XMMatrixDecompose(&sB, &rB, &tB, b.WorldMatrix);

			XMVECTOR finalT = tA;
			XMVECTOR finalR = rA;
			XMVECTOR finalS = sA;

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				finalT = tA;
				finalR = rA;
				finalS = sA;
				break;

			case INTERPOLATION_CAMERA::LERP:
				finalT = XMVectorLerp(tA, tB, t);
				finalR = XMQuaternionSlerp(rA, rB, t);
				finalS = XMVectorLerp(sA, sB, t);
				break;

			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				XMVECTOR t0 = (i == 0) ? tA : XMMatrixDecompose_T(vec[i - 1].WorldMatrix);
				XMVECTOR t1 = tA;
				XMVECTOR t2 = tB;
				XMVECTOR t3 = (i + 2 < vec.size()) ? XMMatrixDecompose_T(vec[i + 2].WorldMatrix) : t2;

				finalT = XMVectorCatmullRom(t0, t1, t2, t3, t);
				finalR = XMQuaternionSlerp(rA, rB, t); // 회전은 안정성을 위해 그대로 Slerp
				finalS = XMVectorLerp(sA, sB, t);      // 스케일은 단순 LERP
				break;
			}
			}

			// 다시 합성
			_matrix result = XMMatrixAffineTransformation(finalS, XMVectorZero(), finalR, finalT);
			m_pTransformCom->Set_WorldMatrix(result);
			return;
		}
	}

	// 범위 밖이면 고정
	const _matrix& m = (curFrame <= vec.front().keyFrame) ? vec.front().WorldMatrix : vec.back().WorldMatrix;
	m_pTransformCom->Set_WorldMatrix(m);
}

void CCamera_CutScene::Interp_Fov(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecFovData;
	if (vec.empty())
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.keyFrame && curFrame <= b.keyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpFov;
			float t = float(curFrame - a.keyFrame) / float(max(1, b.keyFrame - a.keyFrame));

			_float result = a.fFov;

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				result = a.fFov;
				break;

			case INTERPOLATION_CAMERA::LERP:
			default:
				result = a.fFov + (b.fFov - a.fFov) * t;
				break;

			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				_float p0 = (i == 0) ? a.fFov : vec[i - 1].fFov;
				_float p1 = a.fFov;
				_float p2 = b.fFov;
				_float p3 = (i + 2 < vec.size()) ? vec[i + 2].fFov : p2;

				result = CatmullRom(p0, p1, p2, p3, t);
				break;
			}
			}

			Set_FOV(XMConvertToRadians(result)); // ← 라디안 변환 후 적용
			return;
		}
	}

	// 경계 외에서는 처음 또는 마지막 값 고정
	if (curFrame <= vec.front().keyFrame)
		Set_FOV(XMConvertToRadians(vec.front().fFov));
	else if (curFrame >= vec.back().keyFrame)
		Set_FOV(XMConvertToRadians(vec.back().fFov));
}

void CCamera_CutScene::Interp_OffsetRot(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecRotData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.keyFrame && curFrame <= b.keyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpRotation;
			float t = float(curFrame - a.keyFrame) / float(max(1, b.keyFrame - a.keyFrame));

			XMVECTOR rotA = XMLoadFloat3(&a.offSetRot);
			XMVECTOR rotB = XMLoadFloat3(&b.offSetRot);
			XMVECTOR result = {};

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
				break;
			case INTERPOLATION_CAMERA::LERP:
			default:
				result = XMVectorLerp(rotA, rotB, t);
				break;
			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				XMVECTOR p1 = rotA;
				XMVECTOR p2 = rotB;
				XMVECTOR p0 = (i == 0) ? p1 : XMLoadFloat3(&vec[i - 1].offSetRot);
				XMVECTOR p3 = (i + 2 < vec.size()) ? XMLoadFloat3(&vec[i + 2].offSetRot) : p2;
				result = XMVectorCatmullRom(p0, p1, p2, p3, t);
				break;
			}
			}

			// 오프셋 회전 저장 (최종 회전은 update에서 적용)
			m_vCurrentShakeRot = result;
			return;
		}
	}

	// 범위 바깥이면 시작/끝값
	if (curFrame <= vec.front().keyFrame)
		m_vCurrentShakeRot = XMLoadFloat3(&vec.front().offSetRot);
	else if (curFrame >= vec.back().keyFrame)
		if (vec.back().interpRotation == INTERPOLATION_CAMERA::NONE)
			m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
		else
			m_vCurrentShakeRot = XMLoadFloat3(&vec.back().offSetRot);
}

XMVECTOR CCamera_CutScene::XMMatrixDecompose_T(const _matrix& m)
{
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, m);
	return trans;
}

HRESULT CCamera_CutScene::InitDatas()
{
	std::ifstream inFile("../Bin/Save/CutScene/one.json");
	if (inFile.is_open())
	{
		json j;
		inFile >> j;
		inFile.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::ONE, LoadCameraFrameData(j)));
	}

	std::ifstream inFile2("../Bin/Save/CutScene/two.json");
	if (inFile2.is_open())
	{
		json j;
		inFile2 >> j;
		inFile2.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::TWO, LoadCameraFrameData(j)));
	}
	return S_OK;
}

CAMERA_FRAMEDATA CCamera_CutScene::LoadCameraFrameData(const json& j)
{
	CAMERA_FRAMEDATA data;

	// 1. iEndFrame
	data.iEndFrame = j.value("iEndFrame", 0);
	data.bOrbitalToSetOrbital = j.value("bStartBlend", false);
	data.bReadyCutSceneOrbital = j.value("bEndBlend", false);
	data.fPitch = j["Pitch"].get<float>();
	data.fYaw = j["Yaw"].get<float>();

	// 2. vecPosData
	if (j.contains("vecPosData"))
	{
		for (const auto& posJson : j["vecPosData"])
		{
			CAMERA_WORLDFRAME posFrame;
			posFrame.keyFrame = posJson["keyFrame"];
			posFrame.interpPosition = posJson["interpPosition"];

			const std::vector<float>& matValues = posJson["worldMatrix"];
			XMFLOAT4X4 mat;
			memcpy(&mat, matValues.data(), sizeof(float) * 16);
			posFrame.WorldMatrix = XMLoadFloat4x4(&mat);

			data.vecWorldMatrixData.push_back(posFrame);
		}
	}

	// 3. vecRotData
	if (j.contains("vecRotData"))
	{
		for (const auto& rotJson : j["vecRotData"])
		{
			CAMERA_ROTFRAME rotFrame;
			rotFrame.keyFrame = rotJson["keyFrame"];
			rotFrame.offSetRot = XMFLOAT3(
				rotJson["rotation"][0],
				rotJson["rotation"][1],
				rotJson["rotation"][2]
			);
			rotFrame.interpRotation = rotJson["interpRotation"];

			data.vecRotData.push_back(rotFrame);
		}
	}

	// 4. vecFovData
	if (j.contains("vecFovData"))
	{
		for (const auto& fovJson : j["vecFovData"])
		{
			CAMERA_FOVFRAME fovFrame;
			fovFrame.keyFrame = fovJson["keyFrame"];
			fovFrame.fFov = fovJson["fFov"];
			fovFrame.interpFov = fovJson["interpFov"];

			data.vecFovData.push_back(fovFrame);
		}
	}

	return data;
}

_bool CCamera_CutScene::ReadyToOrbitalWorldMatrix(_float fTimeDelta)
{
	_matrix currentMat = m_pTransformCom->Get_WorldMatrix();
	_matrix targetMat = m_initOrbitalMatrix;

	XMVECTOR curScale, curRot, curTrans;
	XMVECTOR tgtScale, tgtRot, tgtTrans;

	XMMatrixDecompose(&curScale, &curRot, &curTrans, currentMat);
	XMMatrixDecompose(&tgtScale, &tgtRot, &tgtTrans, targetMat);

	_float blendSpeed = 5.f;
	_float t = fTimeDelta * blendSpeed;

	// 보간
	XMVECTOR lerpPos = XMVectorLerp(curTrans, tgtTrans, t);
	XMVECTOR slerpRot = XMQuaternionSlerp(curRot, tgtRot, t);
	XMVECTOR lerpScale = XMVectorLerp(curScale, tgtScale, t);

	_matrix blendedMat = XMMatrixScalingFromVector(lerpScale)
		* XMMatrixRotationQuaternion(slerpRot)
		* XMMatrixTranslationFromVector(lerpPos);

	m_pTransformCom->Set_WorldMatrix(blendedMat);

	XMVECTOR diff = XMVector3LengthSq(tgtTrans - lerpPos);
	if (XMVectorGetX(diff) < 0.0001f)
	{
		return true;
	}

	return false;
}

_bool CCamera_CutScene::Camera_Blending(_float fTimeDelta, _matrix targetMat, _matrix currentMat)
{
	XMVECTOR curScale, curRotQuat, curTrans;
	XMVECTOR tgtScale, tgtRotQuat, tgtTrans;
	XMMatrixDecompose(&curScale, &curRotQuat, &curTrans, currentMat);
	XMMatrixDecompose(&tgtScale, &tgtRotQuat, &tgtTrans, targetMat);

	_float blendSpeed = 5.f;
	_float t = fTimeDelta * blendSpeed;
	t = min(t, 1.0f); // overshoot 방지

	XMVECTOR lerpPos = XMVectorLerp(curTrans, tgtTrans, t);
	XMVECTOR slerpRot = XMQuaternionSlerp(curRotQuat, tgtRotQuat, t);
	XMVECTOR lerpScale = XMVectorLerp(curScale, tgtScale, t);

	// 재합성
	_matrix blendedMat = XMMatrixScalingFromVector(lerpScale)
		* XMMatrixRotationQuaternion(slerpRot)
		* XMMatrixTranslationFromVector(lerpPos);

	m_pTransformCom->Set_WorldMatrix(blendedMat);

	// 차이 계산
	XMVECTOR diff = XMVector3LengthSq(tgtTrans - lerpPos);
	if (XMVectorGetX(diff) < 0.00001f)
	{
		return true; 
	}
	return false;
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
