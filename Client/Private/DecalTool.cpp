#include "GameInstance.h"

#include "DecalTool.h"
#include "DecalToolObject.h"

CDecalTool::CDecalTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CYWTool(pDevice, pContext)
{
}

CDecalTool::CDecalTool(const CDecalTool& Prototype)
	:CYWTool(Prototype)
{
}

HRESULT CDecalTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecalTool::Initialize(void* pArg)
{
	/*m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_TestDecal")));
	Safe_AddRef(m_pFocusObject);*/


	return S_OK;
}

void CDecalTool::Priority_Update(_float fTimeDelta)
{
}

void CDecalTool::Update(_float fTimeDelta)
{
	Control(fTimeDelta);
}

void CDecalTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CDecalTool::Render()
{
	return S_OK;
}

HRESULT	CDecalTool::Render_ImGui()
{
	ImGui::Begin("Decal Tool", nullptr);

	if(ImGui::Button("Spawn Decal"))
	{
		Spawn_DecalObject();
	}

	ImGui::End();

	Render_Detail();

	return S_OK;
}

void CDecalTool::Control(_float fTimeDelta)
{
	if (GetForegroundWindow() != g_hWnd)
		return;

	if (ImGuizmo::IsUsing() == false)
	{
		//E ȸ��, R ũ��, T�� ��ġ
		if (m_pGameInstance->Key_Down(DIK_E))
			m_currentOperation = ImGuizmo::ROTATE;
		else if (m_pGameInstance->Key_Down(DIK_R))
			m_currentOperation = ImGuizmo::SCALE;
		else if (m_pGameInstance->Key_Down(DIK_T))
			m_currentOperation = ImGuizmo::TRANSLATE;

		//if (m_pGameInstance->Mouse_Up(DIM::WHEELBUTTON))
		//{
		//	//��� ������Ʈ ���� ����
		//	m_pFocusObject = nullptr;

		//}
	}

	//Ctrl + S �� ����
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_S))
	{
		Save();
	}

}

HRESULT CDecalTool::Spawn_DecalObject()
{
	//��ȯ�ϰ� ��Ŀ�� ����
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
		ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"))))
		return E_FAIL;

	Safe_Release(m_pFocusObject);
	m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")));
	Safe_AddRef(m_pFocusObject);

	return S_OK;
}

