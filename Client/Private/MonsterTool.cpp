#include "GameInstance.h"

#include "MonsterTool.h"
#include "MonsterToolObject.h"

#include "Camera_Manager.h"
#include "ImGuiFileDialog.h"


CMonsterTool::CMonsterTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CYWTool(pDevice, pContext)
{
}

CMonsterTool::CMonsterTool(const CMonsterTool& Prototype)
	:CYWTool(Prototype)
{
}

HRESULT CMonsterTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonsterTool::Initialize(void* pArg)
{
	/*m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_TestDecal")));
	Safe_AddRef(m_pFocusObject);*/


	return S_OK;
}

void CMonsterTool::Priority_Update(_float fTimeDelta)
{
}

void CMonsterTool::Update(_float fTimeDelta)
{
	Control(fTimeDelta);
}

void CMonsterTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CMonsterTool::Render()
{
	return S_OK;
}

HRESULT	CMonsterTool::Render_ImGui()
{
	ImGui::Begin("Monster Tool", nullptr);

	if (ImGui::Button("Spawn Monster"))
	{
		Spawn_MonsterToolObject();
	}

	//���� ���� ���� �޺� �ڽ�
	if (ImGui::BeginCombo("##MonsterCombo", m_Monsters[m_iMonsterIndex].c_str()))
	{
		for (_int i = 0; i < IM_ARRAYSIZE(m_Monsters); i++)
		{
			_bool bSelected = (m_iMonsterIndex == i);
			if (ImGui::Selectable(m_Monsters[i].c_str(), bSelected))
			{
				m_iMonsterIndex = i;
			}

			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::End();

	Render_Detail();

	return S_OK;
}

HRESULT CMonsterTool::Save(const _char* Map)
{

	return S_OK;
}

HRESULT CMonsterTool::Load(const _char* Map)
{

	return S_OK;
}

void CMonsterTool::Control(_float fTimeDelta)
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

		//��Ŀ�� ����
		if (m_pGameInstance->Mouse_Up(DIM::WHEELBUTTON))
		{
			Safe_Release(m_pFocusObject);
			m_pFocusObject = nullptr;
		}
	}

	//Ŭ���ؼ� ����
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		Picking();
	}

	//F Ű������ �ش� ������Ʈ ��ġ�� �̵�
	if (m_pGameInstance->Key_Down(DIK_F))
	{
		Focus();
	}

	//����ƮŰ ������ ���� ���õȰ� ����
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		Delete_FocusObject();
	}

	//��Ʈ Ŭ���ϸ� �ش� ��ġ�� ��Į �̵�
	if (m_pGameInstance->Key_Pressing(DIK_LALT) && m_pGameInstance->Mouse_Up(DIM::LBUTTON))
	{
		SnapTo();
	}

	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_D))
	{
		Duplicate();
	}
}
void CMonsterTool::Picking()
{
	_int iID = { 0 };
	if (m_pGameInstance->PickByClick(&iID))
	{
		cout << "Monster ID(����): " << iID << endl;
	}

	//MonsterToolObject �߿� ���� ���̵� ã�Ƽ� ��Ŀ���Ѵ�.

	list<CGameObject*>& ObjList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_MonsterToolObject"));

	for (CGameObject*pObj : ObjList)
	{
		CMonsterToolObject* pMonsterToolObj = static_cast<CMonsterToolObject*>(pObj);
		if (pMonsterToolObj->m_iID == iID)
		{
			Safe_Release(m_pFocusObject);
			m_pFocusObject = pMonsterToolObj;
			Safe_AddRef(m_pFocusObject);

			break;
		}
	}
	
}

void CMonsterTool::Focus()
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

void CMonsterTool::SnapTo()
{
	if (m_pFocusObject)
	{
		CTransform* pTransform = m_pFocusObject->Get_TransfomCom();

		_float4 vPickedPos = {};
		if (m_pGameInstance->Picking(&vPickedPos))
		{
			pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPickedPos));

		}
	}
}

