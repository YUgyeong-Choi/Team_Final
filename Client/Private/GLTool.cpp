#include "GLTool.h"
#include "GameInstance.h"
#include "UI_Feature.h"

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

	m_pSequence = new CUI_Sequence();

	return S_OK;
}

void CGLTool::Priority_Update(_float fTimeDelta)
{

}

void CGLTool::Update(_float fTimeDelta)
{
	if (m_isPlay)
	{
		m_fElapsedTime += fTimeDelta;

		if (m_fElapsedTime > 0.016f)
		{
			++m_iCurrentFrame;
			m_fElapsedTime = 0.f;

		}
	}

	for (const auto& pObj : m_DynamicUIList)
	{

		if (nullptr == pObj || pObj->Get_bDead())
			continue;

		pObj->Update_UI_From_Tool(m_iCurrentFrame);
	}

	
		
}

void CGLTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CGLTool::Render()
{

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
	json JsonArrayStatic = json::array();
	

	for (auto& pObj : m_StaticUIList)
	{
		json j;

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
		JsonArrayStatic.push_back(j);
		
	}

	ofstream file("../Bin/DataFiles/UI/Temp_Static.json");

	file << JsonArrayStatic.dump(4);

	file.close();

	JsonArrayStatic.clear();

	// dynamic 따로 저장

	json JsonArrayDynamic = json::array();

	for (auto& pObj : m_DynamicUIList)
	{
		json j;

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

		// 이제 기능을 채워줘야됨

		for (auto& pFeature : pObj->Get_Features())
		{
			if (nullptr == pFeature)
				continue;
			
			auto& eFeatureDesc = pFeature->Get_Desc();

			j["ProtoTag"] = eFeatureDesc.strProtoTag;
			j["isLoop"] = eFeatureDesc.isLoop;
			j["iStartFrame"] = eFeatureDesc.iStartFrame;
			j["iEndFrame"] = eFeatureDesc.iEndFrame;

			if(eFeatureDesc.strProtoTag.find("Fade") != string::npos)
			{
				UI_FEATURE_FADE_DESC& eFadeDesc = static_cast<UI_FEATURE_FADE_DESC&>(eFeatureDesc);

				j["fTime"] = eFadeDesc.fTime;
				j["iStartAlpha"] = eFadeDesc.fStartAlpha;
				j["iEndAlpha"] = eFadeDesc.fEndAlpha;
				
			}
			else if (eFeatureDesc.strProtoTag.find("Pos") != string::npos)
			{
				UI_FEATURE_POS_DESC& ePosDesc = static_cast<UI_FEATURE_POS_DESC&>(eFeatureDesc);

				j["fTime"] = ePosDesc.fTime;

				j["fStartPos"]["x"] = ePosDesc.fStartPos.x;
				j["fStartPos"]["y"] = ePosDesc.fStartPos.y;

				j["fEndPos"]["x"] = ePosDesc.fEndPos.x;
				j["fEndPos"]["y"] = ePosDesc.fEndPos.y;
			
			}
			else if (eFeatureDesc.strProtoTag.find("UV") != string::npos)
			{
				UI_FEATURE_UV_DESC& ePosDesc = static_cast<UI_FEATURE_UV_DESC&>(eFeatureDesc);

				j["fStartUV"]["x"] = ePosDesc.fStartUV.x;
				j["fStartUV"]["y"] = ePosDesc.fStartUV.y;

				j["fOffsetUV"]["x"] = ePosDesc.fOffsetUV.x;
				j["fOffsetUV"]["y"] = ePosDesc.fOffsetUV.y;

			}
			

		}
		

		JsonArrayDynamic.push_back(j);

	}

	ofstream ofile("../Bin/DataFiles/UI/Temp_Dynamic.json");

	ofile << JsonArrayDynamic.dump(4);

	ofile.close();

}

void CGLTool::Open_File()
{
	

	
	IGFD::FileDialogConfig config;

	m_strSavePath = R"(../Bin/DataFiles/UI)";
	config.path = m_strSavePath;
	config.countSelectionMax = 0; // 무제한
	

	IFILEDIALOG->OpenDialog("JsonDialog", "Select json File", ".json", config);

	
}

