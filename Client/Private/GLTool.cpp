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

void CGLTool::Obj_Serialize()
{
	json JsonArray = json::array();


	for (auto& pObj : m_StaticUIList)
	{
		if(pObj != nullptr)
			JsonArray.push_back(pObj->Serialize());
	}
	

	for (auto& pObj : m_DynamicUIList)
		if (pObj != nullptr)
			JsonArray.push_back(pObj->Serialize());

	for (auto& pObj : m_TextUIList)
		if (pObj != nullptr)
			JsonArray.push_back(pObj->Serialize());

	ofstream file("../Bin/DataFiles/UI/Temp.json");

	file << JsonArray.dump(4);

	file.close();

}

void CGLTool::Obj_Deserialize()
{
	string filePath = IFILEDIALOG->GetFilePathName();

	json j;

	ifstream file(filePath);

	file >> j;

	for (const auto& eUIJson : j)
	{
		string protoTag = eUIJson["ProtoTag"];
		
		if ("Prototype_GameObject_Static_UI" == protoTag)
		{
			CStatic_UI::STATIC_UI_DESC eDesc = {};

			string textureTag = eUIJson["Texturetag"];
			eDesc.strTextureTag = StringToWStringU8(textureTag);
			eDesc.iTextureLevel = eUIJson["iTextureLevel"];


			(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), ENUM_CLASS(LEVEL::GL), TEXT("Layer_Background_Static"), &eDesc));

			CStatic_UI* pObj = static_cast<CStatic_UI*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::GL), TEXT("Layer_Background_Static")));

			pObj->Deserialize(eUIJson);

			pObj->Update_Data();

			m_StaticUIList.push_back(pObj);

		}
		else if ("Prototype_GameObject_Dynamic_UI" == protoTag)
		{
			CDynamic_UI::DYNAMIC_UI_DESC eDesc = {};

			string textureTag = eUIJson["Texturetag"];
			eDesc.strTextureTag = StringToWStringU8(textureTag);
			eDesc.iTextureLevel = eUIJson["iTextureLevel"];


			(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), ENUM_CLASS(LEVEL::GL), TEXT("Layer_Background_Dynamic"), &eDesc));

			CDynamic_UI* pObj = static_cast<CDynamic_UI*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::GL), TEXT("Layer_Background_Dynamic")));

			pObj->Deserialize(eUIJson);

			pObj->Update_Data();

			m_DynamicUIList.push_back(pObj);

		}
		else if ("Prototype_GameObject_UI_Text" == protoTag)
		{
			CUI_Text::TEXT_UI_DESC eDesc = {};

			(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), ENUM_CLASS(LEVEL::GL), TEXT("Layer_Background_Text"), &eDesc));

			CUI_Text* pObj = static_cast<CUI_Text*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::GL), TEXT("Layer_Background_Text")));

			pObj->Deserialize(eUIJson);

			pObj->Update_Data();

			m_TextUIList.push_back(pObj);
		}

	}


	
}



void CGLTool::Open_File()
{
	
	IGFD::FileDialogConfig config;

	m_strSavePath = R"(../Bin/DataFiles/UI)";
	config.path = m_strSavePath;
	config.countSelectionMax = 0; // 무제한
	

	IFILEDIALOG->OpenDialog("JsonDialog", "Select json File", ".json", config);

	
}



void CGLTool::Add_Static_UI()
{
	eStaticUIDesc.strTextureTag = m_strSelectName;

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eStaticUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

	m_StaticUIList.push_back(static_cast<CStatic_UI*>(pObj));
}

void CGLTool::Add_Dynamic_UI()
{
	eDynamicUIDesc.isFromTool = true;

	eStaticUIDesc.strTextureTag = m_strSelectName;

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"), &eDynamicUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"));

	m_DynamicUIList.push_back(static_cast<CDynamic_UI*>(pObj));
}

