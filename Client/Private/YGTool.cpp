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
	m_CameraSequence->m_iFrameMax = 300;
	m_CameraSequence->Add(0,10);
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
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Sequence Tool", &open, NULL);
	if (ImGui::CollapsingHeader("Sequence Info"))
	{
		ImGui::InputInt("End Frame", &m_iEndFrame, 10, 0);
		m_CameraSequence->Set_EndFrame(m_iEndFrame);
	}

	if (ImGui::Button("Get CurrentKeyFrame"))
	{
		m_pSelectedKey = m_CameraSequence->GetKeyAtFrame(m_iCurrentFrame);
		m_pSelectedKey->keyFrame = m_iCurrentFrame;
	}

	if (m_pSelectedKey)
	{
		ImGui::SeparatorText("Current Key Info");

		// 포지션
		XMFLOAT3 pos = m_pSelectedKey->position;
		if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
			m_pSelectedKey->position = pos;

		// 회전
		XMFLOAT3 rot = m_pSelectedKey->rotation;
		if (ImGui::DragFloat3("Rotation (Euler)", reinterpret_cast<float*>(&rot), 0.5f))
			m_pSelectedKey->rotation = rot;

		// FOV
		ImGui::DragFloat("FOV", &m_pSelectedKey->fFov, 0.1f, 1.0f, 179.0f);

		// 보간 방식
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
			CAMERA_KEYFRAME keyFrame;
			keyFrame.keyFrame = m_pSelectedKey->keyFrame;
			keyFrame.position = m_pSelectedKey->position;
			keyFrame.rotation = m_pSelectedKey->rotation;
			keyFrame.fFov = m_pSelectedKey->fFov;

			keyFrame.interpPosition = m_pSelectedKey->interpPosition;
			keyFrame.interpRotation = m_pSelectedKey->interpRotation;
			keyFrame.interpFov = m_pSelectedKey->interpFov;
			m_vecCameraKeyFrame.push_back(keyFrame);
			m_CameraSequence->Add_KeyFrame(keyFrame.keyFrame);
		}
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

			// 1. 위치
			XMFLOAT3 vPos;
			XMStoreFloat3(&vPos, pTransform->Get_State(STATE::POSITION));
			ImGui::DragFloat3("Camera Position", (float*)&vPos, 0.1f);

			// 2. 회전 (쿼터니언 → Euler)
			_matrix worldMat = pTransform->Get_WorldMatrix();
			XMVECTOR qRot = XMQuaternionRotationMatrix(worldMat);

			XMFLOAT3 euler = QuaternionToEuler(qRot);
			XMFLOAT3 eulerDeg;
			eulerDeg.x = XMConvertToDegrees(euler.x);
			eulerDeg.y = XMConvertToDegrees(euler.y);
			eulerDeg.z = XMConvertToDegrees(euler.z);
			ImGui::DragFloat3("Camera Rotation (Euler)", (float*)&eulerDeg, 0.1f);

			// 2. 카메라 Transform에도 적용
			// A. 포지션
			pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.x, vPos.y, vPos.z, 1.0f));

			// B. 회전 (Euler Deg → Rad → Quaternion → 방향 벡터)
			XMFLOAT3 eulerRad;
			eulerRad.x = XMConvertToRadians(eulerDeg.x);
			eulerRad.y = XMConvertToRadians(eulerDeg.y);
			eulerRad.z = XMConvertToRadians(eulerDeg.z);

			qRot = XMQuaternionRotationRollPitchYaw(eulerRad.x, eulerRad.y, eulerRad.z);
			XMMATRIX rotMat = XMMatrixRotationQuaternion(qRot);

			// C. 회전 행렬의 각 축을 Transform에 적용
			pTransform->Set_State(STATE::RIGHT, rotMat.r[0]);
			pTransform->Set_State(STATE::UP, rotMat.r[1]);
			pTransform->Set_State(STATE::LOOK, rotMat.r[2]);

			if (ImGui::Button(u8"카메라 위치 회전 적용"))
			{
				m_pSelectedKey->position = vPos;
				m_pSelectedKey->rotation = eulerDeg;
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

	ImGui::BeginChild("CutSceneFrameList", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

	// 리스트 출력
	for (size_t i = 0; i < m_vecCameraKeyFrame.size(); ++i)
	{
		const auto& desc = m_vecCameraKeyFrame[i];
		char label[32];
		sprintf_s(label, "KeyFrame: %d", desc.keyFrame);

		bool bSelected = (m_iEditKey == static_cast<int>(i));

		if (ImGui::Selectable(label, bSelected))
		{
			// 선택됨 → 인덱스 및 포인터 저장
			m_iEditKey = static_cast<int>(i);
			m_pEditKey = &m_vecCameraKeyFrame[i];
		}
	}
	ImGui::EndChild();


	if (m_pEditKey)
	{
		ImGui::SeparatorText("Edit Key Info");

		// 포지션
		XMFLOAT3 pos = m_pEditKey->position;
		if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
			m_pEditKey->position = pos;

		// 회전
		XMFLOAT3 rot = m_pEditKey->rotation;
		if (ImGui::DragFloat3("Rotation (Euler)", reinterpret_cast<float*>(&rot), 0.5f))
			m_pEditKey->rotation = rot;

		// FOV
		ImGui::DragFloat("FOV", &m_pEditKey->fFov, 0.1f, 1.0f, 179.0f);

		// 보간 방식
		const char* interpNames[] = { "NONE", "LERP", "CATMULL_ROM" };
		int interpPos = static_cast<int>(m_pEditKey->interpPosition);
		int interpRot = static_cast<int>(m_pEditKey->interpRotation);
		int interpFov = static_cast<int>(m_pEditKey->interpFov);

		if (ImGui::Combo("Interp Position", &interpPos, interpNames, IM_ARRAYSIZE(interpNames)))
			m_pEditKey->interpPosition = static_cast<INTERPOLATION_CAMERA>(interpPos);

		if (ImGui::Combo("Interp Rotation", &interpRot, interpNames, IM_ARRAYSIZE(interpNames)))
			m_pEditKey->interpRotation = static_cast<INTERPOLATION_CAMERA>(interpRot);

		if (ImGui::Combo("Interp FOV", &interpFov, interpNames, IM_ARRAYSIZE(interpNames)))
			m_pEditKey->interpFov = static_cast<INTERPOLATION_CAMERA>(interpFov);

		if (ImGui::Button("Delete"))
		{
			if (m_iEditKey >= 0 && m_iEditKey < static_cast<int>(m_vecCameraKeyFrame.size()))
			{
				m_vecCameraKeyFrame.erase(m_vecCameraKeyFrame.begin() + m_iEditKey);

				// 선택 초기화
				m_CameraSequence->Delete_KeyFrame(m_pEditKey->keyFrame);
				m_iEditKey = -1;
				m_pEditKey = nullptr;
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