void CGLTool::Add_Static_UI_From_File()
{

	string filePath = IFILEDIALOG->GetFilePathName();

	json j;

	ifstream file(filePath);

	file >> j;

	for (const auto& eUIJson : j)
	{
		CStatic_UI::STATIC_UI_DESC eStaticDesc = {};

		

		eStaticDesc.fOffset = eUIJson["Offset"]  ;
		eStaticDesc.iPassIndex = eUIJson["PassIndex"];
		eStaticDesc.iTextureIndex = eUIJson["TextureIndex"]; 
		eStaticDesc.fSizeX = eUIJson["fSizeX"].get<float>() * g_iWinSizeX;
		eStaticDesc.fSizeY = eUIJson["fSizeY"].get<float>() * g_iWinSizeY;
		eStaticDesc.fX = eUIJson["fX"].get<float>() * g_iWinSizeX;
		eStaticDesc.fY = eUIJson["fY"].get<float>() * g_iWinSizeY;

		string textureTag = eUIJson["TextureTag"];
		eStaticDesc.strTextureTag = wstring(textureTag.begin(), textureTag.end());

		if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
			static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eStaticDesc)))
			return;

		auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

		m_StaticUIList.push_back(static_cast<CStatic_UI*>(pObj));
	}

	file.close();
}

void CGLTool::Add_Dynamic_UI_From_File()
{
	//
}

void CGLTool::Add_Static_UI()
{
	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eStaticUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

	m_StaticUIList.push_back(static_cast<CStatic_UI*>(pObj));
}

void CGLTool::Add_Dynamic_UI()
{
	eDynamicUIDesc.isFromTool = true;

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"), &eDynamicUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"));

	m_DynamicUIList.push_back(static_cast<CDynamic_UI*>(pObj));
}

void CGLTool::Add_Sequence_To_DynamicUI()
{
	if (nullptr == m_pSelectDynamicObj)
		return;

	m_pSequence->Clear();

	for (auto& pFeature : m_pSelectDynamicObj->Get_Features())
	{
		if (nullptr == pFeature)
			continue;

		auto eDesc = pFeature->Get_Desc_From_Tool();
		m_pSequence->Push_Item(eDesc);
	}

	

	//m_pSelectDynamicObj->Add_Feature();
}

void CGLTool::Apply_Sequence_To_DynamicUI()
{
	if (nullptr == m_pSelectDynamicObj)
		return;

	for (auto& pFeature : m_pSelectDynamicObj->Get_Features())
	{
		Safe_Release(pFeature);
	}


	for (int i = 0; i < m_pSequence->GetItemCount(); ++i)
	{
		auto eDesc = m_pSequence->Get_Desc(i);

		if (0 == eDesc.iType)
		{
			// fade
			UI_FEATURE_FADE_DESC fadeDesc = {};
			fadeDesc.strProtoTag = "Prototype_Component_UI_Feature_Fade";
			fadeDesc.isLoop = eDesc.isLoop;
			fadeDesc.iStartFrame = eDesc.iStartFrame;
			fadeDesc.iEndFrame = eDesc.iEndFrame;
			fadeDesc.fTime = eDesc.fTime;
			fadeDesc.fStartAlpha = eDesc.fStartAlpha;
			fadeDesc.fEndAlpha = eDesc.fEndAlpha;

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(fadeDesc.strProtoTag), &fadeDesc);
			
		}
		else if (1 == eDesc.iType)
		{
			// uv
			UI_FEATURE_UV_DESC uvDesc = {};
			uvDesc.strProtoTag = "Prototype_Component_UI_Feature_UV";
			uvDesc.isLoop = eDesc.isLoop;
			uvDesc.iStartFrame = eDesc.iStartFrame;
			uvDesc.iEndFrame = eDesc.iEndFrame;
			uvDesc.fStartUV = eDesc.fStartUV;
			uvDesc.fOffsetUV = eDesc.fOffsetUV;

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(uvDesc.strProtoTag), &uvDesc);
		}
		else if (2 == eDesc.iType)
		{
			UI_FEATURE_POS_DESC posDesc = {};
			posDesc.strProtoTag = "Prototype_Component_UI_Feature_Pos";
			posDesc.isLoop = eDesc.isLoop;
			posDesc.iStartFrame = eDesc.iStartFrame;
			posDesc.iEndFrame = eDesc.iEndFrame;
			posDesc.fStartPos = eDesc.fStartPos;
			posDesc.fEndPos = eDesc.fEndPos;
			posDesc.fTime = eDesc.fTime;

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(posDesc.strProtoTag), &posDesc);
		}
	}
}

