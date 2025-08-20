#include "YGTool.h"
#include "GameInstance.h"
#include "Client_Calculation.h"

#include "Camera.h"
#include "Camera_Manager.h"

#include "Camera_CutScene.h"
CYGTool::CYGTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CYGTool::CYGTool(const CYGTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CYGTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CYGTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_CameraSequence = new CCameraSequence();
	m_CameraSequence->m_iFrameMin = 0;
	m_CameraSequence->m_iFrameMax = 2000;
	m_CameraSequence->Add(0,10,0);
	m_CameraSequence->Add(0,10,1);
	m_CameraSequence->Add(0,10,2);
	m_CameraSequence->Add(0,10,3);
	m_CameraSequence->Add(0,10,4);
	return S_OK;
}

void CYGTool::Priority_Update(_float fTimeDelta)
{
	ShowCursor(TRUE);
}

void CYGTool::Update(_float fTimeDelta)
{
}

void CYGTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CYGTool::Render()
{

	if (FAILED(Render_CameraTool()))
		return E_FAIL;

	if (FAILED(Render_CameraFrame()))
		return E_FAIL;

	if (FAILED(Render_CameraSequence()))
		return E_FAIL;

	return S_OK;
}

json CYGTool::SaveCameraFrameData(const CAMERA_FRAMEDATA& data)
{
	json j;

	j["iEndFrame"] = data.iEndFrame;
	j["bStartBlend"] = data.bOrbitalToSetOrbital;
	j["bEndBlend"] = data.bReadyCutSceneOrbital;
	j["Pitch"] = data.fPitch;
	j["Yaw"] = data.fYaw;

	// 1. Matrix Position Frame (WorldMatrix은 16개의 float 값으로 저장)
	for (const auto& matrixPos : data.vecWorldMatrixData)
	{
		XMFLOAT4X4 mat;
		XMStoreFloat4x4(&mat, matrixPos.WorldMatrix);

		std::vector<float> matValues(16);
		memcpy(matValues.data(), &mat, sizeof(float) * 16);

		j["vecMatrixPosData"].push_back({
			{ "keyFrame", matrixPos.iKeyFrame },
			{ "worldMatrix", matValues },
			{ "interpMatrixPosition", matrixPos.interpMatrixPos }
			});
	}

	// 2. Offset Position Frame
	for (const auto& pos : data.vecOffSetPosData)
	{
		j["vecPosData"].push_back({
			{ "keyFrame", pos.iKeyFrame },
			{ "position", { pos.offSetPos.x, pos.offSetPos.y, pos.offSetPos.z } },
			{ "interpPosition", pos.interpOffSetPos }
			});
	}

	// 3. Offset Rotation Frame
	for (const auto& rot : data.vecOffSetRotData)
	{
		j["vecRotData"].push_back({
			{ "keyFrame", rot.iKeyFrame },
			{ "rotation", { rot.offSetRot.x, rot.offSetRot.y, rot.offSetRot.z } },
			{ "interpRotation", rot.interpOffSetRot }
			});
	}

	// 4. FOV Frame
	for (const auto& fov : data.vecFovData)
	{
		j["vecFovData"].push_back({
			{ "keyFrame", fov.iKeyFrame },
			{ "fFov", fov.fFov },
			{ "interpFov", fov.interpFov }
			});
	}

	// 5. Target Frame
	for (const auto& target : data.vecTargetData)
	{
		j["vecTargetData"].push_back({
			{ "keyFrame", target.iKeyFrame },
			{ "targetType", static_cast<_int>(target.eTarget)},
			{ "pitch", target.fPitch },
			{ "yaw", target.fYaw },
			{ "distance", target.fDistance }
			});
	}

	return j;
}


void CYGTool::SaveToJsonFile(const std::string& filePath, const CAMERA_FRAMEDATA& data)
{
	json j = SaveCameraFrameData(data);

	std::ofstream outFile(filePath);
	if (outFile.is_open())
	{
		outFile << std::setw(4) << j;  // 보기 좋게 들여쓰기
		outFile.close();
	}
	else
	{
		MessageBoxA(nullptr, "JSON 파일 저장 실패", "에러", MB_OK);
	}
}

