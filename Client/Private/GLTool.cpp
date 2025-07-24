#include "GLTool.h"
#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CGLTool::CGLTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CGLTool::CGLTool(const CGLTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CGLTool::Initialize_Prototype()
{

	
	
	return S_OK;
}

HRESULT CGLTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	auto map = m_pGameInstance->Get_Prototypes();

	for (auto& pair : *map)
	{
		if(pair.first.find(L"Texture") != pair.first.npos)
			m_TextureNames.push_back(pair.first);
	}

	return S_OK;
}

void CGLTool::Priority_Update(_float fTimeDelta)
{

}

void CGLTool::Update(_float fTimeDelta)
{
}

void CGLTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CGLTool::Render()
{
	if (FAILED(Render_UtilTool()))
		return E_FAIL;

	if (FAILED(Render_SelectOptionTool()))
		return E_FAIL;

	if (FAILED(Render_UIList()))
		return E_FAIL;

	if (FAILED(Render_Sequence()))
		return E_FAIL;

	return S_OK;
}

void CGLTool::Save_File()
{
	json JsonArray = json::array();
	json j;

	for (auto& pObj : m_UIList)
	{
		if (nullptr == pObj || pObj->Get_bDead())
			continue;
		auto eDesc = pObj->Get_Desc();

		// 이제 값 채워서 dump로 넘겨준다 
		j["TextureTag"] = WStringToString(eDesc.strTextureTag);
		j["PassIndex"] = eDesc.iPassIndex;
		j["TextureIndex"] = eDesc.iTextureIndex;
		j["Offset"] = eDesc.fOffset;
		j["fSizeX"] = eDesc.fSizeX / g_iWinSizeX;
		j["fSizeY"] = eDesc.fSizeY / g_iWinSizeY;
		j["fX"] = eDesc.fX / g_iWinSizeX;
		j["fY"] = eDesc.fY / g_iWinSizeY;
		JsonArray.push_back(j);
		
	}

	ofstream file("../Bin/DataFiles/UI/Temp.json");

	file << JsonArray.dump(4);

	file.close();
}

void CGLTool::Open_File()
{
	

	
	IGFD::FileDialogConfig config;

	m_strSavePath = R"(../Bin/DataFiles/UI)";
	config.path = m_strSavePath;
	config.countSelectionMax = 0; // 무제한
	

	IFILEDIALOG->OpenDialog("JsonDialog", "Select json File", ".json", config);

	
}

void CGLTool::Add_UI_From_File()
{

	string filePath = IFILEDIALOG->GetFilePathName();

	json j;

	ifstream file(filePath);

	file >> j;

	for (const auto& eUIJson : j)
	{
		CStatic_UI::STATIC_UI_DESC eStaticDesc = {};

		_float tmpx = 1920.f / 1280.f;
		_float tmpy = 1080.f / 720.f;

		eStaticDesc.fOffset = eUIJson["Offset"]  ;
		eStaticDesc.iPassIndex = eUIJson["PassIndex"];
		eStaticDesc.iTextureIndex = eUIJson["TextureIndex"]; 
		eStaticDesc.fSizeX = eUIJson["fSizeX"] * tmpx;
		eStaticDesc.fSizeY = eUIJson["fSizeY"] * tmpy;
		eStaticDesc.fX = eUIJson["fX"] * tmpx;
		eStaticDesc.fY = eUIJson["fY"] * tmpy;

		string textureTag = eUIJson["TextureTag"];
		eStaticDesc.strTextureTag = wstring(textureTag.begin(), textureTag.end());

		if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
			static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eStaticDesc)))
			return;

		auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

		m_UIList.push_back(static_cast<CStatic_UI*>(pObj));
	}

	file.close();
}

void CGLTool::Add_UI()
{
	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

	m_UIList.push_back(static_cast<CStatic_UI*>(pObj));
}