HRESULT CDecalTool::Save()
{
	filesystem::create_directories("../Bin/Save/DecalTool");
	ofstream DecalDataFile("../Bin/Save/DecalTool/DecalData.json");

	if (!DecalDataFile.is_open())
		return E_FAIL;

	// ��ü ��Į �����͸� ���� JSON
	json DecalList = json::array();

	for (CGameObject* pObj : m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")))
	{
		// ���� ��� ���
		_matrix matWorld = pObj->Get_TransfomCom()->Get_WorldMatrix();
		_float4x4 matWorldFloat4x4;
		XMStoreFloat4x4(&matWorldFloat4x4, matWorld);

		// 4x4 ����� �迭�� ����
		json MatrixJson = json::array();
		for (_int i = 0; i < 4; ++i)
		{
			json Row = json::array();
			for (_int j = 0; j < 4; ++j)
			{
				Row.push_back(matWorldFloat4x4.m[i][j]);
			}
			MatrixJson.push_back(Row);
		}

		// ��Į ��ü ���� �߰�
		json DecalJson;
		DecalJson["WorldMatrix"] = MatrixJson;

		// �ʿ��� ���, ������Ʈ �̸��̳� ID�� ���� ����
		// DecalJson["Name"] = pObj->Get_Name(); �Ǵ� ID ��

		// ����Ʈ�� �߰�
		DecalList.push_back(DecalJson);
	}

	// JSON ���Ͽ� ���
	DecalDataFile << DecalList.dump(4); // �鿩���� 4ĭ���� ���� ���� ���
	DecalDataFile.close();

	return S_OK;
}


void CDecalTool::Render_Detail()
{
#pragma region ������
	ImGui::Begin("Detail", nullptr);

	ImGui::Separator();

	//Detail_Name();

	ImGui::Separator();

	Detail_Transform();

	ImGui::End();
#pragma endregion
}

void CDecalTool::Detail_Transform()
{
	ImGui::Text("Transform");

	if (m_pFocusObject != nullptr)
	{
		CTransform* pTransform = m_pFocusObject->Get_TransfomCom();

		// ���� ��ư ������ ����
		_float fButtonWidth = ImGui::CalcTextSize("Reset").x + ImGui::GetStyle().FramePadding.x * 2;
		_float fRegionWidth = ImGui::GetContentRegionAvail().x;
		ImGui::SameLine(ImGui::GetCursorPosX() + fRegionWidth - fButtonWidth);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

		if (ImGui::Button("Reset"))
		{
			_float4x4 MatrixIdentity = {};
			XMStoreFloat4x4(&MatrixIdentity, XMMatrixIdentity());
			pTransform->Set_WorldMatrix(MatrixIdentity);
		}

		ImGui::PopStyleColor(3);

#pragma region ����� �� ��� ����
		_float4x4 worldMat;
		XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());

		_float matrix[16];
		memcpy(matrix, &worldMat, sizeof(float) * 16);

		// ����
		_float position[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

		// ImGuizmo ����
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImGuizmo::SetRect(0, 0, displaySize.x, displaySize.y);

		_float viewMat[16], projMat[16];
		XMStoreFloat4x4(reinterpret_cast<_float4x4*>(viewMat), m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
		XMStoreFloat4x4(reinterpret_cast<_float4x4*>(projMat), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

		// Gizmo ����
		ImGuizmo::Manipulate(viewMat, projMat, m_currentOperation, ImGuizmo::LOCAL, matrix);
#pragma endregion

#pragma region ������
		_bool bPositionChanged = ImGui::InputFloat3("##Position", position, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Position", m_currentOperation == ImGuizmo::TRANSLATE))
			m_currentOperation = ImGuizmo::TRANSLATE;
#pragma endregion

#pragma region ȸ��
		_bool bRotationChanged = ImGui::InputFloat3("##Rotation", rotation, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotation", m_currentOperation == ImGuizmo::ROTATE))
			m_currentOperation = ImGuizmo::ROTATE;
#pragma endregion

#pragma region ������
		_bool bScaleChanged = ImGui::InputFloat3("##Scale", scale, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", m_currentOperation == ImGuizmo::SCALE))
			m_currentOperation = ImGuizmo::SCALE;
#pragma endregion

#pragma region ����

		if (ImGuizmo::IsUsing())
		{
			// ImGuizmo�� ���۵� matrix �״�� ����
			memcpy(&worldMat, matrix, sizeof(_float) * 16);

			// ��ǥ ������Ʈ�� ���� ��İ� �� ��� ��
			_matrix matPrevMain = m_pFocusObject->Get_TransfomCom()->Get_WorldMatrix();
			_matrix matNewMain = XMLoadFloat4x4(&worldMat);

			// ��ǥ ������Ʈ�� �̵�/ȸ��/������ ��ȭ ��� ���
			_matrix matOffset = matNewMain * XMMatrixInverse(nullptr, matPrevMain);

			pTransform->Set_WorldMatrix(worldMat);

		}
		else if (bPositionChanged || bRotationChanged || bScaleChanged)
		{
			// ���� �Է����� �ٲ� �� �� matrix �籸�� �� ����
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
		}

#pragma endregion
	}
}

CDecalTool* CDecalTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CDecalTool* pInstance = new CDecalTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CDecalTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDecalTool::Clone(void* pArg)
{
	CDecalTool* pInstance = new CDecalTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDecalTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CDecalTool::Free()
{
	__super::Free();

	Safe_Release(m_pFocusObject);
}