CAMERA_FRAMEDATA CYGTool::LoadCameraFrameData(const json& j)
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

HRESULT CYGTool::Render_CameraTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Sequence Tool", &open, NULL);
	if (ImGui::CollapsingHeader("Sequence Info"))
	{
		ImGui::InputInt("End Frame", &m_iEndFrame, 1, 0);
		m_CameraSequence->Set_EndFrame(m_iEndFrame);
		m_CameraDatas.iEndFrame = m_iEndFrame;

		const char* CutsceneTypeNames[] = { "WakeUp", "TutorialDoor", "THREE" };
		int currentCutsceneType = static_cast<int>(m_eCutSceneType); // 현재 값 저장

		if (ImGui::Combo("Load Type", &currentCutsceneType, CutsceneTypeNames, IM_ARRAYSIZE(CutsceneTypeNames)))
		{
			m_eCutSceneType = static_cast<CUTSCENE_TYPE>(currentCutsceneType);
		}

		if (ImGui::Button("Load Data"))
		{
			string filePath;
			switch (m_eCutSceneType)
			{
			case Client::CUTSCENE_TYPE::WAKEUP:
				filePath = "../Bin/Save/CutScene/WakeUp.json";
				break;
			case Client::CUTSCENE_TYPE::TUTORIALDOOR:
				filePath = "../Bin/Save/CutScene/TutorialDoor.json";
				break;
			case Client::CUTSCENE_TYPE::THREE:
				filePath = "../Bin/Save/CutScene/three.json";
				break;
			default:
				break;
			}
			ifstream inFile(filePath);
			if (inFile.is_open())
			{
				json j;
				inFile >> j;
				inFile.close();

				// 모든 것들 초기화
				m_CameraSequence->InitAllFrames();
				m_CameraDatas.vecWorldMatrixData.clear();
				m_CameraDatas.vecOffSetPosData.clear();
				m_CameraDatas.vecOffSetRotData.clear();
				m_CameraDatas.vecFovData.clear();
				m_CameraDatas.vecTargetData.clear();
				m_pSelectedKey = nullptr;
				
				m_CameraDatas = LoadCameraFrameData(j);
				for (auto& pos : m_CameraDatas.vecWorldMatrixData)
				{
					m_CameraSequence->Add_KeyFrame(0, pos.iKeyFrame);
				}

				for (auto& pos : m_CameraDatas.vecOffSetPosData)
				{
					m_CameraSequence->Add_KeyFrame(1, pos.iKeyFrame);
				}

				for (auto& pos : m_CameraDatas.vecOffSetRotData)
				{
					m_CameraSequence->Add_KeyFrame(2, pos.iKeyFrame);
				}

				for (auto& pos : m_CameraDatas.vecFovData)
				{
					m_CameraSequence->Add_KeyFrame(3, pos.iKeyFrame);
				}

				for (auto& pos : m_CameraDatas.vecTargetData)
				{
					m_CameraSequence->Add_KeyFrame(4, pos.iKeyFrame);
				}

				m_iEndFrame = m_CameraDatas.iEndFrame;
			}
		}
	}

	ImGui::SeparatorText("=====");

	ImGui::DragInt("Current Frame", &m_iCurrentFrame, 0, 1, m_iEndFrame);
	if (m_iCurrentFrame < m_iEndFrame)
	{
		m_pSelectedKey = m_CameraSequence->GetKeyAtFrame(m_iCurrentFrame);
		m_pSelectedKey->keyFrame = m_iCurrentFrame;
	}

	ImGui::Checkbox("StartBlend", &m_CameraDatas.bOrbitalToSetOrbital);
	ImGui::Checkbox("EndBlend", &m_CameraDatas.bReadyCutSceneOrbital);

	ImGui::SeparatorText("=====");
	if (m_pSelectedKey)
	{
		ImGui::SeparatorText("Current Key Info");

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		if (m_iSelected == 0)
		{
			// World
			XMFLOAT3 pos = m_pSelectedKey->position;
			if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
				m_pSelectedKey->position = pos;

			XMFLOAT3 rot = m_pSelectedKey->rotation;
			if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rot), 0.1f))
				m_pSelectedKey->rotation = rot;

			_int interpPos = static_cast<int>(m_pSelectedKey->interpWorldPos);

			if (ImGui::Combo("Interp Position", &interpPos, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpWorldPos = static_cast<INTERPOLATION_CAMERA>(interpPos);
		}
		else if (m_iSelected == 1)
		{
			// Offset Pos
			XMFLOAT3 offSetPos = m_pSelectedKey->offSetPosition;
			if (ImGui::DragFloat3("Offset Position", reinterpret_cast<float*>(&offSetPos), 0.05f))
				m_pSelectedKey->offSetPosition = offSetPos;

			_int interpRot = static_cast<int>(m_pSelectedKey->interpOffSetPos);

			if (ImGui::Combo("Interp Position", &interpRot, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpOffSetPos = static_cast<INTERPOLATION_CAMERA>(interpRot);
		}
		else if (m_iSelected == 2)
		{
			// Offset Rot
			XMFLOAT3 offSetRot = m_pSelectedKey->offSetRotation;
			if (ImGui::DragFloat3("Offset Rotation", reinterpret_cast<float*>(&offSetRot), 0.1f))
				m_pSelectedKey->offSetRotation = offSetRot;

			_int interpRot = static_cast<int>(m_pSelectedKey->interpOffSetRot);

			if (ImGui::Combo("Interp Rotation", &interpRot, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpOffSetRot = static_cast<INTERPOLATION_CAMERA>(interpRot);
		}
		else if (m_iSelected == 3)
		{
			// Fov
			ImGui::DragFloat("FOV", &m_pSelectedKey->fFov, 0.1f, 1.0f, 179.0f);

			_int interpFov = static_cast<int>(m_pSelectedKey->interpFov);

			if (ImGui::Combo("Interp FOV", &interpFov, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpFov = static_cast<INTERPOLATION_CAMERA>(interpFov);
		}
		else if (m_iSelected == 4)
		{
			// Target
			const char* targetNames[] = { "None", "Layer_Player", "Layer_Boss1" };
			_int target = static_cast<int>(m_pSelectedKey->eTarget);

			if (ImGui::Combo("Target", &target, targetNames, IM_ARRAYSIZE(targetNames)))
				m_pSelectedKey->eTarget = static_cast<TARGET_CAMERA>(target);


			ImGui::DragFloat("fPitch", &m_pSelectedKey->fPitch, 0.f, -89.0f, 89.0f);
			ImGui::DragFloat("fYaw", &m_pSelectedKey->fYaw, 0.f, -180.0f, 180.0f);
			ImGui::DragFloat("fDistance", &m_pSelectedKey->fDistance, 0.f, 1.0f, 179.0f);
		}
	}

	Render_SetInfos();

	if (ImGui::Button("Add KeyFrame"))
	{
		if (m_pSelectedKey)
		{
			switch (m_iSelected)
			{
			case 0:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecWorldMatrixData)
				{
					if (keyframe.iKeyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					float matrix[16];
					float scale[3] = { 1.0f, 1.0f, 1.0f };
					ImGuizmo::RecomposeMatrixFromComponents(
						reinterpret_cast<float*>(&m_pSelectedKey->position),
						reinterpret_cast<float*>(&m_pSelectedKey->rotation),
						scale,
						matrix
					);

					// 2. float[16] → _float4x4
					_float4x4 worldMat;
					std::memcpy(&worldMat, matrix, sizeof(float) * 16);
					_matrix finalMat = XMLoadFloat4x4(&worldMat);

					CAMERA_WORLDFRAME matrixFrame;
					matrixFrame.iKeyFrame = m_pSelectedKey->keyFrame;
					matrixFrame.WorldMatrix = finalMat;
					matrixFrame.interpMatrixPos = m_pSelectedKey->interpWorldPos;

					auto& v = m_CameraDatas.vecWorldMatrixData;
					auto it = std::lower_bound(
						v.begin(), v.end(), matrixFrame.iKeyFrame,
						[](const CAMERA_WORLDFRAME& a, int key) { return a.iKeyFrame < key; });
					v.insert(it, matrixFrame);

					m_CameraSequence->Add_KeyFrame(0, m_pSelectedKey->keyFrame);
				}
				break;
			}
			case 1:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecOffSetPosData)
				{
					if (keyframe.iKeyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					CAMERA_POSFRAME posFrame;
					posFrame.iKeyFrame = m_pSelectedKey->keyFrame;
					posFrame.offSetPos = m_pSelectedKey->offSetPosition;
					posFrame.interpOffSetPos = m_pSelectedKey->interpOffSetPos;

					auto& v = m_CameraDatas.vecOffSetPosData;
					auto it = std::lower_bound(
						v.begin(), v.end(), posFrame.iKeyFrame,
						[](const CAMERA_POSFRAME& a, int key) { return a.iKeyFrame < key; });
					v.insert(it, posFrame);

					m_CameraSequence->Add_KeyFrame(1, m_pSelectedKey->keyFrame);
				}
				break;
			}
			case 2:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecOffSetRotData)
				{
					if (keyframe.iKeyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					CAMERA_ROTFRAME rotFrame;
					rotFrame.iKeyFrame = m_pSelectedKey->keyFrame;
					rotFrame.offSetRot = m_pSelectedKey->offSetRotation;
					rotFrame.interpOffSetRot = m_pSelectedKey->interpOffSetRot;

					auto& v = m_CameraDatas.vecOffSetRotData;
					auto it = std::lower_bound(
						v.begin(), v.end(), rotFrame.iKeyFrame,
						[](const CAMERA_ROTFRAME& a, int key) { return a.iKeyFrame < key; });
					v.insert(it, rotFrame);

					m_CameraSequence->Add_KeyFrame(2, m_pSelectedKey->keyFrame);
				}
				break;
			}
			case 3:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecFovData)
				{
					if (keyframe.iKeyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					CAMERA_FOVFRAME fovFrame;
					fovFrame.iKeyFrame = m_pSelectedKey->keyFrame;
					fovFrame.fFov = m_pSelectedKey->fFov;
					fovFrame.interpFov = m_pSelectedKey->interpFov;

					auto& v = m_CameraDatas.vecFovData;
					auto it = std::lower_bound(
						v.begin(), v.end(), fovFrame.iKeyFrame,
						[](const CAMERA_FOVFRAME& a, int key) { return a.iKeyFrame < key; });
					v.insert(it, fovFrame);

					m_CameraSequence->Add_KeyFrame(3, m_pSelectedKey->keyFrame);
				}
				break;
			}
			case 4:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecTargetData)
				{
					if (keyframe.iKeyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					CAMERA_TARGETFRAME targetFrame;
					targetFrame.iKeyFrame = m_pSelectedKey->keyFrame;
					targetFrame.eTarget = m_pSelectedKey->eTarget;
					targetFrame.fPitch = m_pSelectedKey->fPitch;
					targetFrame.fYaw = m_pSelectedKey->fYaw;
					targetFrame.fDistance = m_pSelectedKey->fDistance;

					auto& v = m_CameraDatas.vecTargetData;
					auto it = std::lower_bound(
						v.begin(), v.end(), targetFrame.iKeyFrame,
						[](const CAMERA_TARGETFRAME& a, int key) { return a.iKeyFrame < key; });
					v.insert(it, targetFrame);

					m_CameraSequence->Add_KeyFrame(4, m_pSelectedKey->keyFrame);
				}
				break;
			}
			default:
				break;
			}
		}
	}

	if (ImGui::Button("Apply Pitch Yaw"))
	{
		m_CameraDatas.fPitch = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_Pitch();
		m_CameraDatas.fYaw = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_Yaw();
	}
	ImGui::Text("Pitch: %f", CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_Pitch());
	ImGui::Text("Yaw: %f", CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_Yaw());

	if (ImGui::Button("Play CutScene"))
	{
		CCamera_Orbital* pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();
		CCamera_CutScene* pCamera_CutScene = CCamera_Manager::Get_Instance()->GetCutScene();

		pCamera_CutScene->Set_InitOrbitalWorldMatrix(pCamera_Orbital->Get_OrbitalWorldMatrix(m_CameraDatas.fPitch, m_CameraDatas.fYaw));
		_matrix oribtalMatrix = pCamera_Orbital->Get_TransfomCom()->Get_WorldMatrix();
		pCamera_CutScene->Get_TransfomCom()->Set_WorldMatrix(oribtalMatrix);

		CCamera_Manager::Get_Instance()->SetCutSceneCam();
		CCamera_Manager::Get_Instance()->GetCutScene()->Set_CameraFrame(m_CameraDatas);
		CCamera_Manager::Get_Instance()->GetCutScene()->PlayCutScene();
	}

	ImGui::SameLine();
	ImGui::Text("Current Frame: %d", CCamera_Manager::Get_Instance()->GetCutScene()->Get_CurrentFrame());

	ImGui::SeparatorText("Save Data");

	const char* CutsceneTypeNames[] = { "WakeUp", "TutorialDoor", "THREE" };
	int currentCutsceneType = static_cast<int>(m_eCutSceneType); // 현재 값 저장

	ImGui::SeparatorText("Cutscene Type");
	if (ImGui::Combo("Type", &currentCutsceneType, CutsceneTypeNames, IM_ARRAYSIZE(CutsceneTypeNames)))
	{
		m_eCutSceneType = static_cast<CUTSCENE_TYPE>(currentCutsceneType);
	}

	if (ImGui::Button("Save"))
	{
		string filePath;
		switch (m_eCutSceneType)
		{
		case Client::CUTSCENE_TYPE::WAKEUP:
			filePath = "../Bin/Save/CutScene/WakeUp.json";
			break;
		case Client::CUTSCENE_TYPE::TUTORIALDOOR:
			filePath = "../Bin/Save/CutScene/TutorialDoor.json";
			break;
		case Client::CUTSCENE_TYPE::THREE:
			filePath = "../Bin/Save/CutScene/three.json";
			break;
		default:
			break;
		}
		SaveToJsonFile(filePath, m_CameraDatas);
	}

	ImGui::End();
	return S_OK;
}