HRESULT CGLTool::Render_UtilTool()
{
	SetNextWindowSize(ImVec2(500, 300));
	_bool open = true;
	Begin("GL Tools", &open, NULL);

	IGFD::FileDialogConfig config;
	if (Button(u8"Save File"))
	{
		Save_File();
	}

	if (Button(u8"Open File"))
	{
		Open_File();

	}

	if (Button(u8"Add UI"))
	{
		Add_UI();
	}

	if (Button(u8"Delete UI"))
	{
		if (nullptr != m_pSelectObj)
			m_pSelectObj->Set_bDead();
	}

	int index = 0;

	for (const auto& strName : m_TextureNames)
	{
		bool isSelected = (index == m_iSelectTextureIndex);

		if (ImGui::Selectable(WStringToString(strName).c_str(), isSelected))
		{
			m_iSelectTextureIndex = index;
			m_strSelectName = strName;
		}
		++index;
	}

	


	// 파일 탐색기를 띄운다
	ImGui::SetNextWindowSize(ImVec2(800, 600));
	if (IFILEDIALOG->Display("JsonDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			
			Add_UI_From_File();

		}
		IFILEDIALOG->Close();
	}

	ImGui::End();


	

	return S_OK;
}

HRESULT CGLTool::Render_SelectOptionTool()
{
	
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Select option ", &open, NULL);

	float value = 0.6f;
	ImGui::SliderFloat("Offset", &value, 0.0f, 1.0f);


	// 입력 칸 만들기

	ImGui::InputFloat("Offset", &eUITempDesc.fOffset);
	ImGui::InputFloat("SizeX_Ratio", &eUITempDesc.fSizeX);
	ImGui::InputFloat("SizeY_Ratio", &eUITempDesc.fSizeY);
	ImGui::InputFloat("fX_Ratio", &eUITempDesc.fX);
	ImGui::InputFloat("fY_Ratio", &eUITempDesc.fY);
	ImGui::InputInt("PassIndex", &eUITempDesc.iPassIndex);
	ImGui::InputInt("TextureIndex", &eUITempDesc.iTextureIndex);


	// apply 누르면 내가 선언한 변수에 담도록 하기?
	if (ImGui::Button("Apply"))
	{
		eUIDesc = eUITempDesc;
		eUIDesc.fSizeX *= g_iWinSizeX;
		eUIDesc.fSizeY *= g_iWinSizeY;
		eUIDesc.fX *= g_iWinSizeX;
		eUIDesc.fY *= g_iWinSizeY;

		eUIDesc.strTextureTag = m_strSelectName;
	}

	if (ImGui::Button("Change Select UI"))
	{
		m_pSelectObj->Update_UI_From_Tool(eUIDesc);
	}


	ImGui::End();
	return S_OK;
}

HRESULT CGLTool::Render_UIList()
{
	
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("UI List", &open, NULL);

	// 만들어진 uilist 

	int index = 0;
	
	for (const auto& pObj : m_UIList)
	{
		bool isSelected = (index == m_iSelectObjIndex);

		if (nullptr == pObj || pObj->Get_bDead())
			continue;
		
		if (ImGui::Selectable(WStringToString(pObj->Get_StrTextureTag()).c_str(), isSelected))
		{
			m_iSelectObjIndex = index;
			m_pSelectObj = pObj;
			eUITempDesc = m_pSelectObj->Get_Desc();
			eUITempDesc.fSizeX /= g_iWinSizeX;
			eUITempDesc.fSizeY /= g_iWinSizeY;
			eUITempDesc.fX /= g_iWinSizeX;
			eUITempDesc.fY /= g_iWinSizeY;
			eUITempDesc.strTextureTag = pObj->Get_StrTextureTag();
		}
		
		++index;
	}

	ImGui::End();
	return S_OK;
}

HRESULT CGLTool::Render_Sequence()
{
	SetNextWindowSize(ImVec2(800, 200));
	_bool open = true;
	ImGui::Begin("Sequence ", &open, NULL);

	//                                                                                     

	ImGui::End();

	return S_OK;
}

CGLTool* CGLTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CGLTool* pInstance = new CGLTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CGLTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGLTool::Clone(void* pArg)
{
	CGLTool* pInstance = new CGLTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGLTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CGLTool::Free()
{
	__super::Free();

}