void CGLTool::Add_UI_Text()
{
	
	m_eTextUIDesc.strFontTag = TEXT("Font_Medium");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Text"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Text"), &m_eTextUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Text"));

	m_TextUIList.push_back(static_cast<CUI_Text*>(pObj));

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

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(posDesc.strProtoTag), &posDesc);
		}

		else if (3 == eDesc.iType)
		{
			UI_FEATURE_SCALE_DESC scaleDesc = {};
			scaleDesc.strProtoTag = "Prototype_Component_UI_Feature_Scale";
			scaleDesc.isLoop = eDesc.isLoop;
			scaleDesc.iStartFrame = eDesc.iStartFrame;
			scaleDesc.iEndFrame = eDesc.iEndFrame;
			scaleDesc.fStartScale = eDesc.fStartScale;
			scaleDesc.fEndScale = eDesc.fEndScale;
			

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(scaleDesc.strProtoTag), &scaleDesc);
		}
	}
}

void CGLTool::Input_Static_Desc()
{
	ImGui::InputFloat4("Color", reinterpret_cast<float*>(&eStaticUITempDesc.vColor));

	ImGui::InputFloat("Offset", &eStaticUITempDesc.fOffset);
	ImGui::InputInt("PassIndex", &eStaticUITempDesc.iPassIndex);
	ImGui::InputInt("TextureIndex", &eStaticUITempDesc.iTextureIndex);
	ImGui::InputFloat("SizeX_Ratio", &eStaticUITempDesc.fSizeX);
	if (eStaticUITempDesc.fSizeX <= 0.0f)
		eStaticUITempDesc.fSizeX = 0.001f;
	ImGui::InputFloat("SizeY_Ratio", &eStaticUITempDesc.fSizeY);
	if (eStaticUITempDesc.fSizeY <= 0.0f)
		eStaticUITempDesc.fSizeY = 0.001f;
	ImGui::InputFloat("fX_Ratio", &eStaticUITempDesc.fX);
	ImGui::InputFloat("fY_Ratio", &eStaticUITempDesc.fY);

	// 필요하면 슬라이더로
	ImGui::SliderFloat("SizeX_Slider", &eStaticUITempDesc.fSizeX, 0.001f, 1.5f);
	ImGui::SliderFloat("SizeY_Slider", &eStaticUITempDesc.fSizeY, 0.001f, 1.5f);
	ImGui::SliderFloat("fX_Slider", &eStaticUITempDesc.fX, -0.5f, 1.5f);
	ImGui::SliderFloat("fY_Slider", &eStaticUITempDesc.fY, -0.5f, 1.5f);

	ImGui::SliderFloat("Rotation", &eStaticUITempDesc.fRotation, 0.f, 360.f);


	eStaticUIDesc = eStaticUITempDesc;
	eStaticUIDesc.fSizeX *= g_iWinSizeX;
	eStaticUIDesc.fSizeY *= g_iWinSizeY;
	eStaticUIDesc.fX *= g_iWinSizeX;
	eStaticUIDesc.fY *= g_iWinSizeY;
	eStaticUIDesc.fRotation = eStaticUITempDesc.fRotation;

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
}

void CGLTool::Input_Dynamic_Desc()
{
	// 입력 칸 만들기
	ImGui::InputFloat4("Color", reinterpret_cast<float*>(&eDynamicUITempDesc.vColor));
	ImGui::InputFloat("Duration", &eDynamicUITempDesc.fDuration);
	ImGui::InputFloat("Offset", &eDynamicUITempDesc.fOffset);
	ImGui::InputInt("PassIndex", &eDynamicUITempDesc.iPassIndex);
	ImGui::InputInt("TextureIndex", &eDynamicUITempDesc.iTextureIndex);
	ImGui::InputFloat("SizeX_Ratio", &eDynamicUITempDesc.fSizeX);
	if (eDynamicUITempDesc.fSizeX <= 0.0f)
		eDynamicUITempDesc.fSizeX = 0.001f;
	ImGui::InputFloat("SizeY_Ratio", &eDynamicUITempDesc.fSizeY);
	if (eDynamicUITempDesc.fSizeY <= 0.0f)
		eDynamicUITempDesc.fSizeY = 0.001f;
	ImGui::InputFloat("fX_Ratio", &eDynamicUITempDesc.fX);
	ImGui::InputFloat("fY_Ratio", &eDynamicUITempDesc.fY);

	// 필요하면 슬라이더로
	ImGui::SliderFloat("SizeX_Slider", &eDynamicUITempDesc.fSizeX, 0.001f, 1.5f);
	ImGui::SliderFloat("SizeY_Slider", &eDynamicUITempDesc.fSizeY, 0.001f, 1.5f);
	ImGui::SliderFloat("fX_Slider", &eDynamicUITempDesc.fX, -0.5f, 1.5f);
	ImGui::SliderFloat("fY_Slider", &eDynamicUITempDesc.fY, -0.5f, 1.5f);

	ImGui::SliderFloat("Rotation", &eDynamicUITempDesc.fRotation, 0.f, 360.f);

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

}

