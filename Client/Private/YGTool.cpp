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


HRESULT CYGTool::Render_CameraTool()
{
	/*
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

			if (ImGui::RadioButton("None", m_eInterpMode == INTERPOLATION_CAMERA::NONE))
				m_eInterpMode = INTERPOLATION_CAMERA::NONE;

			if (ImGui::RadioButton("Lerp", m_eInterpMode == INTERPOLATION_CAMERA::LERP))
				m_eInterpMode = INTERPOLATION_CAMERA::LERP;

			if (ImGui::RadioButton("CatmullRom", m_eInterpMode == INTERPOLATION_CAMERA::CATMULLROM))
				m_eInterpMode = INTERPOLATION_CAMERA::CATMULLROM;

			ImGui::InputFloat(u8"Duration (sec)", &m_fInterpDuration, 0.1f, 1.0f, "%.2f");
			if (m_fInterpDuration < 0.f) m_fInterpDuration = 0.f; // 음수 방지


			if (ImGui::Checkbox("Zoom Effect", &m_bZoom)) {}

			if (m_bZoom)
			{
				ImGui::DragFloat("FOV", &m_fFov, 0.1f, 1.f, 179.f, "%.1f");
				ImGui::DragFloat("Zoom Duration", &m_fFovDuration, 0.01f, 0.f, 10.f, "%.2f");
			}

			// float[3] → _vector로 변환
			_float worldMatrix[16];
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, worldMatrix);
			XMMATRIX mat = XMLoadFloat4x4((XMFLOAT4X4*)worldMatrix);

			m_CutSceneDesc.worldMatrix = mat;
			m_CutSceneDesc.eInterp = m_eInterpMode;
			m_CutSceneDesc.fInterpDuration = m_fInterpDuration;
			m_CutSceneDesc.bZoom = m_bZoom;
			m_CutSceneDesc.fFov = m_fFov;
			m_CutSceneDesc.fFovDuration = m_fFovDuration;

			if (ImGui::Button(u8"Add"))
			{
				m_vecCameraFrame.push_back(m_CutSceneDesc);
				m_fFov = 60.f;
			}
		}
		else
		{
			ImGui::Text("현재 활성화된 카메라가 없습니다.");
		}
	}

	ImGui::End();
	*/

	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Sequence Tool", &open, NULL);

	ImGui::InputInt("End Frame", &m_iEndFrame, 10, 0);
	m_CameraSequence->Set_EndFrame(m_iEndFrame);

	ImGui::End();
	return S_OK;
}