void CMonsterTool::Duplicate()
{
	//if (m_pFocusObject == nullptr)
	//	return;

	//CDecalToolObject::DECALTOOLOBJECT_DESC Desc = {};
	//Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = m_pFocusObject->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)];
	//Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = m_pFocusObject->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)];
	//Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = m_pFocusObject->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)];

	//Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)];
	//Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)];
	//Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)];

	//Desc.WorldMatrix = m_pFocusObject->Get_TransfomCom()->Get_World4x4();

	////��ȯ�ϰ� ��Ŀ�� ����
	//m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
	//	ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"), &Desc);

	//Safe_Release(m_pFocusObject);
	//m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")));
	//Safe_AddRef(m_pFocusObject);
}

void CMonsterTool::Clear_All_Decal()
{
	Safe_Release(m_pFocusObject);
	m_pFocusObject = nullptr;

	list<CGameObject*> List = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"));

	for (CGameObject* pObj : List)
	{
		pObj->Set_bDead();
	}
}


HRESULT CMonsterTool::Ready_Texture(const _char* Map)
{
	string ResourcePath = string("../Bin/Save/DecalTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);

		return S_OK;
	}

	json ReadyTextureJson;
	try
	{
		inFile >> ReadyTextureJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON �Ľ� ����", MB_OK);
		return E_FAIL;
	}

	//�ؽ��� ������Ÿ�� ����
	for (auto& [PrototypeTag, Path] : ReadyTextureJson.items())
	{
		//�̹� ������ �ѱ��
		if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), StringToWString(PrototypeTag)) != nullptr)
			continue;

		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), StringToWString(PrototypeTag),
			CTexture::Create(m_pDevice, m_pContext, StringToWString(Path).c_str()))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMonsterTool::Spawn_MonsterToolObject()
{
#pragma region ī�޶� �տ��ٰ� ��ȯ
	//ī�޶� �տ��ٰ� ��ȯ
	//ī�޶� ��ġ����, ����� Look ��ŭ ������
	// ī�޶� ��ġ
	_float4 CamPos = *m_pGameInstance->Get_CamPosition();

	// ��ġ�� �ݿ��� ��� ����
	_matrix matWorld = XMMatrixTranslation(CamPos.x, CamPos.y, CamPos.z);

	// ī�޶� ���� ��� (�� ��� �����)
	_matrix CamWorldMatrix = XMMatrixInverse(nullptr, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	_float4x4 CamWM = {};
	XMStoreFloat4x4(&CamWM, CamWorldMatrix);

	// �� ���� ���� (3��° ��)
	_vector vLook = XMVectorSet(CamWM._31, CamWM._32, CamWM._33, 0.f);

	// �� ���� ����ȭ
	vLook = XMVector3Normalize(vLook);

	// �Ÿ� ����
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 1000.f;

	// �� ���Ϳ� �Ÿ� ���ϱ�
	_vector vOffset = XMVectorScale(vLook, fDist);

	// ī�޶� ��ġ ����
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// ���� ��ġ ��� (ī�޶� ��ġ + �� * �Ÿ�)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// ���� ���� ��� ���� (��ġ��)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

#pragma endregion

	CMonsterToolObject::MONSTERTOOLOBJECT_DESC Desc{};
	// ������Ʈ ���� ��Ŀ� ����
	XMStoreFloat4x4(&Desc.WorldMatrix, SpawnWorldMatrix);

	Desc.fSpeedPerSec = 5.f;
	Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	Desc.eMeshLevelID = LEVEL::YW;
	Desc.InitPos = _float3(85.5f, 0.f, -7.5f);
	Desc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(Desc.szMeshID, StringToWString(m_Monsters[m_iMonsterIndex]).c_str());



	Desc.iID = m_iID--;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MonsterToolObject"),
		ENUM_CLASS(LEVEL::YW), TEXT("Layer_MonsterToolObject"), &Desc)))
		return E_FAIL;


	Safe_Release(m_pFocusObject);
	m_pFocusObject = static_cast<CMonsterToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_MonsterToolObject")));
	Safe_AddRef(m_pFocusObject);

	return S_OK;
}

void CMonsterTool::Delete_FocusObject()
{
	Safe_Release(m_pFocusObject);
	m_pFocusObject->Set_bDead();
	m_pFocusObject = nullptr;
}