void CYGTool::Render_SetInfos()
{
	if (m_pSelectedKey && (m_iSelected == 0 || m_iEditKey != -1))
	{
		ImGui::SeparatorText("Current Camera Info");

		if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
		{
			CTransform* pTransform = pCam->Get_TransfomCom();

			_float4x4 worldMat;
			XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());

			_float matrix[16];
			memcpy(matrix, &worldMat, sizeof(float) * 16);

			_float position[3], rotation[3], scale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

			ImGui::DragFloat3("Camera Position", (float*)&position, 0.1f);
			ImGui::DragFloat3("Camera Rotation (Euler)", (float*)&rotation, 0.1f);

			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(float) * 16);
			_matrix finalMat = XMLoadFloat4x4(&worldMat);

			// 5. 적용
			pTransform->Set_WorldMatrix(finalMat);

			if (ImGui::Button("Clone Camera"))
			{
				m_pSelectedKey->position = XMFLOAT3(position[0], position[1], position[2]);
				m_pSelectedKey->rotation = XMFLOAT3(rotation[0], rotation[1], rotation[2]);
			}
		}
	}

	if (m_pSelectedKey && m_iSelected == 4)
	{
		if (ImGui::Button("Set TargetMatrix"))
		{
			CGameObject* pTargetObj = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YG), TEXT("Layer_Player"));
			_float fPitch = XMConvertToRadians(m_pSelectedKey->fPitch);
			_float fYaw = XMConvertToRadians(m_pSelectedKey->fYaw);

			_vector vtargetPos;
			// 기준점 위치 계산 (플레이어 + 높이 + 조금 뒤에)
			vtargetPos = pTargetObj->Get_TransfomCom()->Get_State(STATE::POSITION);
			vtargetPos += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
			vtargetPos += XMVector3Normalize(pTargetObj->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

			// 방향 계산
			_float x = m_pSelectedKey->fDistance * cosf(fPitch) * sinf(fYaw);
			_float y = m_pSelectedKey->fDistance * sinf(fPitch);
			_float z = m_pSelectedKey->fDistance * cosf(fPitch) * cosf(fYaw);
			_vector vOffset = XMVectorSet(x, y, z, 0.f);

			// 목표 카메라 위치
			_vector vTargetCamPos = vtargetPos + vOffset;

			CCamera_Manager::Get_Instance()->GetFreeCam()->Get_TransfomCom()->Set_State(STATE::POSITION, vTargetCamPos);
			CCamera_Manager::Get_Instance()->GetFreeCam()->Get_TransfomCom()->LookAt(vtargetPos);
		}
	}
	
}

