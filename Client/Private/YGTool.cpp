#include "YGTool.h"
#include "GameInstance.h"
#include "Client_Calculation.h"
//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감
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
	return S_OK;
}

void CYGTool::Priority_Update(_float fTimeDelta)
{

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

	// 1. Position Frame (WorldMatrix은 16개의 float 값으로 저장)
	for (const auto& pos : data.vecWorldMatrixData)
	{
		XMFLOAT4X4 mat;
		XMStoreFloat4x4(&mat, pos.WorldMatrix);

		std::vector<float> matValues(16);
		memcpy(matValues.data(), &mat, sizeof(float) * 16);

		j["vecPosData"].push_back({
			{ "keyFrame", pos.keyFrame },
			{ "worldMatrix", matValues },
			{ "interpPosition", pos.interpPosition }
			});
	}

	// 2. Rotation Frame
	for (const auto& rot : data.vecRotData)
	{
		j["vecRotData"].push_back({
			{ "keyFrame", rot.keyFrame },
			{ "rotation", { rot.offSetRot.x, rot.offSetRot.y, rot.offSetRot.z } },
			{ "interpRotation", rot.interpRotation }
			});
	}

	// 3. FOV Frame
	for (const auto& fov : data.vecFovData)
	{
		j["vecFovData"].push_back({
			{ "keyFrame", fov.keyFrame },
			{ "fFov", fov.fFov },
			{ "interpFov", fov.interpFov }
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

HRESULT CYGTool::Render_CameraTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Sequence Tool", &open, NULL);
	if (ImGui::CollapsingHeader("Sequence Info"))
	{
		ImGui::InputInt("End Frame", &m_iEndFrame, 10, 0);
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
				m_CameraDatas.vecRotData.clear();
				m_CameraDatas.vecFovData.clear();
				m_pSelectedKey = nullptr;
				
				m_CameraDatas = LoadCameraFrameData(j);
				for (auto& pos : m_CameraDatas.vecWorldMatrixData)
				{
					m_CameraSequence->Add_KeyFrame(0, pos.keyFrame);
				}

				for (auto& pos : m_CameraDatas.vecRotData)
				{
					m_CameraSequence->Add_KeyFrame(1, pos.keyFrame);
				}

				for (auto& pos : m_CameraDatas.vecFovData)
				{
					m_CameraSequence->Add_KeyFrame(2, pos.keyFrame);
				}

				m_iEndFrame = m_CameraDatas.iEndFrame;
			}
		}
	}

	ImGui::SeparatorText("=====");

	ImGui::Text("Current Frame: %d", m_iCurrentFrame);
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
			if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rot), 0.5f))
				m_pSelectedKey->rotation = rot;

			int interpPos = static_cast<int>(m_pSelectedKey->interpPosition);

			if (ImGui::Combo("Interp Position", &interpPos, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpPosition = static_cast<INTERPOLATION_CAMERA>(interpPos);
		}
		else if (m_iSelected == 1)
		{
			// Offset Rot
			XMFLOAT3 offSetRot = m_pSelectedKey->offSetRotation;
			if (ImGui::DragFloat3("Offset Rotation", reinterpret_cast<float*>(&offSetRot), 0.5f))
				m_pSelectedKey->offSetRotation = offSetRot;

			int interpRot = static_cast<int>(m_pSelectedKey->interpRotation);

			if (ImGui::Combo("Interp Rotation", &interpRot, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpRotation = static_cast<INTERPOLATION_CAMERA>(interpRot);
		}
		else if (m_iSelected == 2)
		{
			// Fov
			ImGui::DragFloat("FOV", &m_pSelectedKey->fFov, 0.1f, 1.0f, 179.0f);

			int interpFov = static_cast<int>(m_pSelectedKey->interpFov);

			if (ImGui::Combo("Interp FOV", &interpFov, interpNames, IM_ARRAYSIZE(interpNames)))
				m_pSelectedKey->interpFov = static_cast<INTERPOLATION_CAMERA>(interpFov);
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
					if (keyframe.keyFrame == m_pSelectedKey->keyFrame)
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

					CAMERA_WORLDFRAME posFrame;
					posFrame.keyFrame = m_pSelectedKey->keyFrame;
					posFrame.WorldMatrix = finalMat;
					posFrame.interpPosition = m_pSelectedKey->interpPosition;
					m_CameraDatas.vecWorldMatrixData.push_back(posFrame);
					m_CameraSequence->Add_KeyFrame(0, m_pSelectedKey->keyFrame);
				}
			}
				break;
			case 1:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecRotData)
				{
					if (keyframe.keyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					CAMERA_ROTFRAME rotFrame;
					rotFrame.keyFrame = m_pSelectedKey->keyFrame;
					rotFrame.offSetRot = m_pSelectedKey->offSetRotation;
					rotFrame.interpRotation = m_pSelectedKey->interpRotation;
					m_CameraDatas.vecRotData.push_back(rotFrame);
					m_CameraSequence->Add_KeyFrame(1, m_pSelectedKey->keyFrame);
				}
			}
				break;
			case 2:
			{
				_bool exist = false;
				for (auto& keyframe : m_CameraDatas.vecFovData)
				{
					if (keyframe.keyFrame == m_pSelectedKey->keyFrame)
					{
						exist = true;
						break;
					}
				}

				if (!exist)
				{
					CAMERA_FOVFRAME fovFrame;
					fovFrame.keyFrame = m_pSelectedKey->keyFrame;
					fovFrame.fFov = m_pSelectedKey->fFov;
					fovFrame.interpFov = m_pSelectedKey->interpFov;
					m_CameraDatas.vecFovData.push_back(fovFrame);
					m_CameraSequence->Add_KeyFrame(2, m_pSelectedKey->keyFrame);
				}
			}
				break;
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
}