void CMonsterTool::Render_Detail()
{
#pragma region ������
	ImGui::Begin("Detail", nullptr);

	Detail_Transform();

	ImGui::Separator();

	Detail_Texture();


	ImGui::End();
#pragma endregion
}

void CMonsterTool::Detail_Transform()
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


void CMonsterTool::Detail_Texture()
{
	//���⼭ ��Į�� ARM, N, BC ������ �ؽ��ĸ� ���� �� �� �ְ� �ϰ�ʹ�.
	//Imgui �ַ�� Ž���⸦ ��� dds�ؽ��ĸ� ã�Ƽ� �ȾƳ־����

//	ImGui::Text("Texture");
//	if (m_pFocusObject == nullptr)
//		return;
//
//	IGFD::FileDialogConfig Config;
//	Config.path = "../Bin/Resources/Textures/Decal/";
//
//#pragma region ARM �ؽ�ó
//	string ARMT_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)]);
//	string N_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)]);
//	string BC_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)]);
//
//	ImGui::TextWrapped(ARMT_Name.c_str());
//	// ARM �ؽ�ó ���� ��ư
//	if (ImGui::Button("Change ARMT Texture"))
//	{
//		ImGuiFileDialog::Instance()->OpenDialog("ChooseARMTTexture", "Select ARMT Texture (.dds)", ".dds", Config);
//	}
//	if (ImGuiFileDialog::Instance()->Display("ChooseARMTTexture"))
//	{
//		if (ImGuiFileDialog::Instance()->IsOk())
//		{
//			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
//			//����η� ��������
//			filePath = ToRelativePath(filePath);
//
//			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
//
//			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
//			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::ARMT, filePath, fileName)))
//				MSG_BOX("ARMT �ؽ��� ���Ƴ��� ����");
//		}
//		ImGuiFileDialog::Instance()->Close();
//	}
//#pragma endregion
//
//#pragma region Normal �ؽ���
//	ImGui::TextWrapped(N_Name.c_str());
//	// Normal �ؽ�ó ���� ��ư
//	if (ImGui::Button("Change Normal Texture"))
//	{
//		ImGuiFileDialog::Instance()->OpenDialog("ChooseNormalTexture", "Select Normal Texture (.dds)", ".dds", Config);
//	}
//	if (ImGuiFileDialog::Instance()->Display("ChooseNormalTexture"))
//	{
//		if (ImGuiFileDialog::Instance()->IsOk())
//		{
//			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
//			//����η� ��������
//			filePath = ToRelativePath(filePath);
//
//			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
//
//			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
//			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::N, filePath, fileName)))
//				MSG_BOX("Normal �ؽ��� ���Ƴ��� ����");
//		}
//		ImGuiFileDialog::Instance()->Close();
//	}
//#pragma endregion
//
//#pragma region BaseColor �ؽ���
//	ImGui::TextWrapped(BC_Name.c_str());
//	// BaseColor �ؽ�ó ���� ��ư
//	if (ImGui::Button("Change BaseColor Texture"))
//	{
//		ImGuiFileDialog::Instance()->OpenDialog("ChooseBaseColorTexture", "Select BaseColor Texture (.dds)", ".dds", Config);
//	}
//	if (ImGuiFileDialog::Instance()->Display("ChooseBaseColorTexture"))
//	{
//		if (ImGuiFileDialog::Instance()->IsOk())
//		{
//			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
//			//����η� ��������
//			filePath = ToRelativePath(filePath);
//
//			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
//
//			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
//			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::BC, filePath, fileName)))
//				MSG_BOX("BaseColor �ؽ��� ���Ƴ��� ����");
//		}
//		ImGuiFileDialog::Instance()->Close();
//	}
//#pragma endregion



}


CMonsterTool* CMonsterTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CMonsterTool* pInstance = new CMonsterTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CMonsterTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMonsterTool::Clone(void* pArg)
{
	CMonsterTool* pInstance = new CMonsterTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonsterTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CMonsterTool::Free()
{
	__super::Free();

	Safe_Release(m_pFocusObject);
}
