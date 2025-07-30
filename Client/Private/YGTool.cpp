#include "YGTool.h"
#include "GameInstance.h"
#include "Client_Calculation.h"
//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��
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
	m_CameraSequence->m_iFrameMax = 300;
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

	// 1. Position Frame (WorldMatrix�� 16���� float ������ ����)
	for (const auto& pos : data.vecPosData)
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
			{ "rotation", { rot.rotation.x, rot.rotation.y, rot.rotation.z } },
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
		outFile << std::setw(4) << j;  // ���� ���� �鿩����
		outFile.close();
	}
	else
	{
		MessageBoxA(nullptr, "JSON ���� ���� ����", "����", MB_OK);
	}
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
	}

	if (ImGui::Button("Get CurrentKeyFrame"))
	{
		m_pSelectedKey = m_CameraSequence->GetKeyAtFrame(m_iCurrentFrame);
		m_pSelectedKey->keyFrame = m_iCurrentFrame;
	}

	if (m_pSelectedKey)
	{
		ImGui::SeparatorText("Current Key Info");

		// ������
		XMFLOAT3 pos = m_pSelectedKey->position;
		if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
			m_pSelectedKey->position = pos;

		// ȸ��
		XMFLOAT3 rot = m_pSelectedKey->rotation;
		if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rot), 0.5f))
			m_pSelectedKey->rotation = rot;

		XMFLOAT3 offSetRot = m_pSelectedKey->offSetRotation;
		if (ImGui::DragFloat3("Offset Rotation", reinterpret_cast<float*>(&offSetRot), 0.5f))
			m_pSelectedKey->offSetRotation = offSetRot;

		// FOV
		ImGui::DragFloat("FOV", &m_pSelectedKey->fFov, 0.1f, 1.0f, 179.0f);

		// ���� ���
		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpPos = static_cast<int>(m_pSelectedKey->interpPosition);
		int interpRot = static_cast<int>(m_pSelectedKey->interpRotation);
		int interpFov = static_cast<int>(m_pSelectedKey->interpFov);

		if (ImGui::Combo("Interp Position", &interpPos, interpNames, IM_ARRAYSIZE(interpNames)))
			m_pSelectedKey->interpPosition = static_cast<INTERPOLATION_CAMERA>(interpPos);

		if (ImGui::Combo("Interp Rotation", &interpRot, interpNames, IM_ARRAYSIZE(interpNames)))
			m_pSelectedKey->interpRotation = static_cast<INTERPOLATION_CAMERA>(interpRot);

		if (ImGui::Combo("Interp FOV", &interpFov, interpNames, IM_ARRAYSIZE(interpNames)))
			m_pSelectedKey->interpFov = static_cast<INTERPOLATION_CAMERA>(interpFov);
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
				for (auto& keyframe : m_CameraDatas.vecPosData)
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

					// 2. float[16] �� _float4x4
					_float4x4 worldMat;
					std::memcpy(&worldMat, matrix, sizeof(float) * 16);
					_matrix finalMat = XMLoadFloat4x4(&worldMat);

					CAMERA_POSFRAME posFrame;
					posFrame.keyFrame = m_pSelectedKey->keyFrame;
					posFrame.WorldMatrix = finalMat;
					posFrame.interpPosition = m_pSelectedKey->interpPosition;
					m_CameraDatas.vecPosData.push_back(posFrame);
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
					rotFrame.rotation = m_pSelectedKey->offSetRotation;
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

	if (ImGui::Button("Play CutScene"))
	{
		CCamera_Manager::Get_Instance()->GetCutScene()->Set_CameraFrame(m_CameraDatas);
		CCamera_Manager::Get_Instance()->GetCutScene()->PlayCutScene();
		CCamera_Manager::Get_Instance()->SetCutSceneCam();
	}

	ImGui::SeparatorText("Save Data");

	const char* CutsceneTypeNames[] = { "ONE", "TWO", "THREE" };
	int currentCutsceneType = static_cast<int>(m_eCutSceneType); // ���� �� ����

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
		case Client::CUTSCENE_TYPE::ONE:
			filePath = "../Bin/Save/CutScene/one.json";
			break;
		case Client::CUTSCENE_TYPE::TWO:
			filePath = "../Bin/Save/CutScene/two.json";
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
	if (m_pSelectedKey)
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

			// 5. ����
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

		// ����Ʈ ���
		for (size_t i = 0; i < m_CameraDatas.vecPosData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecPosData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// ���õ� �� �ε��� �� ������ ����
				m_iEditKey = static_cast<int>(i);
				m_EditPosKey = m_CameraDatas.vecPosData[i];
				m_EditRotKey = {};
				m_EditFovKey = {};
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

		// ��¿� �ؽ�Ʈ
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

		if (ImGui::Button("Set Camera Same"))
		{
			if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
			{
				CTransform* pTransform = pCam->Get_TransfomCom();
				pTransform->Set_WorldMatrix(m_EditPosKey.WorldMatrix);
			}
		}

		if (ImGui::Button("WorldPosRot Apply"))
		{
			m_CameraDatas.vecPosData[m_iEditKey] = m_EditPosKey;
		}

		if (ImGui::Button("WorldPosRot Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_CameraDatas.vecPosData.size()))
			{
				m_CameraSequence->Delete_KeyFrame(0, m_CameraDatas.vecPosData[m_iEditKey].keyFrame);
				m_CameraDatas.vecPosData.erase(m_CameraDatas.vecPosData.begin() + m_iEditKey);
				m_iEditKey = -1;
				m_EditPosKey = {};
			}
		}
	}

	if (ImGui::CollapsingHeader("OffsetRot Info"))
	{
		ImGui::BeginChild("OffsetRotFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		// ����Ʈ ���
		for (size_t i = 0; i < m_CameraDatas.vecRotData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecRotData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// ���õ� �� �ε��� �� ������ ����
				m_iEditKey = static_cast<int>(i);
				m_EditPosKey = {};
				m_EditRotKey = m_CameraDatas.vecRotData[i];;
				m_EditFovKey = {};
			}
		}
		ImGui::EndChild();

		ImGui::SeparatorText("Edit OffsetRot Key Info");

		ImGui::DragFloat3("OffsetRot", reinterpret_cast<float*>(&m_EditRotKey.rotation), 0.1f);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpOffsetRot = static_cast<int>(m_EditRotKey.interpRotation);
		if (ImGui::Combo("OffsetRot Interp", &interpOffsetRot, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditRotKey.interpRotation = static_cast<INTERPOLATION_CAMERA>(interpOffsetRot);

		if (ImGui::Button("OffsetRot Apply"))
		{
			m_CameraDatas.vecRotData[m_iEditKey] = m_EditRotKey;
		}

		if (ImGui::Button("Rot Delete"))
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

		// ����Ʈ ���
		for (size_t i = 0; i < m_CameraDatas.vecFovData.size(); ++i)
		{
			const auto& desc = m_CameraDatas.vecFovData[i];
			char label[32];
			sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

			bool bSelected = (m_iEditKey == static_cast<int>(i));

			if (ImGui::Selectable(label, bSelected))
			{
				// ���õ� �� �ε��� �� ������ ����
				m_iEditKey = static_cast<int>(i);
				m_EditPosKey = {};
				m_EditRotKey = {};
				m_EditFovKey = m_CameraDatas.vecFovData[i];
			}
		}
		ImGui::EndChild();


		ImGui::SeparatorText("Edit Fov Key Info");
		ImGui::DragFloat("Fov", &m_EditFovKey.fFov);

		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpFov = static_cast<int>(m_EditFovKey.interpFov);
		if (ImGui::Combo("Fov Interp", &interpFov, interpNames, IM_ARRAYSIZE(interpNames)))
			m_EditFovKey.interpFov = static_cast<INTERPOLATION_CAMERA>(interpFov);

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