HRESULT CYGTool::Render_CameraFrame()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Camera Frame", &open, NULL);
	ImGui::Text("CutScene Frames:");

	if (ImGui::CollapsingHeader("WorldPosRot Info"))
	{
		ImGui::BeginChild("PosFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecWorldMatrixData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecWorldMatrixData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditPosKey = m_CameraDatas.vecWorldMatrixData[i];
				m_EditRotKey = {};
				m_EditFovKey = {};
				m_iChangeKeyFrame = m_EditPosKey.keyFrame;
			}
		}
		ImGui::EndChild();


		ImGui::SeparatorText("Edit Pos Key Info");

		_float4x4 worldMat;
		XMStoreFloat4x4(&worldMat, m_EditPosKey.WorldMatrix);

		_float matrix[16];
		memcpy(matrix, &worldMat, sizeof(float) * 16);

		_float position[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

		// 출력용 텍스트
		ImGui::Text("Position: %.2f, %.2f, %.2f", position[0], position[1], position[2]);
		ImGui::Text("Rotation: %.2f, %.2f, %.2f", rotation[0], rotation[1], rotation[2]);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpPos = static_cast<int>(m_EditPosKey.interpPosition);
		if (ImGui::Combo("Pos Interp", &interpPos, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditPosKey.interpPosition = static_cast<INTERPOLATION_CAMERA>(interpPos);

		if (ImGui::Button("Clone Camera"))
		{
			if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
			{
				CTransform* pTransform = pCam->Get_TransfomCom();
				m_EditPosKey.WorldMatrix = pTransform->Get_WorldMatrix();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Set Camera Same"))
		{
			if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
			{
				CTransform* pTransform = pCam->Get_TransfomCom();
				pTransform->Set_WorldMatrix(m_EditPosKey.WorldMatrix);
			}
		}

		ImGui::DragInt("WorldPosRot Key", &m_iChangeKeyFrame);
		if (ImGui::Button("WorldPosRot Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(0, m_EditPosKey.keyFrame, m_iChangeKeyFrame);
			m_EditPosKey.keyFrame = m_iChangeKeyFrame;
		}

		if (ImGui::Button("WorldPosRot Apply"))
		{
			m_CameraDatas.vecWorldMatrixData[m_iEditKey] = m_EditPosKey;
		}
		ImGui::SameLine();

		if (ImGui::Button("WorldPosRot Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecWorldMatrixData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(0, m_CameraDatas.vecWorldMatrixData[m_iEditKey].keyFrame);
				m_CameraDatas.vecWorldMatrixData.erase(m_CameraDatas.vecWorldMatrixData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditPosKey = {};
			}
		}
	}

	if (ImGui::CollapsingHeader("OffsetRot Info"))
	{
		ImGui::BeginChild("OffsetRotFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// 리스트 출력
		for (size_t i = 0; i < m_CameraDatas.vecRotData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecRotData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditPosKey = {};
				m_EditRotKey = m_CameraDatas.vecRotData[i];;
				m_EditFovKey = {};
				m_iChangeKeyFrame = m_EditRotKey.keyFrame;
			}
		}
		ImGui::EndChild();

		ImGui::SeparatorText("Edit OffsetRot Key Info");

		ImGui::DragFloat3("OffsetRot", reinterpret_cast<float*>(&m_EditRotKey.offSetRot), 0.1f);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpOffsetRot = static_cast<int>(m_EditRotKey.interpRotation);
		if (ImGui::Combo("OffsetRot Interp", &interpOffsetRot, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditRotKey.interpRotation = static_cast<INTERPOLATION_CAMERA>(interpOffsetRot);

		ImGui::DragInt("OffsetRot Key", &m_iChangeKeyFrame);
		if (ImGui::Button("OffsetRot Change KeyFrame"))
		{
			m_CameraSequence->Change_KeyFrame(1, m_EditRotKey.keyFrame, m_iChangeKeyFrame);
			m_EditRotKey.keyFrame = m_iChangeKeyFrame;
		}

		if (ImGui::Button("OffsetRot Apply"))
		{
			m_CameraDatas.vecRotData[m_iEditKey] = m_EditRotKey;
		}

		if (ImGui::Button("OffsetRot Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecRotData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(1, m_CameraDatas.vecRotData[m_iEditKey].keyFrame);
				m_CameraDatas.vecRotData.erase(m_CameraDatas.vecRotData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditRotKey = {};
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
			sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// 선택됨 → 인덱스 및 포인터 저장
				m_iEditKey = static_cast<int>(i);
				m_EditPosKey = {};
				m_EditRotKey = {};
				m_EditFovKey = m_CameraDatas.vecFovData[i];
				m_iChangeKeyFrame = m_EditFovKey.keyFrame;
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
			m_CameraSequence->Change_KeyFrame(1, m_EditFovKey.keyFrame, m_iChangeKeyFrame);
			m_EditFovKey.keyFrame = m_iChangeKeyFrame;
		}

		if (ImGui::Button("Fov Apply"))
		{
			m_CameraDatas.vecFovData[m_iEditKey] = m_EditFovKey;
		}

		if (ImGui::Button("Fov Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecFovData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(2, m_CameraDatas.vecFovData[m_iEditKey].keyFrame);
				m_CameraDatas.vecFovData.erase(m_CameraDatas.vecFovData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditFovKey = {};
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