HRESULT CYGTool::Render_CameraFrame()
{
	ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiWindowFlags_HorizontalScrollbar);
	_bool open = true;
	ImGui::Begin("Camera Frame", &open, NULL);

	if (auto* cut = CCamera_Manager::Get_Instance()->GetCutScene())
	{
		_int cur = cut->Get_CurrentFrame();       
		ImGui::Text("CutScene Frame: %d", cur);
	}

	if (ImGui::CollapsingHeader("WorldPosRot Info"))
	{
		ImGui::BeginChild("PosFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecWorldMatrixData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecWorldMatrixData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.iKeyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditMatrixPosKey = m_CameraDatas.vecWorldMatrixData[i];
				m_EditOffSetPosKey = {};
				m_EditOffSetRotKey = {};
				m_EditFovKey = {};
				m_EditTargetKey = {};
				m_iChangeKeyFrame = m_EditMatrixPosKey.iKeyFrame;
			}
		}
		ImGui::EndChild();


		ImGui::SeparatorText("Edit Pos Key Info");

		_float4x4 worldMat;
		XMStoreFloat4x4(&worldMat, m_EditMatrixPosKey.WorldMatrix);

		_float matrix[16];
		memcpy(matrix, &worldMat, sizeof(float) * 16);

		_float position[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

		// 출력용 텍스트
		ImGui::Text("Position: %.2f, %.2f, %.2f", position[0], position[1], position[2]);
		ImGui::Text("Rotation: %.2f, %.2f, %.2f", rotation[0], rotation[1], rotation[2]);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpPos = static_cast<int>(m_EditMatrixPosKey.interpMatrixPos);
		if (ImGui::Combo("Pos Interp", &interpPos, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditMatrixPosKey.interpMatrixPos = static_cast<INTERPOLATION_CAMERA>(interpPos);

		if (ImGui::Button("Clone Camera"))
		{
			if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
			{
				CTransform* pTransform = pCam->Get_TransfomCom();
				m_EditMatrixPosKey.WorldMatrix = pTransform->Get_WorldMatrix();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Set Camera Same"))
		{
			if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
			{
				CTransform* pTransform = pCam->Get_TransfomCom();
				pTransform->Set_WorldMatrix(m_EditMatrixPosKey.WorldMatrix);
			}
		}

		ImGui::DragInt("WorldPosRot Key", &m_iChangeKeyFrame);
		if (ImGui::Button("WorldPosRot Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(0, m_EditMatrixPosKey.iKeyFrame, m_iChangeKeyFrame);
			m_EditMatrixPosKey.iKeyFrame = m_iChangeKeyFrame;
			m_CameraDatas.vecWorldMatrixData[m_iEditKey].iKeyFrame = m_EditMatrixPosKey.iKeyFrame;
		}

		if (ImGui::Button("WorldPosRot Apply"))
		{
			m_CameraDatas.vecWorldMatrixData[m_iEditKey] = m_EditMatrixPosKey;
		}
		ImGui::SameLine();

		if (ImGui::Button("WorldPosRot Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecWorldMatrixData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(0, m_CameraDatas.vecWorldMatrixData[m_iEditKey].iKeyFrame);
				m_CameraDatas.vecWorldMatrixData.erase(m_CameraDatas.vecWorldMatrixData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditMatrixPosKey = {};
			}
		}
	}

	if (ImGui::CollapsingHeader("OffsetPos Info"))
	{
		ImGui::BeginChild("OffsetPosFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecOffSetPosData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecOffSetPosData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.iKeyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditMatrixPosKey = {};
				m_EditOffSetPosKey = m_CameraDatas.vecOffSetPosData[i];
				m_EditOffSetRotKey = {};
				m_EditFovKey = {};
				m_EditTargetKey = {};
				m_iChangeKeyFrame = m_EditOffSetPosKey.iKeyFrame;
			}
		}
		ImGui::EndChild();

		ImGui::SeparatorText("Edit OffsetPos Key Info");

		ImGui::DragFloat3("Offset Position", reinterpret_cast<float*>(&m_EditOffSetPosKey.offSetPos), 0.1f);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpOffsetRot = static_cast<int>(m_EditOffSetPosKey.interpOffSetPos);
		if (ImGui::Combo("OffsetRot Interp", &interpOffsetRot, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditOffSetPosKey.interpOffSetPos = static_cast<INTERPOLATION_CAMERA>(interpOffsetRot);

		ImGui::DragInt("OffsetPos Key", &m_iChangeKeyFrame);
		if (ImGui::Button("OffsetPos Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(1, m_EditOffSetPosKey.iKeyFrame, m_iChangeKeyFrame);
			m_EditOffSetPosKey.iKeyFrame = m_iChangeKeyFrame;
			m_CameraDatas.vecOffSetPosData[m_iEditKey].iKeyFrame = m_EditOffSetPosKey.iKeyFrame;
		}

		if (ImGui::Button("OffsetPos Apply"))
		{
			m_CameraDatas.vecOffSetPosData[m_iEditKey] = m_EditOffSetPosKey;
		}
		ImGui::SameLine();
		if (ImGui::Button("OffsetPos Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecOffSetPosData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(1, m_CameraDatas.vecOffSetPosData[m_iEditKey].iKeyFrame);
				m_CameraDatas.vecOffSetPosData.erase(m_CameraDatas.vecOffSetPosData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditOffSetPosKey = {};
			}
		}
	}

	if (ImGui::CollapsingHeader("OffsetRot Info"))
	{
		ImGui::BeginChild("OffsetRotFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecOffSetRotData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecOffSetRotData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.iKeyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditMatrixPosKey = {};
				m_EditOffSetPosKey = {};
				m_EditOffSetRotKey = m_CameraDatas.vecOffSetRotData[i];;
				m_EditFovKey = {};
				m_EditTargetKey = {};
				m_iChangeKeyFrame = m_EditOffSetRotKey.iKeyFrame;
			}
		}
		ImGui::EndChild();

		ImGui::SeparatorText("Edit OffsetRot Key Info");

		ImGui::DragFloat3("OffsetRot", reinterpret_cast<float*>(&m_EditOffSetRotKey.offSetRot), 0.1f);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpOffsetRot = static_cast<int>(m_EditOffSetRotKey.interpOffSetRot);
		if (ImGui::Combo("OffsetRot Interp", &interpOffsetRot, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditOffSetRotKey.interpOffSetRot = static_cast<INTERPOLATION_CAMERA>(interpOffsetRot);

		ImGui::DragInt("OffsetRot Key", &m_iChangeKeyFrame);
		if (ImGui::Button("OffsetRot Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(2, m_EditOffSetRotKey.iKeyFrame, m_iChangeKeyFrame);
			m_EditOffSetRotKey.iKeyFrame = m_iChangeKeyFrame;
			m_CameraDatas.vecOffSetRotData[m_iEditKey].iKeyFrame = m_EditOffSetRotKey.iKeyFrame;
		}

		if (ImGui::Button("OffsetRot Apply"))
		{
			m_CameraDatas.vecOffSetRotData[m_iEditKey] = m_EditOffSetRotKey;
		}
		ImGui::SameLine();
		if (ImGui::Button("OffsetRot Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecOffSetRotData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(2, m_CameraDatas.vecOffSetRotData[m_iEditKey].iKeyFrame);
				m_CameraDatas.vecOffSetRotData.erase(m_CameraDatas.vecOffSetRotData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditOffSetRotKey = {};
			}
		}
	}

	if (ImGui::CollapsingHeader("Fov Info"))
	{
		ImGui::BeginChild("FovFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecFovData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecFovData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.iKeyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditMatrixPosKey = {};
				m_EditOffSetPosKey = {};
				m_EditOffSetRotKey = {};
				m_EditFovKey = m_CameraDatas.vecFovData[i];
				m_EditTargetKey = {};
				m_iChangeKeyFrame = m_EditFovKey.iKeyFrame;
			}
		}
		ImGui::EndChild();


		ImGui::SeparatorText("Edit Fov Key Info");
		ImGui::DragFloat("Fov", &m_EditFovKey.fFov);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpFov = static_cast<int>(m_EditFovKey.interpFov);
		if (ImGui::Combo("Fov Interp", &interpFov, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditFovKey.interpFov = static_cast<INTERPOLATION_CAMERA>(interpFov);

		ImGui::DragInt("Fov Key", &m_iChangeKeyFrame);
		if (ImGui::Button("Fov Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(3, m_EditFovKey.iKeyFrame, m_iChangeKeyFrame);
			m_EditFovKey.iKeyFrame = m_iChangeKeyFrame;
			m_CameraDatas.vecFovData[m_iEditKey].iKeyFrame = m_EditFovKey.iKeyFrame;
		}

		if (ImGui::Button("Fov Apply"))
		{
			m_CameraDatas.vecFovData[m_iEditKey] = m_EditFovKey;
		}
		ImGui::SameLine();
		if (ImGui::Button("Fov Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecFovData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(3, m_CameraDatas.vecFovData[m_iEditKey].iKeyFrame);
				m_CameraDatas.vecFovData.erase(m_CameraDatas.vecFovData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditFovKey = {};
			}
		}
	}

	if (ImGui::CollapsingHeader("Target Info"))
	{
		ImGui::BeginChild("TargetFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecTargetData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecTargetData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.iKeyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditMatrixPosKey = {};
				m_EditOffSetPosKey = {};
				m_EditOffSetRotKey = {};
				m_EditFovKey = {};
				m_EditTargetKey = m_CameraDatas.vecTargetData[i];
				m_iChangeKeyFrame = m_EditTargetKey.iKeyFrame;
			}
		}
		ImGui::EndChild();


		ImGui::SeparatorText("Edit Target Key Info");
		ImGui::DragFloat("fPitch", &m_EditTargetKey.fPitch, 0.f, -89.0f, 89.0f);
		ImGui::DragFloat("fYaw", &m_EditTargetKey.fYaw, 0.f, -180.0f, 180.0f);

		if (ImGui::Button("Clone Pitch Yaw"))
		{
			m_EditTargetKey.fPitch = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_Pitch();
			m_EditTargetKey.fYaw = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_Yaw();
		}

		ImGui::DragFloat("fDistance", &m_EditTargetKey.fDistance, 0.f, 1.0f, 179.0f);

		const char* targetNames[] = { "None", "Layer_Player", "Layer_Boss1" };
		_int target = static_cast<int>(m_EditTargetKey.eTarget);
		if (ImGui::Combo("Target", &target, targetNames, IM_ARRAYSIZE(targetNames)))
			m_EditTargetKey.eTarget = static_cast<TARGET_CAMERA>(target);

		ImGui::DragInt("Target Key", &m_iChangeKeyFrame);
		if (ImGui::Button("Target Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(4, m_EditTargetKey.iKeyFrame, m_iChangeKeyFrame);
			m_EditTargetKey.iKeyFrame = m_iChangeKeyFrame;
			m_CameraDatas.vecTargetData[m_iEditKey].iKeyFrame = m_EditTargetKey.iKeyFrame;
		}

		if (ImGui::Button("Target Apply"))
		{
			m_CameraDatas.vecTargetData[m_iEditKey] = m_EditTargetKey;
		}
		ImGui::SameLine();
		if (ImGui::Button("Target Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecTargetData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(4, m_CameraDatas.vecTargetData[m_iEditKey].iKeyFrame);
				m_CameraDatas.vecTargetData.erase(m_CameraDatas.vecTargetData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditTargetKey = {};
			}
		}
	}

	ImGui::End();
	
	return S_OK;
}


HRESULT CYGTool::Render_CameraSequence()
{
	SetNextWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Camera Sequencer");

 	ImSequencer::Sequencer(
		m_CameraSequence,
		&m_iCurrentFrame,
		&m_bExpanded,
		&m_iSelected,
		&m_iFirstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL);

	ImGui::End();

	return S_OK;
}


CYGTool* CYGTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CYGTool* pInstance = new CYGTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CYGTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGTool::Clone(void* pArg)
{
	CYGTool* pInstance = new CYGTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CYGTool::Free()
{
	__super::Free();
	Safe_Delete(m_CameraSequence);
}
