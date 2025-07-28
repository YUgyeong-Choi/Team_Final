#include "YGTool.h"
#include "GameInstance.h"

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


	return S_OK;
}



HRESULT CYGTool::Render_CameraTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Camera Tools", &open, NULL);

	if (ImGui::CollapsingHeader("Camera Control"))
	{
		if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
		{
			CTransform* pTransform = pCam->Get_TransfomCom();

			// ���� ���� ���
			_float4x4 worldMat;
			XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());

			// ��� -> float[16]
			_float matrix[16];
			memcpy(matrix, &worldMat, sizeof(float) * 16);

			// ����
			_float position[3], rotation[3], scale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

			if (ImGui::DragFloat3("Position", position, 0.1f, -FLT_MAX, FLT_MAX, "%.3f"))
			{
				XMVECTOR newPos = XMVectorSet(position[0], position[1], position[2], 1.f);
				pTransform->Set_State(STATE::POSITION, newPos);
			}

			if (ImGui::DragFloat3("Rotation", rotation, 0.1f, -FLT_MAX, FLT_MAX, "%.3f"))
			{
				// ���Ϸ�(degree) �� radian
				XMVECTOR quat = XMQuaternionRotationRollPitchYaw(
					XMConvertToRadians(rotation[0]), // pitch (X)
					XMConvertToRadians(rotation[1]), // yaw (Y)
					XMConvertToRadians(rotation[2])  // roll (Z)
				);

				_matrix rotMatrix = XMMatrixRotationQuaternion(quat);

				pTransform->Set_State(STATE::RIGHT, XMVector3Normalize(rotMatrix.r[0]));
				pTransform->Set_State(STATE::UP, XMVector3Normalize(rotMatrix.r[1]));
				pTransform->Set_State(STATE::LOOK, XMVector3Normalize(rotMatrix.r[2]));
			}

			if (ImGui::Checkbox(u8"Use Lerp?", &m_bUseLerp)) {}
			ImGui::InputFloat(u8"Duration (sec)", &m_fDuration, 0.1f, 1.0f, "%.2f");
			if (m_fDuration < 0.f) m_fDuration = 0.f; // ���� ����

			XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());
			memcpy(matrix, &worldMat, sizeof(float) * 16);
			ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

			// float[3] �� _vector�� ��ȯ
			m_CutSceneDesc.vPosition = XMVectorSet(position[0], position[1], position[2], 1.f);
			m_CutSceneDesc.vRotation = XMVectorSet(rotation[0], rotation[1], rotation[2], 0.f);
			m_CutSceneDesc.bUseLerp = m_bUseLerp;
			m_CutSceneDesc.fDuration = m_fDuration;


			if (ImGui::Button(u8"Add"))
			{
				m_vecCameraFrame.push_back(m_CutSceneDesc);
				m_fDuration = {};
			}
		}
		else
		{
			ImGui::Text("���� Ȱ��ȭ�� ī�޶� �����ϴ�.");
		}
	}


	ImGui::Separator();
	ImGui::Text("CutScene Frames:");

	ImGui::BeginChild("CutSceneFrameList", ImVec2(0, 200), true);

	// ����Ʈ ���
	for (size_t i = 0; i < m_vecCameraFrame.size(); ++i)
	{
		const auto& desc = m_vecCameraFrame[i];

		XMFLOAT3 pos{}, rot{};
		XMStoreFloat3(&pos, desc.vPosition);
		XMStoreFloat3(&rot, desc.vRotation);

		char label[256];
		sprintf_s(label, sizeof(label),
			"Frame %zu: Pos(%.2f, %.2f, %.2f) Rot(%.2f, %.2f, %.2f) Dur: %.2fs",
			i, pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, desc.fDuration);

		// ���� �����ϰ�
		if (ImGui::Selectable(label, selectedFrameIndex == i))
		{
			selectedFrameIndex = static_cast<int>(i);
			m_CutSceneDesc = m_vecCameraFrame[i]; // ���� �� ���� ���������� ����
		}
	}
	ImGui::EndChild();


	if (selectedFrameIndex >= 0 && selectedFrameIndex < (int)m_vecCameraFrame.size())
	{
		// ���õ� ���� �ٲ�� ������ �� �ʱ�ȭ
		if (selectedFrameIndex != lastSelectedIndex)
		{
			lastSelectedIndex = selectedFrameIndex;

			XMFLOAT3 pos, rot;
			XMStoreFloat3(&pos, m_vecCameraFrame[selectedFrameIndex].vPosition);
			XMStoreFloat3(&rot, m_vecCameraFrame[selectedFrameIndex].vRotation);

			editedPos = pos;
			editedRot = rot;
			editedDuration = m_vecCameraFrame[selectedFrameIndex].fDuration;
			editedLerp = m_vecCameraFrame[selectedFrameIndex].bUseLerp;
		}

		ImGui::Separator();
		ImGui::Text("Selected Frame: %d", selectedFrameIndex);

		// Position �Է�
		ImGui::Text("Position");
		ImGui::InputFloat3("##pos", reinterpret_cast<float*>(&editedPos));

		// Rotation �Է�
		ImGui::Text("Rotation");
		ImGui::InputFloat3("##rot", reinterpret_cast<float*>(&editedRot));

		// Duration �Է�
		ImGui::Text("Duration (sec)");
		ImGui::InputFloat("##duration", &editedDuration, 0.1f, 1.0f, "%.2f");

		// UseLerp �Է�
		ImGui::Text("Use Lerp");
		ImGui::Checkbox("##Lerp", &editedLerp);

		// ���� ��ư
		if (ImGui::Button("Apply Changes"))
		{
			// _vector�� �ٽ� ��ȯ�ؼ� ����
			m_vecCameraFrame[selectedFrameIndex].vPosition = XMVectorSet(editedPos.x, editedPos.y, editedPos.z, 1.0f);
			m_vecCameraFrame[selectedFrameIndex].vRotation = XMVectorSet(editedRot.x, editedRot.y, editedRot.z, 0.0f); 

			m_vecCameraFrame[selectedFrameIndex].fDuration = editedDuration;
			m_vecCameraFrame[selectedFrameIndex].bUseLerp = editedLerp;

			m_CutSceneDesc = m_vecCameraFrame[selectedFrameIndex]; // ���� �۾� �� ���� ����ȭ
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			m_vecCameraFrame.erase(m_vecCameraFrame.begin() + selectedFrameIndex);
			selectedFrameIndex = -1;
			lastSelectedIndex = -1;
		}

		ImGui::SameLine();
		if (ImGui::Button("Clone Camera"))
		{
			m_vecCameraFrame[selectedFrameIndex] = m_CutSceneDesc;
			XMFLOAT3 pos, rot;
			XMStoreFloat3(&pos, m_vecCameraFrame[selectedFrameIndex].vPosition);
			XMStoreFloat3(&rot, m_vecCameraFrame[selectedFrameIndex].vRotation);

			editedPos = pos;
			editedRot = rot;
			editedDuration = m_vecCameraFrame[selectedFrameIndex].fDuration;
		}
	}

	if (ImGui::Button("Play CutScene"))
	{
		CCamera_Manager::Get_Instance()->SetCutSceneCam();
		CCamera_CutScene* cutSceneCamera = static_cast<CCamera_CutScene*>(CCamera_Manager::Get_Instance()->GetCurCam());
		cutSceneCamera->Set_CameraFrame(m_vecCameraFrame);
		cutSceneCamera->PlayCutScene();
	}

	ImGui::End();
	return S_OK;
}

HRESULT CYGTool::Render_CameraFrame()
{

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

}
