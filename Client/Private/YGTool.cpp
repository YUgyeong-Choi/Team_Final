#include "YGTool.h"
#include "GameInstance.h"

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

			// 기존 월드 행렬
			_float4x4 worldMat;
			XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());

			// 행렬 -> float[16]
			_float matrix[16];
			memcpy(matrix, &worldMat, sizeof(float) * 16);

			// 분해
			_float position[3], rotation[3], scale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

			if (ImGui::DragFloat3("Position", position, 0.1f, -FLT_MAX, FLT_MAX, "%.3f"))
			{
				XMVECTOR newPos = XMVectorSet(position[0], position[1], position[2], 1.f);
				pTransform->Set_State(STATE::POSITION, newPos);
			}

			if (ImGui::DragFloat3("Rotation", rotation, 0.1f, -FLT_MAX, FLT_MAX, "%.3f"))
			{
				// 오일러(degree) → radian
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
			if (m_fDuration < 0.f) m_fDuration = 0.f; // 음수 방지

			// float[3] → _vector로 변환
			_float worldMatrix[16];
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, worldMatrix);
			XMMATRIX mat = XMLoadFloat4x4((XMFLOAT4X4*)worldMatrix);

			m_CutSceneDesc.worldMatrix = mat;
			m_CutSceneDesc.bUseLerp = m_bUseLerp;
			m_CutSceneDesc.fDuration = m_fDuration;

			if (ImGui::Button(u8"Add"))
			{
				m_vecCameraFrame.push_back(m_CutSceneDesc);
			}
		}
		else
		{
			ImGui::Text("현재 활성화된 카메라가 없습니다.");
		}
	}


	ImGui::Separator();
	ImGui::Text("CutScene Frames:");

	ImGui::BeginChild("CutSceneFrameList", ImVec2(0, 200), true);

	// 리스트 출력
	for (size_t i = 0; i < m_vecCameraFrame.size(); ++i)
	{
		const auto& desc = m_vecCameraFrame[i];

		// 기존 월드 행렬
		XMFLOAT4X4 worldMat;
		XMStoreFloat4x4(&worldMat, desc.worldMatrix);

		// 행렬 -> float[16]
		_float matrix[16];
		memcpy(matrix, &worldMat, sizeof(float) * 16);

		// 분해
		_float position[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

		char label[256];
		sprintf_s(label, sizeof(label),
			"Frame %zu: Pos(%.2f, %.2f, %.2f) Rot(%.2f, %.2f, %.2f) Dur: %.2fs",
			i, position[0], position[1], position[2],
			rotation[0], rotation[1], rotation[2],
			desc.fDuration);

		if (ImGui::Selectable(label, selectedFrameIndex == i))
		{
			selectedFrameIndex = static_cast<int>(i);
			m_CutSceneDesc = m_vecCameraFrame[i]; // 복사
		}

	}
	ImGui::EndChild();


	if (selectedFrameIndex >= 0 && selectedFrameIndex < (int)m_vecCameraFrame.size())
	{
		// 선택된 것이 바뀌면 편집용 값 초기화
		if (selectedFrameIndex != lastSelectedIndex)
		{
			lastSelectedIndex = selectedFrameIndex;

			// 기존 월드 행렬
			XMFLOAT4X4 worldMat;
			XMStoreFloat4x4(&worldMat, m_vecCameraFrame[selectedFrameIndex].worldMatrix);

			// 행렬 -> float[16]
			_float matrix[16];
			memcpy(matrix, &worldMat, sizeof(float) * 16);

			// 분해
			_float position[3], rotation[3], scale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

			editedPos = { position[0], position[1], position[2] };
			editedRot = { rotation[0], rotation[1], rotation[2] };
			editedDuration = m_vecCameraFrame[selectedFrameIndex].fDuration;
			editedLerp = m_vecCameraFrame[selectedFrameIndex].bUseLerp;
		}

		ImGui::Separator();
		ImGui::Text("Selected Frame: %d", selectedFrameIndex);

		// Position 입력
		ImGui::Text("Position");
		ImGui::InputFloat3("##pos", reinterpret_cast<float*>(&editedPos));

		// Rotation 입력
		ImGui::Text("Rotation");
		ImGui::InputFloat3("##rot", reinterpret_cast<float*>(&editedRot));

		// Duration 입력
		ImGui::Text("Duration (sec)");
		ImGui::InputFloat("##duration", &editedDuration, 0.1f, 1.0f, "%.2f");

		// UseLerp 입력
		ImGui::Text("Use Lerp");
		ImGui::Checkbox("##Lerp", &editedLerp);

		// 적용 버튼
		if (ImGui::Button("Apply Changes"))
		{
			// 1. float[3] → 행렬 재조합
			float pos[3] = { editedPos.x, editedPos.y, editedPos.z };
			float rot[3] = { editedRot.x, editedRot.y, editedRot.z };
			float scl[3] = { 1.f, 1.f, 1.f }; // 스케일 고정

			float matrix[16];
			ImGuizmo::RecomposeMatrixFromComponents(pos, rot, scl, matrix);
			XMMATRIX mat = XMLoadFloat4x4((XMFLOAT4X4*)matrix);

			// 2. 프레임 정보 갱신
			m_vecCameraFrame[selectedFrameIndex].worldMatrix = mat;
			m_vecCameraFrame[selectedFrameIndex].bUseLerp = editedLerp;
			m_vecCameraFrame[selectedFrameIndex].fDuration = editedDuration;
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