void CGLTool::Input_Sequence_Desc()
{
	// 입력 칸 만들기
	ImGui::Checkbox("IsLoop", &m_eFeatureDesc.isLoop);
	ImGui::InputInt("Type", &m_eFeatureDesc.iType);
	ImGui::InputInt("StartFrame", &m_eFeatureDesc.iStartFrame);
	ImGui::InputInt("EndFrame", &m_eFeatureDesc.iEndFrame);

	ImGui::InputFloat("StartAlpha", &m_eFeatureDesc.fStartAlpha);
	ImGui::InputFloat("EndAlpha", &m_eFeatureDesc.fEndAlpha);

	ImGui::InputFloat2("StartUV", reinterpret_cast<float*>(&m_eFeatureDesc.fStartUV));
	ImGui::InputFloat2("OffsetUV", reinterpret_cast<float*>(&m_eFeatureDesc.fOffsetUV));

	ImGui::InputFloat2("StartPos", reinterpret_cast<float*>(&m_eFeatureDesc.fStartPos));
	ImGui::InputFloat2("EndPos", reinterpret_cast<float*>(&m_eFeatureDesc.fEndPos));

	// 최소값 설정
	ImGui::InputFloat2("StartScale", reinterpret_cast<float*>(&m_eFeatureDesc.fStartScale));
	if (m_eFeatureDesc.fStartScale.x <= 0.0f)
		m_eFeatureDesc.fStartScale.x = 0.001f;
	if (m_eFeatureDesc.fStartScale.y <= 0.0f)
		m_eFeatureDesc.fStartScale.y = 0.001f;

	ImGui::InputFloat2("EndScale", reinterpret_cast<float*>(&m_eFeatureDesc.fEndScale));
	if (m_eFeatureDesc.fEndScale.x <= 0.0f)
		m_eFeatureDesc.fEndScale.x = 0.001f;
	if (m_eFeatureDesc.fEndScale.y <= 0.0f)
		m_eFeatureDesc.fEndScale.y = 0.001f;


	if (0 == m_eFeatureDesc.iType)
		m_eFeatureDesc.strTypeTag = "Fade";
	else if (1 == m_eFeatureDesc.iType)
		m_eFeatureDesc.strTypeTag = "UV";
	else if (2 == m_eFeatureDesc.iType)
		m_eFeatureDesc.strTypeTag = "Pos";
	else if (3 == m_eFeatureDesc.iType)
		m_eFeatureDesc.strTypeTag = "Scale";

}

