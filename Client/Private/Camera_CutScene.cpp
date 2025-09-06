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

	m_pTransformCom->Set_WorldMatrix(m_CutSceneDatas[CUTSCENE_TYPE::WAKEUP].vecWorldMatrixData.front().WorldMatrix);
	m_pGameInstance->Set_Transform(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));

	return S_OK;
}

void CCamera_CutScene::Priority_Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	if (KEY_DOWN(DIK_C))
	{
		m_bStopCamera = !m_bStopCamera;
	}
	
	if (m_bStopCamera)
		fTimeDelta = 0.f;
	
	if (m_bActive)
	{
		if (!m_bOrbitalToSetOrbital)
		{
			_bool bFinish = ReadyToOrbitalWorldMatrix(fTimeDelta);
			m_bOrbitalToSetOrbital = bFinish;
		}

		if (m_bOrbitalToSetOrbital && !m_bReadyCutScene)
		{
			_bool bCheck = false;
			if (m_CameraDatas.vecTargetData.size() == 0)
			{
				// 목표 행렬
				_matrix targetMat = m_CameraDatas.vecWorldMatrixData.front().WorldMatrix;
				// 현재 행렬
				_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

				_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);
				m_bReadyCutScene = bFinish;
				bCheck = true;
			}

			if (m_CameraDatas.vecWorldMatrixData.size() == 0)
			{
				m_bReadyCutScene = true;
				bCheck = true;
			}

			if (!bCheck && (m_CameraDatas.vecWorldMatrixData.front().iKeyFrame > m_CameraDatas.vecTargetData.front().iKeyFrame))
			{
				m_bReadyCutScene = true;
			}

			if (!bCheck && (m_CameraDatas.vecWorldMatrixData.front().iKeyFrame < m_CameraDatas.vecTargetData.front().iKeyFrame))
			{
				// 목표 행렬
				_matrix targetMat = m_CameraDatas.vecWorldMatrixData.front().WorldMatrix;
				// 현재 행렬
				_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

				_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);
				m_bReadyCutScene = bFinish;
			}
		}

		if (m_bReadyCutScene && !m_bReadyCutSceneOrbital)
		{
			m_fElapsedTime += fTimeDelta;

			// 시간 누적 → 프레임 단위 변환
			_int iNewFrame = static_cast<_int>(m_fElapsedTime * m_fFrameSpeed);

			if (iNewFrame != m_iCurrentFrame)
			{ 
				m_iCurrentFrame = iNewFrame;

				//printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzPlszzzzzzzzzzzzzzzzzzzzzzzzz\n");
				//printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzPlszzzzzzzzzzzzzzzzzzzzzzzzz\n");
				//printf("%d CurrentFrame\n", m_iCurrentFrame);

				Interp_WorldMatrixOnly(m_iCurrentFrame);
				Interp_Fov(m_iCurrentFrame);
				Interp_OffsetRot(m_iCurrentFrame);
				Interp_OffsetPos(m_iCurrentFrame);
				Interp_Target(m_iCurrentFrame);

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
				CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_InitCam(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
				CCamera_Manager::Get_Instance()->SetOrbitalCam();
			}
		}
	}
	//__super::Priority_Update(fTimeDelta);
}

void CCamera_CutScene::Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	if (KEY_DOWN(DIK_CAPSLOCK))
		PrintMatrix("CutScene CameraWold", XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));

	if (m_bStopCamera)
		fTimeDelta = 0.f;

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

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const float t = float(curFrame - a.iKeyFrame) / float(max(1, b.iKeyFrame - a.iKeyFrame));
			const INTERPOLATION_CAMERA interp = a.interpMatrixPos;

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
	//const _matrix& m = (curFrame <= vec.front().iKeyFrame) ? vec.front().WorldMatrix : vec.back().WorldMatrix;
	//m_pTransformCom->Set_WorldMatrix(m);
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

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpFov;
			float t = float(curFrame - a.iKeyFrame) / float(max(1, b.iKeyFrame - a.iKeyFrame));

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
	if (curFrame <= vec.front().iKeyFrame)
		Set_FOV(XMConvertToRadians(vec.front().fFov));
	else if (curFrame >= vec.back().iKeyFrame)
		Set_FOV(XMConvertToRadians(vec.back().fFov));
}

void CCamera_CutScene::Interp_OffsetRot(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecOffSetRotData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpOffSetRot;
			float t = float(curFrame - a.iKeyFrame) / float(max(1, b.iKeyFrame - a.iKeyFrame));

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
	if (curFrame <= vec.front().iKeyFrame)
		m_vCurrentShakeRot = XMLoadFloat3(&vec.front().offSetRot);
	else if (curFrame >= vec.back().iKeyFrame)
		if (vec.back().interpOffSetRot == INTERPOLATION_CAMERA::NONE)
			m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
		else
			m_vCurrentShakeRot = XMLoadFloat3(&vec.back().offSetRot);
}

