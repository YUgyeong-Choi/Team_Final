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


	for (auto& pair : *map)
	{
		if (pair.first.find(L"GameObject") != pair.first.npos)
		{
			if(pair.first.find(L"UI") != pair.first.npos)
				m_ProtoNames.push_back(pair.first);
		}
			
	}


	m_pSequence = new CUI_Sequence();

	m_pMergeContainer = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), nullptr));



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

	// 시퀀스 업데이트 추가
	for (auto& container : m_ContainerList)
	{
		if (nullptr != container)
		{
			for (auto& part : container->Get_PartUI())
			{
				if (auto pDynamicUI = dynamic_cast<CDynamic_UI*>(part))
				{
					pDynamicUI->Update_UI_From_Frame(m_iCurrentFrame);
				}
			}
		}
	
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


	for (auto& pContainer : m_ContainerList)
	{
		if (nullptr != pContainer)
			JsonArray.push_back(pContainer->Serialize());
	}


	ofstream file("../Bin/Save/UI/Temp.json");

	file << JsonArray.dump(4);

	file.close();

}

void CGLTool::Obj_Deserialize()
{
	string filePath = IFILEDIALOG->GetFilePathName();

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		ENUM_CLASS(LEVEL::GL), TEXT("Layer_Container"), nullptr)))
		return ;

	m_ContainerList.push_back(static_cast<CUI_Container*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::GL), TEXT("Layer_Container"))));

	json j;

	ifstream file(filePath);

	file >> j;

	if (j.is_array())
	{

		if (j.front().contains("Parts"))
		{
			for (const auto& containerJson : j)
			{

				if (containerJson.is_object())
					(m_ContainerList.back())->Deserialize(containerJson);

			}
		}

		else if (j.front().contains("ProtoTag"))
		{
			for (const auto& objJson : j)
			{
				if (!objJson.contains("ProtoTag")) continue;

				string protoTag = objJson["ProtoTag"];
				m_ContainerList.back()->Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), nullptr);


				auto& partList = m_ContainerList.back()->Get_PartUI();
				if (!partList.empty())
				{
					partList.back()->Deserialize(objJson);
					partList.back()->Update_Data();
				}


			}
		}

		
		
	}
	
	
}



void CGLTool::Open_File()
{
	
	IGFD::FileDialogConfig config;

	m_strSavePath = R"(../Bin/Save/UI)";
	config.path = m_strSavePath;
	config.countSelectionMax = 0; // 무제한
	

	IFILEDIALOG->OpenDialog("JsonDialog", "Select json File", ".json", config);

	
}

void CGLTool::Add_Container()
{
	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Container"), nullptr)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Container"));

	if (nullptr == pObj)
		return;

	m_ContainerList.push_back(static_cast<CUI_Container*>(pObj));

}

void CGLTool::Add_UI_Select_Prototype()
{
	if (nullptr == m_pContainerObj)
		return;

	if (m_strSelectProtoName.empty())
		return;

	m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), m_strSelectProtoName, static_cast<_uint>(LEVEL::GL), TEXT("Layer_Temp"), nullptr);

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Temp"));
	if (nullptr == pObj)
		return;


	static_cast<CUIObject*>(pObj)->Ready_Components_File(m_strSelectTextureName);


	m_pContainerObj->Add_UI_From_Tool(static_cast<CUIObject*>(pObj));

}

void CGLTool::Save_Container()
{
	if (nullptr == m_pContainerObj)
		return;

	json J;

	J.push_back(m_pContainerObj->Serialize());

	ofstream file("../Bin/Save/UI/Temp_Container.json");

	file << J.dump(4);

	file.close();
}

void CGLTool::Upload_Merge_Container()
{
	if (nullptr == m_pSelectConatinerPart)
		return;

	m_pMergeContainer->Get_PartUI().push_back(m_pSelectConatinerPart);


}

void CGLTool::Download_Merge_Container()
{
	for (auto& part : m_pMergeContainer->Get_PartUI())
	{
		if (nullptr != part)
		{
			m_pContainerObj->Get_PartUI().push_back(part);
			Safe_AddRef(part);
		}
		
	}

}

void CGLTool::Clear_Merge_Container()
{
	m_pMergeContainer->Get_PartUI().clear();
}

