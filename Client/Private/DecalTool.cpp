#include "GameInstance.h"

#include "DecalTool.h"
#include "DecalToolObject.h"

#include "Camera_Manager.h"
#include "ImGuiFileDialog.h"


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

HRESULT CDecalTool::Save(const _char* Map)
{
	string DecalFilePath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	filesystem::create_directories("../Bin/Save/DecalTool");
	ofstream DecalDataFile(DecalFilePath);

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

	MSG_BOX("��Į ���� ����!");

	return S_OK;
}

HRESULT CDecalTool::Load(const _char* Map)
{
	//���� �ʿ� ��ġ�� ������Ʈ�� ��� ��������
	Clear_All_Decal();

	string DecalDataPath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //���߿� ���� �� �ٲܶ�

	ifstream inFile(DecalDataPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Decal_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}


	// JSON �Ľ�
	json JSON;
	inFile >> JSON;

	// ������ ��ŭ Decal ��ȯ
	for (auto& item : JSON)
	{
		// 4x4 ��� �б�
		_float4x4 WorldMatrix;
		for (_int iRow = 0; iRow < 4; ++iRow)
		{
			for (_int iCol = 0; iCol < 4; ++iCol)
			{
				WorldMatrix.m[iRow][iCol] = item["WorldMatrix"][iRow][iCol].get<_float>();
			}
		}

		CDecal::DECAL_DESC DecalDesc = {};
		DecalDesc.WorldMatrix = WorldMatrix;

		// Decal ��ü ����
		m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
			ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"), &DecalDesc);
	}

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

	//Ŭ���ϸ� ���� ����� ��Į�� ��Ŀ�� �Ѵ�.
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		//��ƮŰ ������ ������ ��ŷ���� ����(������Ʈ ���̰��� ������Ʈ ����Ǵ°� ���� ������)
		if (m_pGameInstance->Key_Pressing(DIK_LALT))
			return;

		// ImGui�� ���콺 �Է��� �������� ��ŷ�� ���� ����
		if (ImGui::GetIO().WantCaptureMouse)
			return;

		_float4 vWorldPos = {};
		if (m_pGameInstance->Picking(&vWorldPos))
		{
			Safe_Release(m_pFocusObject);
			m_pFocusObject = Get_ClosestDecalObject(XMLoadFloat4(&vWorldPos));
			Safe_AddRef(m_pFocusObject);
		}
		
	}

	//F Ű������ �ش� ������Ʈ ��ġ�� �̵�
	if (m_pGameInstance->Key_Down(DIK_F))
	{
		if (nullptr == m_pFocusObject)
			return;

		_vector vObjectPos = m_pFocusObject->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vCameraPos = XMVectorAdd(vObjectPos, XMVectorSet(0.f, 3.f, -3.f, 0.f));

		CTransform* pCameraTransformCom = CCamera_Manager::Get_Instance()->GetFreeCam()->Get_TransfomCom();

		//������ �ΰ� �̵�����
		pCameraTransformCom->Set_State(STATE::POSITION, vCameraPos);

		//LookAt ����
		pCameraTransformCom->LookAt(vObjectPos);
	}

}

void CDecalTool::Clear_All_Decal()
{
	Safe_Release(m_pFocusObject);
	m_pFocusObject = nullptr;

	list<CGameObject*> List = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"));

	for (CGameObject* pObj : List)
	{
		pObj->Set_bDead();
	}
}

CDecalToolObject* CDecalTool::Get_ClosestDecalObject(_fvector vPosition)
{
	list<CGameObject*> List = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"));

	_float fMinDistance = FLT_MAX;
	CGameObject* pClosestObject = nullptr;

	for (CGameObject* pObj : List)
	{
		_float fDist = XMVectorGetX(XMVector3LengthSq(vPosition - pObj->Get_TransfomCom()->Get_State(STATE::POSITION)));

		if (fDist < fMinDistance)
		{
			fMinDistance = fDist;
			pClosestObject = pObj;
		}

	}

	return  static_cast<CDecalToolObject*>(pClosestObject);
}

HRESULT CDecalTool::Spawn_DecalObject()
{
	CDecal::DECAL_DESC DecalDesc = {};

	//��ȯ�ϰ� ��Ŀ�� ����
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
		ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"), &DecalDesc)))
		return E_FAIL;

	Safe_Release(m_pFocusObject);
	m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")));
	Safe_AddRef(m_pFocusObject);

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

	ImGui::Separator();

	Detail_Texture();


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


void CDecalTool::Detail_Texture()
{
	//���⼭ ��Į�� ARM, N, BC ������ �ؽ��ĸ� ���� �� �� �ְ� �ϰ�ʹ�.
	//Imgui �ַ�� Ž���⸦ ��� dds�ؽ��ĸ� ã�Ƽ� �ȾƳ־����

	/*CDecalToolObject* pSelectedDecal = Get_Selected_Decal();
	if (!pSelectedDecal)
		return;*/



	IGFD::FileDialogConfig Config;
	Config.path = "../Bin/Resources/Textures/Decal/";

#pragma region ARM �ؽ�ó
	ImGui::TextWrapped("Current ARM Texture: %s", "armPathStr.c_str()");
	// ARM �ؽ�ó ���� ��ư
	if (ImGui::Button("Change ARM Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseARMTexture", "Select ARM Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseARMTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
			if (m_pFocusObject)
			{
				if(FAILED(m_pFocusObject->Set_ARM_Texture(filePath, fileName)))
					MSG_BOX("ARM �ؽ��� ���Ƴ��� ����");
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion
	ImGui::Separator();
#pragma region Normal �ؽ���
	ImGui::TextWrapped("Current Normal Texture: %s", "normalPathStr.c_str()");
	// Normal �ؽ�ó ���� ��ư
	if (ImGui::Button("Change Normal Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseNormalTexture", "Select Normal Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseNormalTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//pSelectedDecal->Set_Normal_Texture(StringToWString(filePath));
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion
	ImGui::Separator();
#pragma region BaseColor �ؽ���
	ImGui::TextWrapped("Current BaseColor Texture: %s", "baseColorPathStr.c_str()");
	// BaseColor �ؽ�ó ���� ��ư
	if (ImGui::Button("Change BaseColor Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseBaseColorTexture", "Select BaseColor Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseBaseColorTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//pSelectedDecal->Set_BaseColor_Texture(StringToWString(filePath));
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion


	
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