HRESULT CYGTool::Render_CameraFrame()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Camera Frame", &open, NULL);
	ImGui::Text("CutScene Frames:");

	ImGui::BeginChild("CutSceneFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

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
			desc.fInterpDuration);

		if (ImGui::Selectable(label, m_iSelectedFrameIndex == i))
		{
			m_iSelectedFrameIndex = static_cast<int>(i);
			m_CutSceneDesc = m_vecCameraFrame[i]; // 복사
		}

	}
	ImGui::EndChild();


	if (m_iSelectedFrameIndex >= 0 && m_iSelectedFrameIndex < (int)m_vecCameraFrame.size())
	{
		// 선택된 것이 바뀌면 편집용 값 초기화
		if (m_iSelectedFrameIndex != m_iLastSelectedIndex)
		{
			m_iLastSelectedIndex = m_iSelectedFrameIndex;

			// 기존 월드 행렬
			XMFLOAT4X4 worldMat;
			XMStoreFloat4x4(&worldMat, m_vecCameraFrame[m_iSelectedFrameIndex].worldMatrix);

			// 행렬 -> float[16]
			_float matrix[16];
			memcpy(matrix, &worldMat, sizeof(float) * 16);

			// 분해
			_float position[3], rotation[3], scale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

			m_editedPos = { position[0], position[1], position[2] };
			m_editedRot = { rotation[0], rotation[1], rotation[2] };
			m_fEditedInterpDuration = m_vecCameraFrame[m_iSelectedFrameIndex].fInterpDuration;
			m_eEditInterpMode = m_vecCameraFrame[m_iSelectedFrameIndex].eInterp;
			m_bEditZoom = m_vecCameraFrame[m_iSelectedFrameIndex].bZoom;;
			m_fEditFov = m_vecCameraFrame[m_iSelectedFrameIndex].fFov;
			m_fFovDuration = m_vecCameraFrame[m_iSelectedFrameIndex].fFovDuration;
		}

		ImGui::Separator();
		ImGui::Text("Selected Frame: %d", m_iSelectedFrameIndex);

		// Position 입력
		ImGui::Text("Position");
		ImGui::InputFloat3("##pos", reinterpret_cast<float*>(&m_editedPos));

		// Rotation 입력
		ImGui::Text("Rotation");
		ImGui::InputFloat3("##rot", reinterpret_cast<float*>(&m_editedRot));

		// Duration 입력
		ImGui::Text("Duration (sec)");
		ImGui::InputFloat("##duration", &m_fEditedInterpDuration, 0.1f, 1.0f, "%.2f");

		// UseLerp 입력
		ImGui::Text("Use Lerp");
		if (ImGui::RadioButton("None", m_eEditInterpMode == INTERPOLATION_CAMERA::NONE))
			m_eEditInterpMode = INTERPOLATION_CAMERA::NONE;

		if (ImGui::RadioButton("Lerp", m_eEditInterpMode == INTERPOLATION_CAMERA::LERP))
			m_eEditInterpMode = INTERPOLATION_CAMERA::LERP;

		if (ImGui::RadioButton("CatmullRom", m_eEditInterpMode == INTERPOLATION_CAMERA::CATMULLROM))
			m_eEditInterpMode = INTERPOLATION_CAMERA::CATMULLROM;


		if (ImGui::Checkbox("Zoom Effect", &m_bEditZoom)) {}

		if (m_bEditZoom)
		{
			ImGui::DragFloat("FOV", &m_fEditFov, 0.1f, 1.f, 179.f, "%.1f");
			ImGui::DragFloat("Zoom Duration", &m_fEditFovDuration, 0.01f, 0.f, 10.f, "%.2f");
		}


		// 적용 버튼
		if (ImGui::Button("Apply Changes"))
		{
			// 1. float[3] → 행렬 재조합
			float pos[3] = { m_editedPos.x, m_editedPos.y, m_editedPos.z };
			float rot[3] = { m_editedRot.x, m_editedRot.y, m_editedRot.z };
			float scl[3] = { 1.f, 1.f, 1.f }; // 스케일 고정

			float matrix[16];
			ImGuizmo::RecomposeMatrixFromComponents(pos, rot, scl, matrix);
			XMMATRIX mat = XMLoadFloat4x4((XMFLOAT4X4*)matrix);

			// 2. 프레임 정보 갱신
			m_vecCameraFrame[m_iSelectedFrameIndex].worldMatrix = mat;
			m_vecCameraFrame[m_iSelectedFrameIndex].eInterp = m_eEditInterpMode;
			m_vecCameraFrame[m_iSelectedFrameIndex].fInterpDuration = m_fEditedInterpDuration;
			m_vecCameraFrame[m_iSelectedFrameIndex].bZoom = m_bEditZoom;
			m_vecCameraFrame[m_iSelectedFrameIndex].fFov = m_fEditFov;
			m_vecCameraFrame[m_iSelectedFrameIndex].fFovDuration = m_fEditFovDuration;
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			m_vecCameraFrame.erase(m_vecCameraFrame.begin() + m_iSelectedFrameIndex);
			m_iSelectedFrameIndex = -1;
			m_iLastSelectedIndex = -1;
		}

		ImGui::SameLine();
		if (ImGui::Button("Clone Camera"))
		{
			m_vecCameraFrame[m_iSelectedFrameIndex] = m_CutSceneDesc;
		}
	}

	if (ImGui::Button("Play CutScene"))
	{
		CCamera_Manager::Get_Instance()->SetCutSceneCam();
		CCamera_CutScene* cutSceneCamera = static_cast<CCamera_CutScene*>(CCamera_Manager::Get_Instance()->GetCurCam());
		cutSceneCamera->Set_CameraFrame(m_vecCameraFrame);
		cutSceneCamera->PlayCutScene();
		cutSceneCamera->Set_CurrentFrame(m_iSelectedFrameIndex);
	}

	if(CCamera_CutScene* _scene = dynamic_cast<CCamera_CutScene*>(CCamera_Manager::Get_Instance()->GetCurCam()))
	{
		ImGui::Text("Play Frame: %d", _scene->Get_CurrentFrame());
	}
	

	ImGui::End();
	return S_OK;
}


HRESULT CYGTool::Render_CameraSequence()
{
	SetNextWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Camera Sequencer");

	static int currentFrame = 0;
	static bool expanded = true;
	static int selected = -1;
	static int firstFrame = 0;

	ImSequencer::Sequencer(
		m_CameraSequence,
		&currentFrame,
		&expanded,
		&selected,
		&firstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL);

	//m_CameraSequence->Render_EditorUI();

	ImGui::End();

	return S_OK;
}

//void Render_EditorUI()
//{
//	CAMERA_KEY& key = m_vecKeys[m_iSelectedIndex];
//
//	ImGui::Separator();
//	ImGui::Text("Editing Camera Key %d", m_iSelectedIndex);
//
//	// 시작/끝 프레임
//	ImGui::InputInt("Start Frame", &key.startFrame);
//	ImGui::InputInt("End Frame", &key.endFrame);
//
//	// 타입 선택
//	const char* typeNames[] = { "Position", "Rotation", "FOV" };
//	int type = key.type;
//	if (ImGui::Combo("Type", &type, typeNames, IM_ARRAYSIZE(typeNames)))
//	{
//		key.type = type;
//		// 색상 자동 설정
//		switch (type)
//		{
//		case 0: key.color = IM_COL32(255, 200, 0, 255); break;
//		case 1: key.color = IM_COL32(100, 255, 255, 255); break;
//		case 2: key.color = IM_COL32(200, 100, 255, 255); break;
//		}
//	}
//
//	// 색상도 편집 가능하게 (선택)
//	ImGui::ColorEdit4("Color", (float*)&key.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
//
//	// 개발 편의를 위한 디버그 출력
//	ImGui::Text("Duration: %d frames", key.endFrame - key.startFrame);
//}

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