void CCamera_CutScene::Interp_OffsetPos(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecOffSetPosData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpOffSetPos;
			float t = float(curFrame - a.iKeyFrame) / float(max(1, b.iKeyFrame - a.iKeyFrame));

			XMVECTOR rotA = XMLoadFloat3(&a.offSetPos);
			XMVECTOR rotB = XMLoadFloat3(&b.offSetPos);
			XMVECTOR result = {};

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				m_vCurrentShakePos = { 0.f, 0.f, 0.f, 0.f };
				break;
			case INTERPOLATION_CAMERA::LERP:
			default:
				result = XMVectorLerp(rotA, rotB, t);
				break;
			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				XMVECTOR p1 = rotA;
				XMVECTOR p2 = rotB;
				XMVECTOR p0 = (i == 0) ? p1 : XMLoadFloat3(&vec[i - 1].offSetPos);
				XMVECTOR p3 = (i + 2 < vec.size()) ? XMLoadFloat3(&vec[i + 2].offSetPos) : p2;
				result = XMVectorCatmullRom(p0, p1, p2, p3, t);
				break;
			}
			}

			// 오프셋 위치 저장 (최종 위치는 update에서 적용)
			m_vCurrentShakePos = result;
			return;
		}
	}

	// 범위 바깥이면 시작/끝값
	if (curFrame <= vec.front().iKeyFrame)
		m_vCurrentShakePos = XMLoadFloat3(&vec.front().offSetPos);
	else if (curFrame >= vec.back().iKeyFrame)
		if (vec.back().interpOffSetPos == INTERPOLATION_CAMERA::NONE)
			m_vCurrentShakePos = { 0.f, 0.f, 0.f, 0.f };
		else
			m_vCurrentShakePos = XMLoadFloat3(&vec.back().offSetPos);
}

// 각도 유틸(라디안)
inline _float WrapPi(_float a) {
	a = fmodf(a + XM_PI, XM_2PI);
	if (a <= 0) a += XM_2PI;
	return a - XM_PI;
}
inline _float ShortestDelta(_float from, _float to) {
	return WrapPi(to - from);
}
inline _float LerpAngle(_float from, _float to, _float t) {
	return from + ShortestDelta(from, to) * t;
}

void CCamera_CutScene::Interp_Target(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecTargetData;
	if (vec.size() < 1) return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame < a.iKeyFrame || curFrame > b.iKeyFrame)
			continue;

		CGameObject* pTargetObj = nullptr;
		if (a.eTarget != TARGET_CAMERA::NONE)
		{
			switch (a.eTarget)
			{
			case TARGET_CAMERA::PLAYER:
				pTargetObj = m_pGameInstance->Get_LastObject(
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player"));
				break;
			default:
				break;
			}
			if (!pTargetObj) return;

			const float denom = float(max(1, b.iKeyFrame - a.iKeyFrame));
			const float t = float(curFrame - a.iKeyFrame) / denom;

			_float fPitch = LerpFloat(a.fPitch, b.fPitch, t);      // Pitch는 범위가 좁으면 일반 Lerp로 충분
			_float fYaw = LerpAngle(a.fYaw, b.fYaw, t);        //  Yaw는 짧은 경로로 보간

			_float fDistance = LerpFloat(a.fDistance, b.fDistance, t);

			// 기준점(플레이어 머리 위 약간 + 뒤로 약간)
			_vector vtargetPos = pTargetObj->Get_TransfomCom()->Get_State(STATE::POSITION);
			vtargetPos += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
			vtargetPos += XMVector3Normalize(
				pTargetObj->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

			// 구면좌표 → 오프셋
			const _float x = fDistance * cosf(fPitch) * sinf(fYaw);
			const _float y = fDistance * sinf(fPitch);
			const _float z = fDistance * cosf(fPitch) * cosf(fYaw);
			const _vector vOffset = XMVectorSet(x, y, z, 0.f);

			const _vector vTargetCamPos = vtargetPos + vOffset;

			// 위치 보간 + 스냅
			const _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vNewPos = XMVectorLerp(vCurPos, vTargetCamPos, t);
			if (XMVectorGetX(XMVector3LengthSq(vNewPos - vTargetCamPos)) < 1e-6f)
				vNewPos = vTargetCamPos;

			m_pTransformCom->Set_State(STATE::POSITION, vNewPos);
			m_pTransformCom->LookAt(vtargetPos);
			return;
		}
	}
}
XMVECTOR CCamera_CutScene::XMMatrixDecompose_T(const _matrix& m)
{
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, m);
	return trans;
}

