#include "MapTool.h"
#include "GameInstance.h"

#include <filesystem>
#include "MapToolObject.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��

CMapTool::CMapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CMapTool::CMapTool(const CMapTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CMapTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	return S_OK;
}

void CMapTool::Priority_Update(_float fTimeDelta)
{

}

void CMapTool::Update(_float fTimeDelta)
{
}

void CMapTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CMapTool::Render()
{
	if (FAILED(Render_MapTool()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapTool::Render_MapTool()
{
	Hierarchy();
	Asset();

	return S_OK;
}

void CMapTool::Hierarchy()
{
#pragma region ���̾��Ű
	ImGui::Begin("Hierarchy", nullptr);
	if (ImGui::Button("Save Map"))
	{
		list<CGameObject*>& MapObjectList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_MapToolObject"));

		for (CGameObject* pGameObject : MapObjectList)
		{
			_matrix matWorld = pGameObject->Get_TransfomCom()->Get_WorldMatrix();
			_float4x4 matWorldFloat4x4;
			XMStoreFloat4x4(&matWorldFloat4x4, matWorld);

		}

		filesystem::create_directories("../Bin/Save/MapTool");
		ofstream file("../Bin/Save/MapTool/MapData.json");
		file.close();
	}


	if (ImGui::Button("Load Map"))
	{

	}
	ImGui::End();
#pragma endregion
}

void CMapTool::Asset()
{
#pragma region ����
	ImGui::Begin("Asset", nullptr);

	if (ImGui::Button("Spawn"))
	{
		if (FAILED(Spawn_MapToolObject()))
		{
			MSG_BOX("���� ����");
		}
	}

	if (ImGui::BeginListBox("##Model List", ImVec2(-FLT_MIN, 200)))
	{
		/*
		* �� ������ �����Կ� ����ִ� ��� <���̸�>�� ���
		*/

		for (_int i = 0; i < m_ModelNames.size(); ++i)
		{
			const _bool isSelected = (m_iSelectedModelIndex == i);
			if (ImGui::Selectable(m_ModelNames[i].c_str(), isSelected))
			{
				m_iSelectedModelIndex = i;
			}

			// ���õ� �׸� ��Ŀ��
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();

	}

	/*
		���� Ž���� ����
		�ش� ��.bin �� ��θ� �����ͼ�
		�� ������ �����.

		_matrix		PreTransformMatrix = XMMatrixIdentity();

		PreTransformMatrix = XMMatrixIdentity();
		PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_���̸�"),
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/���̸�.bin", PreTransformMatrix))))
			return E_FAIL;

		�� �������� �����Ѵ�.
	*/
	if (ImGui::Button("Load Model"))
	{
		IGFD::FileDialogConfig config;
		config.path = "../Bin/Resources/Models/Bin_NonAnim";
		config.countSelectionMax = 1;

		IFILEDIALOG->OpenDialog("Load Model", "Load .bin model", ".bin", config);
	}

	// �� �����Ӹ��� Display ȣ��
	if (IFILEDIALOG->Display("Load Model"))
	{
		if (IFILEDIALOG->IsOk())
		{
			filesystem::path ModelPath = IFILEDIALOG->GetFilePathName();

			if (!ModelPath.empty())
			{
				// Prototype �̸� ����
				wstring PrototypeTag = L"Prototype_Component_Model_" + ModelPath.stem().wstring();
				string strPrototypeTag = ModelPath.stem().string(); // Ȯ���� ���� ���� �̸���

				string ModelFilePath = ModelPath.string();
				const _char* pModelFilePath = ModelFilePath.c_str();

				if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
				{
					MSG_BOX("�ε� ����");
				}
				else
				{
					m_ModelNames.push_back(strPrototypeTag);
				}
			}
		}


		IFILEDIALOG->Close();
	}

	ImGui::End();

#pragma endregion
}

HRESULT CMapTool::Spawn_MapToolObject()
{

	if (m_iSelectedModelIndex == -1)
		return E_FAIL;

	//���� ���õ� �𵨷� �ʿ�����Ʈ�� ����

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_���̸�"),
	wstring ModelName = wstring(m_ModelNames[m_iSelectedModelIndex].begin(), m_ModelNames[m_iSelectedModelIndex].end());
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
	ModelPrototypeTag += ModelName;
	lstrcpy(MapToolObjDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	/*
	������ ������ �� � ���̾ ��Ҵ��� �����ϴ� ���̾� ����Ʈ�� �ʿ���
	������ �� ���̾ �����ϴ� �� Ȯ��(���̾ ������ �������� �ʴٴ� ��)
	�� ���̾ ������� ����Ʈ���� ���� �ؾ���
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject");
	LayerTag += ModelName;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapTool::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
		return E_FAIL;

	return S_OK;
}


CMapTool* CMapTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CMapTool* pInstance = new CMapTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CMapTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapTool::Clone(void* pArg)
{
	CMapTool* pInstance = new CMapTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CMapTool::Free()
{
	__super::Free();

}