void CGLTool::Input_Text()
{
	size_t buf_size = m_strInput.size() + 512;
	char* buf = new char[buf_size];
	memcpy(buf, m_strInput.c_str(), m_strInput.size() + 1);

	if (ImGui::InputTextMultiline("##multiline", buf, buf_size, ImVec2(-FLT_MIN, 200)))
	{
		m_strInput = buf;
	}

	delete[] buf;

	// 만들때 필요한거 정리해서 넣자

	ImGui::Checkbox("IsCenter", &m_eTextTempUIDesc.isCenter);
	ImGui::InputFloat4("Color", reinterpret_cast<float*>(&m_eTextTempUIDesc.vColor));
	
	ImGui::InputFloat("Offset", &m_eTextTempUIDesc.fOffset);
	ImGui::InputFloat2("FontOffset", reinterpret_cast<float*>( &m_eTextTempUIDesc.fFontOffset));
	ImGui::InputFloat("FontScale", &m_eTextTempUIDesc.fFontScale);
	
	ImGui::InputFloat("fX_Ratio", &m_eTextTempUIDesc.fX);
	ImGui::InputFloat("fY_Ratio", &m_eTextTempUIDesc.fY);
	ImGui::SliderFloat("fX_Slider", &m_eTextTempUIDesc.fX, -0.5f, 1.5f);
	ImGui::SliderFloat("fY_Slider", &m_eTextTempUIDesc.fY, -0.5f, 1.5f);

	ImGui::SliderFloat("Rotation", &m_eTextTempUIDesc.fRotation, 0.f, 360.f);

	m_eTextUIDesc = m_eTextTempUIDesc;
	m_eTextUIDesc.fX *= g_iWinSizeX;
	m_eTextUIDesc.fY *= g_iWinSizeY;

	
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

			Input_Static_Desc();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Input Dynamic Desc"))
		{
			Input_Dynamic_Desc();
			
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Input Sequence Desc"))
		{
			Input_Sequence_Desc();
			
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(u8"텍스트 입력하기"))
		{
			Input_Text();

			if (ImGui::Button(u8"선택된 텍스트 바꾸기"))
			{
				m_eTextTempUIDesc.strCaption = StringToWStringU8(m_strInput);
				m_eTextUIDesc.strCaption = StringToWStringU8(m_strInput);

			
			}

			if (nullptr == m_pSelectTxtObj || m_pSelectTxtObj->Get_bDead())
			{

			}
			else
			{
				m_pSelectTxtObj->Update_UI_From_Tool(m_eTextUIDesc);
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Button"))
		{

			IGFD::FileDialogConfig config;
			if (Button(u8"Save File"))
			{
				Obj_Serialize();

				//Save_File();
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

			if (Button(u8"Add UI Text"))
			{
				Add_UI_Text();
			}

			if (Button(u8"Delete Static UI"))
			{
				if (nullptr != m_pSelectStaticObj)
					m_pSelectStaticObj->Set_bDead();

				int currentIndex = 0;
				for (auto& pObj : m_StaticUIList)
				{
					if (m_iSelectObjIndex == currentIndex)
					{
						pObj = nullptr;
						break;
					}

					++currentIndex;
				}

				m_pSelectStaticObj = nullptr;
				m_iSelectObjIndex = -1;
			}

			if (Button(u8"Delete Dynamic UI"))
			{
				if (nullptr != m_pSelectDynamicObj)
					m_pSelectDynamicObj->Set_bDead();

				int currentIndex = 0;
				for (auto& pObj : m_DynamicUIList)
				{
					if (m_iDynamicObjIndex == currentIndex)
					{
						pObj = nullptr;
						break;
					}

					++currentIndex;
				}

				m_pSelectDynamicObj = nullptr;
				m_iDynamicObjIndex = -1;
			}
			if (Button(u8"Delete Text UI"))
			{
				if (nullptr != m_pSelectTxtObj)
					m_pSelectTxtObj->Set_bDead();

				int currentIndex = 0;
				for (auto& pObj : m_TextUIList)
				{
					if (m_iTextObjIndex == currentIndex)
					{
						pObj = nullptr;
						break;
					}
						

					++currentIndex;
				}

				m_pSelectTxtObj = nullptr;
				m_iTextObjIndex = -1;
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

					Obj_Deserialize();

					/*if (fileName.find("Static") != fileName.npos)
					{
						Add_Static_UI_From_File();
					}
					else if (fileName.find("Dynamic") != fileName.npos)
					{
						Add_Dynamic_UI_From_File();
					}*/

					

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
						eDynamicUITempDesc = m_pSelectDynamicObj->Get_Desc();
						eDynamicUITempDesc.fSizeX /= g_iWinSizeX;
						eDynamicUITempDesc.fSizeY /= g_iWinSizeY;
						eDynamicUITempDesc.fX /= g_iWinSizeX;
						eDynamicUITempDesc.fY /= g_iWinSizeY;
						eDynamicUITempDesc.strTextureTag = pObj->Get_StrTextureTag();

					
					}


					++index;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Text"))
			{
				int index = 0;

				for (const auto& pObj : m_TextUIList)
				{
					bool isSelected = (index == m_iTextObjIndex);

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					if (ImGui::Selectable(to_string(index).c_str(), isSelected))
					{
						m_iTextObjIndex = index;
						m_pSelectTxtObj = pObj;
						m_eTextTempUIDesc = m_pSelectTxtObj->Get_Desc();
						m_eTextTempUIDesc.fX /= g_iWinSizeX;
						m_eTextTempUIDesc.fY /= g_iWinSizeY;

						m_strInput = WStringToStringU8(m_eTextTempUIDesc.strCaption);
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