HRESULT CGLTool::Render_SelectOptionTool()
{
	
	SetNextWindowSize(ImVec2(400, 400));
	_bool open = true;
	ImGui::Begin("Select option ", &open, NULL);

	//float value = 0.6f;
	//ImGui::SliderFloat("Offset", &value, 0.0f, 1.0f);

	if (ImGui::BeginTabBar("Util"))
	{
		if (ImGui::BeginTabItem("Input Static Desc"))
		{
			// 입력 칸 만들기

			ImGui::InputFloat("Offset", &eStaticUITempDesc.fOffset);
			ImGui::InputInt("PassIndex", &eStaticUITempDesc.iPassIndex);
			ImGui::InputInt("TextureIndex", &eStaticUITempDesc.iTextureIndex);
			ImGui::InputFloat("SizeX_Ratio", &eStaticUITempDesc.fSizeX);
			ImGui::InputFloat("SizeY_Ratio", &eStaticUITempDesc.fSizeY);
			ImGui::InputFloat("fX_Ratio", &eStaticUITempDesc.fX);
			ImGui::InputFloat("fY_Ratio", &eStaticUITempDesc.fY);

			// 필요하면 슬라이더로
			ImGui::SliderFloat("SizeX_Slider", &eStaticUITempDesc.fSizeX, 0.001f, 1.5f);
			ImGui::SliderFloat("SizeY_Slider", &eStaticUITempDesc.fSizeY, 0.001f, 1.5f);
			ImGui::SliderFloat("fX_Slider", &eStaticUITempDesc.fX, -0.5f, 1.5f);
			ImGui::SliderFloat("fY_Slider", &eStaticUITempDesc.fY, -0.5f, 1.5f);

			eStaticUIDesc = eStaticUITempDesc;
			eStaticUIDesc.fSizeX *= g_iWinSizeX;
			eStaticUIDesc.fSizeY *= g_iWinSizeY;
			eStaticUIDesc.fX *= g_iWinSizeX;
			eStaticUIDesc.fY *= g_iWinSizeY;

			eStaticUIDesc.strTextureTag = m_strSelectName;

			if (eStaticUIDesc.iPassIndex >= UI_END)
			{
				eStaticUIDesc.iPassIndex = UI_END - 1;
				eStaticUITempDesc.iPassIndex = UI_END - 1;
			}

			if (eStaticUIDesc.iPassIndex < 0)
			{
				eStaticUIDesc.iPassIndex = 0;
				eStaticUITempDesc.iPassIndex = 0;
			}

			if (nullptr == m_pSelectStaticObj || m_pSelectStaticObj->Get_bDead())
			{

			}
			else
			{
				m_pSelectStaticObj->Update_UI_From_Tool(eStaticUIDesc);
			}


			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Input Dynamic Desc"))
		{
			// 입력 칸 만들기

			// 입력 칸 만들기

			ImGui::InputFloat("Offset", &eDynamicUITempDesc.fOffset);
			ImGui::InputInt("PassIndex", &eDynamicUITempDesc.iPassIndex);
			ImGui::InputInt("TextureIndex", &eDynamicUITempDesc.iTextureIndex);
			ImGui::InputFloat("SizeX_Ratio", &eDynamicUITempDesc.fSizeX);
			ImGui::InputFloat("SizeY_Ratio", &eDynamicUITempDesc.fSizeY);
			ImGui::InputFloat("fX_Ratio", &eDynamicUITempDesc.fX);
			ImGui::InputFloat("fY_Ratio", &eDynamicUITempDesc.fY);

			// 필요하면 슬라이더로
			ImGui::SliderFloat("SizeX_Slider", &eDynamicUITempDesc.fSizeX, 0.001f, 1.5f);
			ImGui::SliderFloat("SizeY_Slider", &eDynamicUITempDesc.fSizeY, 0.001f, 1.5f);
			ImGui::SliderFloat("fX_Slider", &eDynamicUITempDesc.fX, -0.5f, 1.5f);
			ImGui::SliderFloat("fY_Slider", &eDynamicUITempDesc.fY, -0.5f, 1.5f);

			eDynamicUIDesc = eDynamicUITempDesc;
			eDynamicUIDesc.fSizeX *= g_iWinSizeX;
			eDynamicUIDesc.fSizeY *= g_iWinSizeY;
			eDynamicUIDesc.fX *= g_iWinSizeX;
			eDynamicUIDesc.fY *= g_iWinSizeY;

			eDynamicUIDesc.strTextureTag = m_strSelectName;

			if (eDynamicUIDesc.iPassIndex >= D_UI_END)
			{
				eDynamicUIDesc.iPassIndex = D_UI_END - 1;
				eDynamicUITempDesc.iPassIndex = D_UI_END - 1;
			}

			if (eDynamicUIDesc.iPassIndex < 0)
			{
				eDynamicUIDesc.iPassIndex = 0;
				eDynamicUITempDesc.iPassIndex = 0;
			}

			if (nullptr == m_pSelectDynamicObj || m_pSelectDynamicObj->Get_bDead())
			{

			}
			else
			{
				m_pSelectDynamicObj->Update_UI_From_Tool(eDynamicUIDesc);
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Input Sequence Desc"))
		{
			// 입력 칸 만들기
			ImGui::Checkbox("IsLoop", &m_eFeatureDesc.isLoop);
			ImGui::InputInt("Type", &m_eFeatureDesc.iType);
			ImGui::InputInt("StartFrame", &m_eFeatureDesc.iStartFrame);
			ImGui::InputInt("EndFrame", &m_eFeatureDesc.iEndFrame);

			ImGui::InputFloat("StartAlpha", &m_eFeatureDesc.fStartAlpha);
			ImGui::InputFloat("EndAlpha", &m_eFeatureDesc.fEndAlpha);
			ImGui::InputFloat("FadeTime", &m_eFeatureDesc.fTime);

			ImGui::InputFloat2("StartUV", reinterpret_cast<float*>(& m_eFeatureDesc.fStartUV));
			ImGui::InputFloat2("OffsetUV", reinterpret_cast<float*>(& m_eFeatureDesc.fOffsetUV));

			ImGui::InputFloat2("StartPos", reinterpret_cast<float*>(&m_eFeatureDesc.fStartPos));
			ImGui::InputFloat2("EndPos", reinterpret_cast<float*>(&m_eFeatureDesc.fEndPos));


			if (0 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "Fade";
			else if (1 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "UV";
			else if (2 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "Pos";

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Button"))
		{

			IGFD::FileDialogConfig config;
			if (Button(u8"Save File"))
			{
				Save_File();
			}

			if (Button(u8"Open File"))
			{
				Open_File();

			}

			if (Button(u8"Add Static UI"))
			{
				Add_Static_UI();
			}

			if (Button(u8"Add Dynamic UI"))
			{
				Add_Dynamic_UI();
			}

			if (Button(u8"Delete Static UI"))
			{
				if (nullptr != m_pSelectStaticObj)
					m_pSelectStaticObj->Set_bDead();
				m_iSelectObjIndex = -1;
			}

			if (Button(u8"Delete Dynamic UI"))
			{
				if (nullptr != m_pSelectDynamicObj)
					m_pSelectDynamicObj->Set_bDead();
				m_iDynamicObjIndex = -1;
			}

			if (Button(u8"Add Sequence"))
			{
				m_pSequence->Add(0);
				// 추가 한거 마지막에 만든 정보 삽입
				m_pSequence->Upadte_Items(m_pSequence->GetItemCount() - 1, m_eFeatureDesc);
			}

			if (Button(u8"Apply Sequence"))
			{
				// ui에 담아주기
				
				
				Apply_Sequence_To_DynamicUI();

			}

			if (Button(u8"update Select UI Sequence"))
			{
				// 현재 선택한 ui의 시퀀스를 띄운다
				Add_Sequence_To_DynamicUI();

			}
		
			if (Button(u8"Play Sequence"))
			{
				m_isPlay = true;
			}

			if (Button(u8"Stop Sequence and reset"))
			{
				m_isPlay = false;
				m_iCurrentFrame = 0;

				for (const auto& pObj : m_DynamicUIList)
				{

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					pObj->Reset();
				}
			}




			// 파일 탐색기를 띄운다
			ImGui::SetNextWindowSize(ImVec2(800, 600));
			if (IFILEDIALOG->Display("JsonDialog"))
			{
				if (IFILEDIALOG->IsOk())
				{
					string fileName = IFILEDIALOG->GetCurrentFileName();

					if (fileName.find("Static"))
					{
						Add_Static_UI_From_File();
					}
					else if (fileName.find("Dynamic"))
					{

					}

					

				}
				IFILEDIALOG->Close();
			}


			ImGui::EndTabItem();
		}


		ImGui::EndTabBar();
	}

		

	ImGui::End();
	return S_OK;
}

HRESULT CGLTool::Render_UIList()
{
	SetNextWindowSize(ImVec2(400, 300));

	_bool open = true;
	ImGui::Begin("UIList", &open, NULL);
	{
		if (ImGui::BeginTabBar("MyTabBar"))
		{
			if (ImGui::BeginTabItem("Static"))
			{
				int index = 0;

				for (const auto& pObj : m_StaticUIList)
				{
					bool isSelected = (index == m_iSelectObjIndex);

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					if (ImGui::Selectable(to_string(index).c_str(), isSelected))
					{
						m_iSelectObjIndex = index;
						m_pSelectStaticObj = pObj;
						eStaticUITempDesc = m_pSelectStaticObj->Get_Desc();
						eStaticUITempDesc.fSizeX /= g_iWinSizeX;
						eStaticUITempDesc.fSizeY /= g_iWinSizeY;
						eStaticUITempDesc.fX /= g_iWinSizeX;
						eStaticUITempDesc.fY /= g_iWinSizeY;
						eStaticUITempDesc.strTextureTag = pObj->Get_StrTextureTag();

					}


					++index;
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Dynamic"))
			{

				int index = 0;

				for (const auto& pObj : m_DynamicUIList)
				{
					bool isSelected = (index == m_iDynamicObjIndex);

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					if (ImGui::Selectable(to_string(index).c_str(), isSelected))
					{
						m_iDynamicObjIndex = index;
						m_pSelectDynamicObj = pObj;
						eDynamicUIDesc = m_pSelectDynamicObj->Get_Desc();
						eDynamicUIDesc.fSizeX /= g_iWinSizeX;
						eDynamicUIDesc.fSizeY /= g_iWinSizeY;
						eDynamicUIDesc.fX /= g_iWinSizeX;
						eDynamicUIDesc.fY /= g_iWinSizeY;
						eDynamicUIDesc.strTextureTag = pObj->Get_StrTextureTag();

					
					}


					++index;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Texture"))
			{


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

				ImGui::EndTabItem();
			}


			ImGui::EndTabBar();

		}


		ImGui::End();
	}

	
	
	return S_OK;
}

HRESULT CGLTool::Render_Sequence()
{
	SetNextWindowSize(ImVec2(800, 200));
	_bool open = true;
	ImGui::Begin("Sequence ", &open, NULL);

	//
	
	
	int iFirstFrame = 0;
	ImSequencer::Sequencer(m_pSequence,
		&m_iCurrentFrame,
		&m_bExpanded,
		&m_iSelectedEntry,
		&iFirstFrame, // Optional callback for drawing custom UI
		ImSequencer::SEQUENCER_EDIT_ALL |
		ImSequencer::SEQUENCER_ADD |
		ImSequencer::SEQUENCER_DEL |
		ImSequencer::SEQUENCER_COPYPASTE);

	if (m_iSelectedEntry != -1)
	{
		m_eFeatureDesc = m_pSequence->Get_Desc(m_iSelectedEntry);
	}

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

	Safe_Delete(m_pSequence);

}