void CGLTool::Delete_Merge_Container_Select_Obj()
{
	if (-1 == m_iMergeContainerPartIndex)
		return;

	_int iIndex = { 0 };

	for (auto& part : m_pMergeContainer->Get_PartUI())
	{
		bool isSelected = (iIndex == m_iSelectObjIndex);

		if (isSelected)
		{
			part = nullptr;
			break;
		}
	}

	m_iMergeContainerPartIndex = -1;

}

void CGLTool::Set_Container_Active()
{
	if (nullptr == m_pContainerObj)
		return;
	m_pContainerObj->Active_Update(m_isActive);
}

_bool CGLTool::Check_Dynamic_UI()
{
	if (nullptr == m_pSelectConatinerPart)
		return false;

	if (L"Prototype_GameObject_Static_UI" == m_pSelectConatinerPart->Get_ProtoTag())
		return false;

	if (L"Prototype_GameObject_UI_Text" == m_pSelectConatinerPart->Get_ProtoTag())
		return false;


	return true;
}

void CGLTool::Delete_Container()
{
}

void CGLTool::Delete_SelectPart()
{
}

void CGLTool::Add_Sequence_To_DynamicUI()
{
	if (!Check_Dynamic_UI())
		return;


	m_pSequence->Clear();

	for (auto& pFeature : static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Get_Features())
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
	if (!Check_Dynamic_UI())
		return;

	for (auto& pFeature : static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Get_Features())
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

			static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(fadeDesc.strProtoTag), &fadeDesc);
			
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

			static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(uvDesc.strProtoTag), &uvDesc);
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

			static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(posDesc.strProtoTag), &posDesc);
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
			

			static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(scaleDesc.strProtoTag), &scaleDesc);
		}
		else if (4 == eDesc.iType)
		{
			// fade
			UI_FEATURE_ROTATION_DESC fadeDesc = {};
			fadeDesc.strProtoTag = "Prototype_Component_UI_Feature_Rotation";
			fadeDesc.isLoop = eDesc.isLoop;
			fadeDesc.iStartFrame = eDesc.iStartFrame;
			fadeDesc.iEndFrame = eDesc.iEndFrame;
			fadeDesc.fStartRotation = eDesc.fStartAlpha;
			fadeDesc.fEndRotation = eDesc.fEndAlpha;
			fadeDesc.fRotationPos = eDesc.fStartPos;
			fadeDesc.fInitPos = eDesc.fEndPos;

			static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(fadeDesc.strProtoTag), &fadeDesc);

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

	eStaticUIDesc.strTextureTag = m_strSelectTextureName;

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

	//

	if (nullptr == m_pSelectConatinerPart || m_pSelectConatinerPart->Get_bDead())
	{

	}
	else
	{
		(m_pSelectConatinerPart)->Update_UI_From_Tool(&eStaticUIDesc);
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

	if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Text") != m_pSelectConatinerPart->Get_ProtoTag().npos)
	{
		size_t buf_size = m_strInput.size() + 512;
		char* buf = new char[buf_size];
		memcpy(buf, m_strInput.c_str(), m_strInput.size() + 1);

		if (ImGui::InputTextMultiline("##multiline", buf, buf_size, ImVec2(-FLT_MIN, 200)))
		{
			m_strInput = buf;
		}

		delete[] buf;

		eDynamicUITempDesc.strText = m_strInput;

		_int iAlignType = ENUM_CLASS(eDynamicUITempDesc.iAlignType);
		ImGui::InputInt("IsCenter", &iAlignType);
		eDynamicUITempDesc.iAlignType = (iAlignType);

	}

	eDynamicUIDesc = eDynamicUITempDesc;
	eDynamicUIDesc.fSizeX *= g_iWinSizeX;
	eDynamicUIDesc.fSizeY *= g_iWinSizeY;
	eDynamicUIDesc.fX *= g_iWinSizeX;
	eDynamicUIDesc.fY *= g_iWinSizeY;

	eDynamicUIDesc.strTextureTag = m_strSelectTextureName;

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

	if (nullptr == m_pSelectConatinerPart || m_pSelectConatinerPart->Get_bDead())
	{

	}
	else
	{
		(m_pSelectConatinerPart)->Update_UI_From_Tool(&eDynamicUIDesc);
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
	else if(4 == m_eFeatureDesc.iType)
		m_eFeatureDesc.strTypeTag = "Rotation";
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

	_int iAlignType = ENUM_CLASS(m_eTextTempUIDesc.eAlign);

	ImGui::InputInt("IsCenter", &iAlignType);
	ImGui::InputFloat4("Color", reinterpret_cast<float*>(&m_eTextTempUIDesc.vColor));
	
	ImGui::InputFloat("Offset", &m_eTextTempUIDesc.fOffset);
	ImGui::InputFloat2("FontOffset", reinterpret_cast<float*>( &m_eTextTempUIDesc.fFontOffset));
	ImGui::InputFloat("FontScale", &m_eTextTempUIDesc.fFontScale);
	
	ImGui::InputFloat("fX_Ratio", &m_eTextTempUIDesc.fX);
	ImGui::InputFloat("fY_Ratio", &m_eTextTempUIDesc.fY);
	ImGui::SliderFloat("fX_Slider", &m_eTextTempUIDesc.fX, -0.5f, 1.5f);
	ImGui::SliderFloat("fY_Slider", &m_eTextTempUIDesc.fY, -0.5f, 1.5f);

	ImGui::SliderFloat("Rotation", &m_eTextTempUIDesc.fRotation, 0.f, 360.f);


	m_eTextTempUIDesc.eAlign = TEXTALIGN(iAlignType);

	m_eTextUIDesc = m_eTextTempUIDesc;
	m_eTextUIDesc.fX *= g_iWinSizeX;
	m_eTextUIDesc.fY *= g_iWinSizeY;

	if (nullptr == m_pSelectConatinerPart || m_pSelectConatinerPart->Get_bDead())
	{

	}
	else
	{
		(m_pSelectConatinerPart)->Update_UI_From_Tool(&m_eTextUIDesc);
	}
}

void CGLTool::Input_Button_Desc()
{


	// 입력 칸 만들기
	ImGui::InputFloat4("Color", reinterpret_cast<float*>(&eButtonUITempDesc.vColor));
	ImGui::InputFloat("Duration", &eButtonUITempDesc.fDuration);
	ImGui::InputFloat("Offset", &eButtonUITempDesc.fOffset);
	ImGui::InputInt("PassIndex", &eButtonUITempDesc.iPassIndex);
	ImGui::InputInt("TextureIndex", &eButtonUITempDesc.iTextureIndex);
	ImGui::InputFloat("SizeX_Ratio", &eButtonUITempDesc.fSizeX);
	if (eButtonUITempDesc.fSizeX <= 0.0f)
		eButtonUITempDesc.fSizeX = 0.001f;
	ImGui::InputFloat("SizeY_Ratio", &eButtonUITempDesc.fSizeY);
	if (eButtonUITempDesc.fSizeY <= 0.0f)
		eButtonUITempDesc.fSizeY = 0.001f;
	ImGui::InputFloat("fX_Ratio", &eButtonUITempDesc.fX);
	ImGui::InputFloat("fY_Ratio", &eButtonUITempDesc.fY);

	// 필요하면 슬라이더로
	ImGui::SliderFloat("SizeX_Slider", &eButtonUITempDesc.fSizeX, 0.001f, 1.5f);
	ImGui::SliderFloat("SizeY_Slider", &eButtonUITempDesc.fSizeY, 0.001f, 1.5f);
	ImGui::SliderFloat("fX_Slider", &eButtonUITempDesc.fX, -0.5f, 1.5f);
	ImGui::SliderFloat("fY_Slider", &eButtonUITempDesc.fY, -0.5f, 1.5f);

	ImGui::SliderFloat("Rotation", &eButtonUITempDesc.fRotation, 0.f, 360.f);

	ImGui::InputFloat2("Padding", reinterpret_cast<float*>(&eButtonUITempDesc.fPadding));
	ImGui::InputFloat("FontSize", &eButtonUITempDesc.fFontSize);


	eButtonUIDesc = eButtonUITempDesc;
	eButtonUIDesc.fSizeX *= g_iWinSizeX;
	eButtonUIDesc.fSizeY *= g_iWinSizeY;
	eButtonUIDesc.fX *= g_iWinSizeX;
	eButtonUIDesc.fY *= g_iWinSizeY;

	if (eButtonUIDesc.iPassIndex >= D_UI_END)
	{
		eButtonUIDesc.iPassIndex = D_UI_END - 1;
		eButtonUITempDesc.iPassIndex = D_UI_END - 1;
	}

	if (eButtonUIDesc.iPassIndex < 0)
	{
		eButtonUIDesc.iPassIndex = 0;
		eButtonUITempDesc.iPassIndex = 0;
	}

	size_t buf_size = m_strCaption.size() + 512;
	char* buf = new char[buf_size];
	memcpy(buf, m_strCaption.c_str(), m_strCaption.size() + 1);

	if (ImGui::InputTextMultiline("##multiline", buf, buf_size, ImVec2(-FLT_MIN, 200)))
	{
		m_strCaption = buf;
	}

	delete[] buf;

	eButtonUITempDesc.strCaption = StringToWStringU8(m_strCaption);
	eButtonUIDesc.strCaption = StringToWStringU8(m_strCaption);

	if (nullptr == m_pSelectConatinerPart || m_pSelectConatinerPart->Get_bDead())
	{

	}
	else
	{
		(m_pSelectConatinerPart)->Update_UI_From_Tool(&eButtonUIDesc);
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
		//
		if (nullptr != m_pSelectConatinerPart)
		{

			if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Prototype_GameObject_Static_UI") != m_pSelectConatinerPart->Get_ProtoTag().npos)
			{
				if (ImGui::BeginTabItem("Input Static Desc"))
				{
					// 입력 칸 만들기

					Input_Static_Desc();

					ImGui::EndTabItem();
				}
			}
			else if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Prototype_GameObject_UI_Text") != m_pSelectConatinerPart->Get_ProtoTag().npos)
			{
				if (ImGui::BeginTabItem(u8"텍스트 입력하기"))
				{
					Input_Text();

					if (ImGui::Button(u8"선택된 텍스트 바꾸기"))
					{
						m_eTextTempUIDesc.strCaption = StringToWStringU8(m_strInput);
						m_eTextUIDesc.strCaption = StringToWStringU8(m_strInput);


					}

					ImGui::EndTabItem();
				}
			}

			else if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Prototype_GameObject_UI_Button") != m_pSelectConatinerPart->Get_ProtoTag().npos)
			{
				if (ImGui::BeginTabItem("Input Button Desc"))
				{
					// 입력 칸 만들기

					Input_Button_Desc();

					ImGui::EndTabItem();
				}
			}
			
			else
			{
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

				
			}

			

			

			
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

			if (Button(u8"Add Container"))
			{
				Add_Container();

			}



			if (Button(u8"Save Container"))
			{

				Save_Container();
			}

			if (Button(u8"Add Select ProtoUI"))
			{
				Add_UI_Select_Prototype();

			}

			if (Button(u8"Delete Select UI"))
			{

				if (nullptr != m_pSelectConatinerPart)
					m_pSelectConatinerPart->Set_bDead();

				int index = 0;

				for (auto& pObj : m_pContainerObj->Get_PartUI())
				{
					if (pObj == nullptr)
						continue;

					bool isSelected = (index == m_iSelectObjIndex);

					if (isSelected)
					{
						pObj = nullptr;
						break;
					}
						

					++index;
				}

				m_pSelectConatinerPart = nullptr;
				m_iSelectObjIndex = -1;

			

	
			}
			
			if (Button(u8"Delete Container"))
			{
				if (nullptr != m_pContainerObj)
					m_pContainerObj->Set_bDead();

				

				int index = 0;

				for (auto& pObj : m_ContainerList)
				{
					if (pObj == nullptr)
						continue;

					bool isSelected = (index == m_iSelectContainerIndex);

					if (isSelected)
					{
						for (auto pPart : pObj->Get_PartUI())
							pPart->Set_bDead();

						pObj = nullptr;
					}
					

					++index;
				}

				m_pContainerObj = nullptr;
				m_iSelectContainerIndex = -1;


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

			/*	for (const auto& pObj : m_DynamicUIList)
				{

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					pObj->Reset();
				}*/
			}

			if (Button(u8"Upload Merge Container"))
			{
				Upload_Merge_Container();
			}

			if (Button(u8"Download Merge Container"))
			{
				Download_Merge_Container();
			}

			if (Button(u8"Clear Merge Container"))
			{
				Clear_Merge_Container();
			}

			if (Button(u8"Delete Select UI Merge Container"))
			{
				Delete_Merge_Container_Select_Obj();
			}


			// 파일 탐색기를 띄운다
			ImGui::SetNextWindowSize(ImVec2(800, 600));
			if (IFILEDIALOG->Display("JsonDialog"))
			{
				if (IFILEDIALOG->IsOk())
				{
					string fileName = IFILEDIALOG->GetCurrentFileName();

					Obj_Deserialize();

					

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


			if (ImGui::BeginTabItem("Container"))
			{


				int index = 0;

				for (const auto& pObj : m_ContainerList)
				{
					if (pObj == nullptr)
						continue;

					bool isSelected = (index == m_iSelectContainerIndex);

					if (ImGui::Selectable(to_string(index).c_str(), isSelected))
					{
						m_iSelectContainerIndex = index;
						m_pContainerObj = pObj;
					}
					++index;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("ContainerParts"))
			{
			

				if (nullptr != m_pContainerObj)
				{
					int index = 0;

					
					auto& containerPart = m_pContainerObj->Get_PartUI();

					for (auto& part : containerPart)
					{
						if (nullptr == part)
							continue;

						bool isSelected = (index == m_iSelectObjIndex);

						if (ImGui::Selectable(to_string(index).c_str(), isSelected))
						{
							m_iSelectObjIndex = index;
							m_pSelectConatinerPart = part;

							//
							if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Prototype_GameObject_Static_UI") != m_pSelectConatinerPart->Get_ProtoTag().npos)
							{
								eStaticUIDesc = static_cast<CStatic_UI*>(m_pSelectConatinerPart)->Get_Desc();

								eStaticUITempDesc = eStaticUIDesc;

								eStaticUITempDesc.fSizeX /= g_iWinSizeX;
								eStaticUITempDesc.fSizeY /= g_iWinSizeY;
								eStaticUITempDesc.fX /= g_iWinSizeX;
								eStaticUITempDesc.fY /= g_iWinSizeY;

								
							}

							//
							else if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Prototype_GameObject_UI_Text") != m_pSelectConatinerPart->Get_ProtoTag().npos)
							{
								m_eTextUIDesc = static_cast<CUI_Text*>(m_pSelectConatinerPart)->Get_Desc();

								m_eTextTempUIDesc = m_eTextUIDesc;

								m_eTextTempUIDesc.fSizeX /= g_iWinSizeX;
								m_eTextTempUIDesc.fSizeY /= g_iWinSizeY;
								m_eTextTempUIDesc.fX /= g_iWinSizeX;
								m_eTextTempUIDesc.fY /= g_iWinSizeY;

								m_strInput = WStringToStringU8(m_eTextTempUIDesc.strCaption);
							}

							else if (m_pSelectConatinerPart->Get_ProtoTag().find(L"Prototype_GameObject_UI_Button") != m_pSelectConatinerPart->Get_ProtoTag().npos)
							{
								eButtonUIDesc = static_cast<CUI_Button*>(m_pSelectConatinerPart)->Get_Desc();

								eButtonUITempDesc = eButtonUIDesc;

								eButtonUITempDesc.fSizeX /= g_iWinSizeX;
								eButtonUITempDesc.fSizeY /= g_iWinSizeY;
								eButtonUITempDesc.fX /= g_iWinSizeX;
								eButtonUITempDesc.fY /= g_iWinSizeY;

								m_strCaption = WStringToStringU8(eButtonUITempDesc.strCaption);
							}

							else
							{
								eDynamicUIDesc = static_cast<CDynamic_UI*>(m_pSelectConatinerPart)->Get_Desc();

								eDynamicUITempDesc = eDynamicUIDesc;

								eDynamicUITempDesc.fSizeX /= g_iWinSizeX;
								eDynamicUITempDesc.fSizeY /= g_iWinSizeY;
								eDynamicUITempDesc.fX /= g_iWinSizeX;
								eDynamicUITempDesc.fY /= g_iWinSizeY;
							}

							//

						}
						++index;
					}
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Merge Container Parts"))
			{


				if (nullptr != m_pMergeContainer)
				{
					int index = 0;


					auto& containerPart = m_pMergeContainer->Get_PartUI();

					for (auto& part : containerPart)
					{
						if (nullptr == part)
							continue;

						bool isSelected = (index == m_iMergeContainerPartIndex);

						if (ImGui::Selectable(to_string(index).c_str(), isSelected))
						{
							m_iMergeContainerPartIndex = index;

							//
							
						}
						++index;
					}
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
						m_strSelectTextureName = strName;
					}
					++index;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Prototype"))
			{


				int index = 0;

				for (const auto& strName : m_ProtoNames)
				{
					bool isSelected = (index == m_iSelectProtoIndex);

					if (ImGui::Selectable(WStringToString(strName).c_str(), isSelected))
					{
						m_iSelectProtoIndex = index;
						m_strSelectProtoName = strName;
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

	Safe_Release(m_pMergeContainer);
}