HRESULT CCamera_CutScene::InitDatas()
{
	m_CutSceneDatas.clear();

	ifstream inFile("../Bin/Save/CutScene/WakeUp.json");
	if (inFile.is_open())
	{
		json j;
		inFile >> j;
		inFile.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::WAKEUP, LoadCameraFrameData(j)));
	}

	ifstream inFile2("../Bin/Save/CutScene/TutorialDoor.json");
	if (inFile2.is_open())
	{
		json j;
		inFile2 >> j;
		inFile2.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::TUTORIALDOOR, LoadCameraFrameData(j)));
	}

	ifstream inFile3("../Bin/Save/CutScene/OutDoor.json");
	if (inFile3.is_open())
	{
		json j;
		inFile3 >> j;
		inFile3.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::OUTDOOOR, LoadCameraFrameData(j)));
	}

	ifstream inFile4("../Bin/Save/CutScene/FuocoDoor.json");
	if (inFile4.is_open())
	{
		json j;
		inFile4 >> j;
		inFile4.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::FUOCO, LoadCameraFrameData(j)));
	}

	ifstream inFile5("../Bin/Save/CutScene/FestivalDoor.json");
	if (inFile5.is_open())
	{
		json j;
		inFile5 >> j;
		inFile5.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::FESTIVAL, LoadCameraFrameData(j)));
	}

	return S_OK;
}

CAMERA_FRAMEDATA CCamera_CutScene::LoadCameraFrameData(const json& j)
{
	CAMERA_FRAMEDATA data;

	data.iEndFrame = j.value("iEndFrame", 0);
	data.bOrbitalToSetOrbital = j.value("bStartBlend", false);
	data.bReadyCutSceneOrbital = j.value("bEndBlend", false);
	data.fPitch = j["Pitch"].get<float>();
	data.fYaw = j["Yaw"].get<float>();

	// 1. vecMatrixPosData
	if (j.contains("vecMatrixPosData"))
	{
		for (const auto& posJson : j["vecMatrixPosData"])
		{
			CAMERA_WORLDFRAME posFrame;
			posFrame.iKeyFrame = posJson["keyFrame"];
			posFrame.interpMatrixPos = posJson["interpMatrixPosition"];

			const std::vector<float>& matValues = posJson["worldMatrix"];
			XMFLOAT4X4 mat;
			memcpy(&mat, matValues.data(), sizeof(float) * 16);
			posFrame.WorldMatrix = XMLoadFloat4x4(&mat);

			data.vecWorldMatrixData.push_back(posFrame);
		}
	}

	// 2. vecPosData
	if (j.contains("vecPosData"))
	{
		for (const auto& rotJson : j["vecPosData"])
		{
			CAMERA_POSFRAME posFrame;
			posFrame.iKeyFrame = rotJson["keyFrame"];
			posFrame.offSetPos = XMFLOAT3(
				rotJson["position"][0],
				rotJson["position"][1],
				rotJson["position"][2]
			);
			posFrame.interpOffSetPos = rotJson["interpPosition"];

			data.vecOffSetPosData.push_back(posFrame);
		}
	}

	// 3. vecRotData
	if (j.contains("vecRotData"))
	{
		for (const auto& rotJson : j["vecRotData"])
		{
			CAMERA_ROTFRAME rotFrame;
			rotFrame.iKeyFrame = rotJson["keyFrame"];
			rotFrame.offSetRot = XMFLOAT3(
				rotJson["rotation"][0],
				rotJson["rotation"][1],
				rotJson["rotation"][2]
			);
			rotFrame.interpOffSetRot = rotJson["interpRotation"];

			data.vecOffSetRotData.push_back(rotFrame);
		}
	}

	// 4. vecFovData
	if (j.contains("vecFovData"))
	{
		for (const auto& fovJson : j["vecFovData"])
		{
			CAMERA_FOVFRAME fovFrame;
			fovFrame.iKeyFrame = fovJson["keyFrame"];
			fovFrame.fFov = fovJson["fFov"];
			fovFrame.interpFov = fovJson["interpFov"];

			data.vecFovData.push_back(fovFrame);
		}
	}

	// 5. vecTargetData
	if (j.contains("vecTargetData"))
	{
		for (const auto& fovJson : j["vecTargetData"])
		{
			CAMERA_TARGETFRAME targetFrame;
			targetFrame.iKeyFrame = fovJson["keyFrame"];
			targetFrame.eTarget = static_cast<TARGET_CAMERA>(fovJson["targetType"]);
			targetFrame.fPitch = fovJson["pitch"];
			targetFrame.fYaw = fovJson["yaw"];
			targetFrame.fDistance = fovJson["distance"];

			data.vecTargetData.push_back(targetFrame);
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
