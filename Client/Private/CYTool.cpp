#include "CYTool.h"
#include "GameInstance.h"
#include "EffectSequence.h"
#include "ToolParticle.h"
#include "ToolSprite.h"
#include "ToolTrail.h"
#include "ToolMeshEffect.h"
#include "Client_Calculation.h"
#include "Player.h"
#include "Weapon.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��

CCYTool::CCYTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CCYTool::CCYTool(const CCYTool& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CCYTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCYTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pSequence = new CEffectSequence();

	Load_Textures();

	return S_OK;
}

void CCYTool::Priority_Update(_float fTimeDelta)
{
	Key_Input();
	if (m_bPlaySequence)
	{
		m_fCurFrame += m_fTickPerSecond * fTimeDelta;
		m_iCurFrame = static_cast<_int>(m_fCurFrame);
	}
	else
	{
		m_fCurFrame = static_cast<_float>(m_iCurFrame);
	}
	if (m_fCurFrame > m_pSequence->GetFrameMax())
	{
		if (m_pSequence->m_bLoop == true)
		{
			m_fCurFrame = static_cast<_float>(m_pSequence->GetFrameMin());
			for (auto& pItem : m_pSequence->m_Items)
			{
				pItem.pEffect->Reset_TrackPosition();
			}
		}
		else
		{
			m_fCurFrame = static_cast<_float>(m_pSequence->GetFrameMax());
			m_bPlaySequence = false;
		}
	}
	for (auto& pItem : m_pSequence->m_Items)
	{
		if (m_iCurFrame >= *pItem.iStart && m_iCurFrame <= *pItem.iEnd)
		{
			pItem.pEffect->Priority_Update(fTimeDelta);
			// ����Ʈ�� ���
			//PlaySpriteEffect(pItem.iType, m_iCurFrame - pItem.iStart);
		}
	}
}

void CCYTool::Update(_float fTimeDelta)
{
	//if (m_bPlaySequence)
	{
		for (auto& pItem : m_pSequence->m_Items)
		{
			if (m_iCurFrame >= *pItem.iStart && m_fCurFrame <= *pItem.iEnd)
			{
				if (m_bUpdateRuntimeMode)
					pItem.pEffect->Update(fTimeDelta);
				else
					pItem.pEffect->Update_Tool(fTimeDelta, static_cast<_float>(m_iCurFrame - *pItem.iStart));
				// ����Ʈ�� ���
				//PlaySpriteEffect(pItem.iType, m_fCurFrame - pItem.iStart);
			}
		}
	}
}

void CCYTool::Late_Update(_float fTimeDelta)
{
	for (auto& pItem : m_pSequence->m_Items)
	{
		if (m_iCurFrame >= *pItem.iStart && m_iCurFrame <= *pItem.iEnd)
		{
			pItem.pEffect->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CCYTool::Render()
{
	//if (FAILED(Render_EffectTool()))
	//	return E_FAIL;
	//ImGui::ShowDemoWindow(); // Show demo window! :)
	ImGuizmo::BeginFrame();
	if (FAILED(SequenceWindow()))
		return E_FAIL;

	if (FAILED(Edit_Preferences()))
		return E_FAIL;

	if (m_bOpenSaveEffectOnly)
		Save_Effect();
	if (m_bOpenLoadEffectOnly)
		Load_Effect();

	if (m_bOpenSaveEffectContainer)
		Save_EffectSet();
	if (m_bOpenLoadEffectContainer)
		Load_EffectSet();


	return S_OK;
}

HRESULT CCYTool::Render_EffectTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("CY Tools", &open, NULL);

	IGFD::FileDialogConfig config;
	if (Button(u8"�ȳ�"))
	{
		IFILEDIALOG->OpenDialog("Effect Tool Dialog", "Select File", nullptr, config);
	}

	if (IFILEDIALOG->Display("Effect Tool Dialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// ó��
			// first: ���ϸ�.Ȯ����
			// second: ��ü ��� (���ϸ�����)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{

				}
			}
		}
		IFILEDIALOG->Close();
	}

	ImGui::End();
	return S_OK;
}

HRESULT CCYTool::SequenceWindow()
{
	ImGui::Begin("ImSequence Window");
	if (ImGui::RadioButton("Sprite Effect", m_eEffectType == EFF_SPRITE)) {
		m_eEffectType = EFF_SPRITE;
		m_strSeqItemName = "Sprite";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 200, 60, 40);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Particle Effect", m_eEffectType == EFF_PARTICLE)) {
		m_eEffectType = EFF_PARTICLE;
		m_strSeqItemName = "Particle";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 60, 200, 80);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Mesh Effect", m_eEffectType == EFF_MESH)) {
		m_eEffectType = EFF_MESH;
		m_strSeqItemName = "Mesh";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 100, 100, 220);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Trail Effect", m_eEffectType == EFF_TRAIL)) {
		m_eEffectType = EFF_TRAIL;
		m_strSeqItemName = "Trail";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 170, 80, 250);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add to Sequence"))
	{
		CEffectBase* pInstance = { nullptr };
		switch (m_eEffectType)
		{
		case Client::EFF_SPRITE:
		{
			CToolSprite::DESC desc = {};
			desc.bAnimation = true;
			desc.fRotationPerSec = 0.f;
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 8;
			desc.iTileY = 8;
			desc.iShaderPass = ENUM_CLASS(SE_UVSPRITE_COLOR);
			desc.bTool = true;
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
		}
			break;
		case Client::EFF_PARTICLE:
		{
			CToolParticle::DESC desc = {};
			desc.fRotationPerSec = XMConvertToRadians(90.f);
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 1;
			desc.iTileY = 1;
			desc.ePType = m_eParticleType;
			desc.iNumInstance = m_iNumInstance;
			desc.isLoop = true;
			desc.vCenter = _float3(0.f,0.f,0.f);
			desc.vLifeTime = m_vLifeTime;
			desc.vPivot = _float3(0.f, 0.f, 0.f);
			desc.vRange = m_vRange;
			desc.vSize = m_vSize;
			desc.vSpeed = m_vSpeed;
			desc.bBillboard = false;
			desc.bTool = true;
			desc.iShaderPass = ENUM_CLASS(PE_WB);
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"), &desc));
		}
			break;
		case Client::EFF_MESH:
		{
			CToolMeshEffect::DESC desc = {};
			desc.bAnimation = true;
			desc.fRotationPerSec = 0.f;
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 4;
			desc.iTileY = 1;
			desc.bBillboard = false;
			desc.bTool = true;
			desc.iShaderPass = ENUM_CLASS(ME_MASKONLY);
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"), &desc));
		}
			break;
		case Client::EFF_TRAIL:
		{
			if (GET_PLAYER(ENUM_CLASS(LEVEL::CY)) == nullptr)
				break;
			//CGameObject* TestWeapon = m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Player_Weapon"), 0);
			//CModel* pWeaponModelCom = static_cast<CModel*>(TestWeapon->Get_Component(TEXT("Com_Model")));
			//_uint iInnerBoneIdx = pWeaponModelCom->Find_BoneIndex("BN_Blade_B");
			//_uint iOuterBoneIdx = pWeaponModelCom->Find_BoneIndex("BN_Blade");
			//
			//CToolTrail::DESC desc = {};
			//desc.bAnimation = true;
			//desc.fRotationPerSec = 0.f;
			//desc.fSpeedPerSec = 5.f;
			//desc.iTileX = 4;
			//desc.iTileY = 1;
			//desc.bBillboard = false;
			//desc.bTool = true;
			//desc.iShaderPass = ENUM_CLASS(TE_DEFAULT);
			//desc.pInnerSocketMatrix = const_cast<_float4x4*>(pWeaponModelCom->Get_CombinedTransformationMatrix(iInnerBoneIdx));
			//desc.pOuterSocketMatrix = const_cast<_float4x4*>(pWeaponModelCom->Get_CombinedTransformationMatrix(iOuterBoneIdx));
			//desc.pParentCombinedMatrix = const_cast<_float4x4*>(static_cast<CWeapon*>(TestWeapon)->Get_CombinedWorldMatrix());

			/******************************************/

			CGameObject* TestWeapon = m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::CY), TEXT("Player_Weapon"), 0);
			if (TestWeapon == nullptr)
			{
				MSG_BOX("���� ���� ��ã�ھ�� ");
				ImGui::End();
				return E_FAIL;
			}
			
			CModel* pWeaponModelCom = static_cast<CModel*>(TestWeapon->Get_Component(TEXT("Com_Model")));
			_uint iInnerBoneIdx = pWeaponModelCom->Find_BoneIndex("BN_Blade_B");
			_uint iOuterBoneIdx = pWeaponModelCom->Find_BoneIndex("BN_Blade_End");

			CToolTrail::DESC desc = {};
			desc.bAnimation = true;
			desc.fRotationPerSec = 0.f;
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 1;
			desc.iTileY = 1;
			desc.bBillboard = false;
			desc.bTool = true;
			desc.iShaderPass = ENUM_CLASS(TE_DEFAULT);
			desc.pInnerSocketMatrix = const_cast<_float4x4*>(pWeaponModelCom->Get_CombinedTransformationMatrix(iInnerBoneIdx));
			desc.pOuterSocketMatrix = const_cast<_float4x4*>(pWeaponModelCom->Get_CombinedTransformationMatrix(iOuterBoneIdx));
			desc.pParentCombinedMatrix = const_cast<_float4x4*>(static_cast<CWeapon*>(TestWeapon)->Get_CombinedWorldMatrix());

			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolTrailEffect"), &desc));
		}
			break;
		}
		if (pInstance != nullptr)
			m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save EffectContainer")){
		m_bOpenSaveEffectContainer = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load EffectContainer")){
		m_bOpenLoadEffectContainer = true;
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::DragInt("MaxFrame", &m_pSequence->m_iFrameMax, 1.0f, 10, 1000, "%d");
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_pSequence->m_bLoop);
	ImGui::SameLine();
	ImGui::Checkbox("Update_RuntimeMode", &m_bUpdateRuntimeMode);

	ImSequencer::Sequencer(
		m_pSequence,
		&m_iCurFrame,
		&m_bExpanded,
		&m_iSelected,
		&m_iFirstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL);

	ImGui::End();
	return S_OK;
}

HRESULT CCYTool::Edit_Preferences()
{
	ImGui::Begin("Edit Item Preferences");
	if (ImGui::Button("Load Effect"))
	{
		m_bOpenLoadEffectOnly = true;
	}
	ImGui::SameLine();
	if (m_pSequence == nullptr || m_pSequence->m_Items.empty() || m_iSelected == -1)
	{
		ImGui::End();
		return S_OK;
	}
	auto pEffect = m_pSequence->m_Items[m_iSelected].pEffect;
	
	if (ImGui::Button("Save Selected Effect"))
	{
		m_bOpenSaveEffectOnly = true;
	}

	// ������ �̸� �����ϴ� �뵵�θ� ���� ��������� �� �־��� ..
	static _char szNameBuffer[128] = {};
	ImGui::InputText("Effect Name", szNameBuffer, IM_ARRAYSIZE(szNameBuffer));

	// ��ư�� ������ �� �̸� �ݿ�
	if (ImGui::Button("Apply Name"))
		m_pSequence->m_Items[m_iSelected].strName = szNameBuffer;

	ImGui::Text("StartTrackPos: %d", *pEffect->Get_StartTrackPosition_Ptr());
	ImGui::Text("EndTrackPos: %d", *pEffect->Get_EndTrackPosition_Ptr());
	ImGui::Text("Duration: %d", *pEffect->Get_Duration_Ptr());
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::Checkbox("Billboard", pEffect->Get_Billboard_Ptr());
	ImGui::Checkbox("Animation", pEffect->Get_Animation_Ptr());
	ImGui::Checkbox("Flip UV", pEffect->Get_FlipUV_Ptr());

	ImGui::PushItemWidth(100);
	ImGui::Text("Tile X"); ImGui::SameLine();
	ImGui::InputInt("##Tile X", pEffect->Get_TileX()); ImGui::SameLine();
	ImGui::Text("Tile Y"); ImGui::SameLine();
	ImGui::InputInt("##Tile Y", pEffect->Get_TileY());
	ImGui::DragFloat("Tile Animation Speed", pEffect->Get_TileTickPerSec(), 1.f, 0.f, 240.f, "%.0f");
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::ColorEdit4("Center Color##picker", reinterpret_cast<_float*>(pEffect->Get_CenterColor()), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_None);
	ImGui::SameLine();
	ImGui::Text("Center Color");
	ImGui::SameLine();
	ImGui::DragFloat("Glow Intensity", pEffect->Get_EmissiveIntensity_Ptr(), 0.05f, 0.0f, 10.f, "%.2f");
	ImGui::DragFloat("Distortion Strength", pEffect->Get_DistortionStrength_Ptr(), 0.1f, 0.f, 255.f, "%.1f");
	ImGui::PopItemWidth();
	string strRGName;
	for (_uint i = ENUM_CLASS(RENDERGROUP::RG_BLEND); i < ENUM_CLASS(RENDERGROUP::RG_EFFECT_END); i++)
	{
		if (i == ENUM_CLASS(RENDERGROUP::RG_BLEND))strRGName = "RG_BLEND";
		if (i == ENUM_CLASS(RENDERGROUP::RG_EFFECT_GLOW))strRGName = "RG_EFFECT_GLOW";
		if (i == ENUM_CLASS(RENDERGROUP::RG_EFFECT_WB))strRGName = "RG_EFFECT_WB";
		if (i == ENUM_CLASS(RENDERGROUP::RG_EFFECT_NL))strRGName = "RG_EFFECT_NL";
		if (i == ENUM_CLASS(RENDERGROUP::RG_EFFECT_LIGHT))strRGName = "RG_EFFECT_LIGHT";
		if (ImGui::RadioButton(strRGName.c_str(), *pEffect->Get_RenderGroup_Ptr() == i)) {
			*pEffect->Get_RenderGroup_Ptr() = i;
		}
		if (i!= ENUM_CLASS(RENDERGROUP::RG_EFFECT_NL))
			ImGui::SameLine();
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	// �ؽ��� ���� â
	if (FAILED(Draw_TextureBrowser(pEffect)))
	{
		MSG_BOX("Failed to draw texture browser");
		ImGui::End();
		return E_FAIL;
	}

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	


	// Ű������ 
	Edit_Keyframes(pEffect);

	// �ؽ��� ����, UV ĭ �� ���� ��

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	ImGui::Begin("Type Window");


	switch (m_pSequence->m_Items[m_iSelected].iType)
	{
	case Client::EFF_SPRITE:
		if (FAILED(Window_Sprite()))
		{
			ImGui::End();
			ImGui::End();
			return E_FAIL;
		}
		ImGui::End();
		break;
	case Client::EFF_PARTICLE:
		if (FAILED(Window_Particle()))
		{
			ImGui::End();
			ImGui::End();
			return E_FAIL;
		}
		ImGui::End();
		break;
	case Client::EFF_MESH:
		if (FAILED(Window_Mesh()))
		{
			ImGui::End();
			ImGui::End();
			return E_FAIL;
		}
		ImGui::End();
		break;
	case Client::EFF_TRAIL:
		if (FAILED(Window_Trail()))
		{
			ImGui::End();
			ImGui::End();
			return E_FAIL;
		}
		ImGui::End();
		break;
	}
	//ImGui::SameLine();


	ImGui::End();

	Guizmo_Tool();

	return S_OK;
}

void CCYTool::Change_SelectedItem(class CEffectBase* pEffect)
{
	if (m_iSelected >= m_pSequence->m_Items.size())
	{
		m_iLastSelected = m_iSelected = -1;
		return;
	}

}

HRESULT CCYTool::Window_Sprite()
{
	CToolSprite* pSE = dynamic_cast<CToolSprite*>(m_pSequence->m_Items[m_iSelected].pEffect);

	ImGui::Text("Select Pass\n4. UVSprite_Color_WB\t5. MaskOnly\t6. MaskNoise\t7. DistortionOnlyFlow\n8. Distortion NoMaskFlow");

	for (_uint i = SE_UVSPRITE_COLOR_WB; i < SE_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##SE").c_str(), *pSE->Get_ShaderPass_Ptr() == i)) {
			*pSE->Get_ShaderPass_Ptr() = (SPRITEEFFECT_PASS_INDEX)i;
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));


	return S_OK;
}

HRESULT CCYTool::Window_Particle()
{
	CToolParticle* pPE = dynamic_cast<CToolParticle*>(m_pSequence->m_Items[m_iSelected].pEffect);
	if (m_iLastSelected != m_iSelected)
	{
		auto& desc = pPE->Get_InstanceBufferDesc();

		m_eParticleType = desc.ePType;
		m_iNumInstance = desc.iNumInstance;
		m_vLifeTime = desc.vLifeTime;
		m_vRange = desc.vRange;
		m_vSize = desc.vSize;
		m_vSpeed = desc.vSpeed;
		m_vRotationSpeed = desc.vRotationSpeed;
		m_vOrbitSpeed = desc.vOrbitSpeed;
		m_vAccel = desc.vAccel;
		m_vMin_MaxSpeed.x = desc.fMinSpeed;
		m_vMin_MaxSpeed.y = desc.fMaxSpeed;
		m_iLastSelected = m_iSelected;
		m_tPCB.bUseGravity = desc.bGravity;
		m_tPCB.fGravity = desc.fGravity;
		m_tPCB.vOrbitAxis = desc.vOrbitAxis;
		m_tPCB.bUseOrbit = desc.bOrbit;
		m_tPCB.bUseSpin = desc.bSpin;
		m_tPCB.vPivot = desc.vPivot;
		m_tPCB.bIsLoop = desc.isLoop;
		m_tPCB.vCenter = desc.vCenter;
		m_tPCB.vTileCnt = desc.vTileCnt;
		m_tPCB.isTileLoop = desc.isTileLoop;
	}

	ImGui::Text("Select Pass\n0. Default\t1. MaskOnly\t2. WBTest\t 3. vstretch\n4. Diffuse_WB\t5. Rain(Distort+Mask)");
	for (_uint i = 0; i < PE_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##PE").c_str(), *pPE->Get_ShaderPass_Ptr() == i)) {
			*pPE->Get_ShaderPass_Ptr() = (PARTICLEEFFECT_PASS_INDEX)i;
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	m_tPCB = pPE->Get_CBuffer();

	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	ImGui::DragInt("Num Instance", &m_iNumInstance, 1, 1, 10000, "%d");
	ImGui::DragFloat3("Pivot", reinterpret_cast<_float*>(&m_tPCB.vPivot), 0.01f, -1000.f, 1000.f, "%.2f");
	ImGui::DragFloat3("Center", reinterpret_cast<_float*>(&m_tPCB.vCenter), 0.01f, -1000.f, 1000.f, "%.2f");
	ImGui::DragFloat3("Range", reinterpret_cast<_float*>(&m_vRange), 0.01f, 0.01f, 1000.f, "%.2f");
	ImGui::DragFloat2("Speed", reinterpret_cast<_float*>(&m_vSpeed), 0.01f, 0.01f, 1000.f, "%.2f");
	ImGui::DragFloat2("Size", reinterpret_cast<_float*>(&m_vSize), 0.01f, 0.01f, 1000.f, "%.2f");
	ImGui::DragFloat2("LifeTime", reinterpret_cast<_float*>(&m_vLifeTime), 0.01f, 0.f, 200.f, "%.2f");	
	ImGui::DragFloat("##Gravity Power", reinterpret_cast<_float*>(&m_tPCB.fGravity), 0.1f, 0.f, 200.f, "%.1f"); ImGui::SameLine();

	m_bUseGravity = m_tPCB.bUseGravity == 0 ? false : true;
	ImGui::Checkbox("Gravity", &m_bUseGravity);
	m_tPCB.bUseGravity = m_bUseGravity == true ? 1 : 0;

	// --- ����(Spin) ---
	ImGui::DragFloat2("Spin Speed (deg/s)", reinterpret_cast<_float*>(&m_vRotationSpeed), 0.1f, -2000.f, 2000.f);

	// --- ����(Orbit) ---
	ImGui::InputFloat3("Orbit Axis", &m_tPCB.vOrbitAxis.x);
	ImGui::SameLine();
	if (ImGui::Button("Normalize##orbit")) {
		_vector a = XMVector3Normalize(XMLoadFloat3(&m_tPCB.vOrbitAxis));
		XMStoreFloat3(&m_tPCB.vOrbitAxis, a);
	}
	ImGui::DragFloat2("Orbit Speed (deg/s)", reinterpret_cast<_float*>(&m_vOrbitSpeed), 0.1f, -2000.f, 2000.f);

	// ������
	ImGui::DragFloat2("Accel", reinterpret_cast<_float*>(&m_vAccel), 0.01f, -1000.f, 1000.f, "%.2f");
	ImGui::DragFloat2("Min/Max Speed", reinterpret_cast<_float*>(&m_vMin_MaxSpeed), 0.01f, -1000.f, 1000.f, "%.2f");
	
	// Ÿ�� ���� ����
	m_bIsTileLoop = m_tPCB.isTileLoop == 0 ? false : true;
	ImGui::Checkbox("TileLoop", &m_bIsTileLoop);
	m_tPCB.isTileLoop = m_bIsTileLoop == true ? 1 : 0;

	// stretch �ÿ� �þ�� ����
	if (*pPE->Get_ShaderPass_Ptr() == PE_WB_VSTRETCH)
	{
		ImGui::DragFloat("StretchFactor", pPE->Get_StretchFactor_Ptr(), 0.001f, 0.001f, 10.f, "%.3f");
	}
	/**************************************************/
	
	m_bIsLoop = m_tPCB.bIsLoop == 0 ? false : true;
	ImGui::Checkbox("Loop", &m_bIsLoop); ImGui::SameLine();
	m_tPCB.bIsLoop = m_bIsLoop == true ? 1 : 0;

	m_bUseSpin = m_tPCB.bUseSpin == 0 ? false : true;
	ImGui::Checkbox("Spin", &m_bUseSpin); ImGui::SameLine();
	m_tPCB.bUseSpin = m_bUseSpin == true ? 1 : 0;

	m_bUseOrbit = m_tPCB.bUseOrbit == 0 ? false : true;
	ImGui::Checkbox("Orbit", &m_bUseOrbit);
	m_tPCB.bUseOrbit = m_bUseOrbit == true ? 1 : 0;



	//ImGui::DragFloat("Rotation Speed", &m_fRotationSpeed, 1.f, -360.f, 360.f, "%.1f");
	ImGui::SeparatorText("Particle Type");
	if (ImGui::RadioButton("Spread", m_eParticleType == PTYPE_SPREAD)) {
		m_eParticleType = PTYPE_SPREAD;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Directional", m_eParticleType == PTYPE_DIRECTIONAL)) {
		m_eParticleType = PTYPE_DIRECTIONAL;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Random", m_eParticleType == PTYPE_ALLRANDOM)) {
		m_eParticleType = PTYPE_ALLRANDOM;
	}
	m_tPCB.vTileCnt = _float2(_float(*pPE->Get_TileX()), _float(*pPE->Get_TileY()));
	m_tPCB.fTileTickPerSec = *pPE->Get_TileTickPerSec();
	pPE->Set_CBuffer(m_tPCB);

	if(ImGui::Button("Update Particle"))
	{
		CVIBuffer_Point_Instance::DESC desc = {};
		desc.ePType = m_eParticleType;
		desc.iNumInstance = m_iNumInstance;
		desc.isLoop = m_bIsLoop;
		desc.vCenter = m_tPCB.vCenter;
		desc.vLifeTime = m_vLifeTime;
		desc.vPivot = m_tPCB.vPivot;
		desc.vRange = m_vRange;
		desc.vSize = m_vSize;
		desc.vSpeed = m_vSpeed;
		desc.bGravity = m_tPCB.bUseGravity;
		desc.fGravity = m_tPCB.fGravity;
		desc.vOrbitAxis = m_tPCB.vOrbitAxis;
		desc.vRotationSpeed = m_vRotationSpeed;
		desc.vOrbitSpeed = m_vOrbitSpeed;
		desc.bOrbit = m_tPCB.bUseOrbit;
		desc.bSpin = m_tPCB.bUseSpin;
		desc.vAccel = m_vAccel;
		desc.fMinSpeed = m_vMin_MaxSpeed.x;
		desc.fMaxSpeed = m_vMin_MaxSpeed.y;
		desc.isTool = true;
		desc.vTileCnt = m_tPCB.vTileCnt;
		desc.isTileLoop = m_tPCB.isTileLoop;
		desc.fTileTickPerSec = m_tPCB.fTileTickPerSec;
		pPE->Change_InstanceBuffer(&desc);
	}

	// Accel, Decel
	// Random? <- ������ ������ ���� ���͹�, ������ ���Ⱒ�� �ִ�ġ ���� �ʿ�
	// Rotation Speed

	return S_OK;
}

HRESULT CCYTool::Window_Mesh()
{
	CToolMeshEffect* pME = dynamic_cast<CToolMeshEffect*>(m_pSequence->m_Items[m_iSelected].pEffect);
	if (pME == nullptr)
		return E_FAIL;

	ImGui::Text("Select Pass\n0. Default\t1. Mask only\t2. Mask Noise\t3. UVMask\t4. Mask Only WB");
	for (_uint i = 0; i < ME_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##ME").c_str(), *pME->Get_ShaderPass_Ptr() == i)) {
			*pME->Get_ShaderPass_Ptr() = (MESHEFFECT_PASS_INDEX)i;
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	if (ImGui::Button("Load Models"))
	{
		IGFD::FileDialogConfig config;
		config.path = R"(..\Bin\Resources\Models\EffectMesh)";
		config.countSelectionMax = 0; // ������
		IFILEDIALOG->OpenDialog("Mesh Effect Model Select", "Select File", ".bin", config);
		
	}
	if (FAILED(Load_EffectModel()))
		return E_FAIL;
	

	if (ImGui::BeginCombo("##SelectModel", m_iSelectedModelIdx >= 0 ? m_ModelNames[m_iSelectedModelIdx].c_str() : "Preset Model"))
	{
		for (_int i = 0; i < m_ModelNames.size(); ++i)
		{
			_bool isSelected = (i == m_iSelectedModelIdx);
			if (ImGui::Selectable(m_ModelNames[i].c_str(), isSelected))
			{
				if (FAILED(pME->Change_Model(StringToWString(m_ModelNames[i]))))
				{
					MSG_BOX("�� ���� ���� ");
					return E_FAIL;
				}
				m_iSelectedModelIdx = i;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	//Edit_Keyframes(pME);

	return S_OK;
}

HRESULT CCYTool::Window_Trail()
{
	CToolTrail* pTE = dynamic_cast<CToolTrail*>(m_pSequence->m_Items[m_iSelected].pEffect);

	ImGui::Text("Select Pass\n0. Default\n1.Drop");

	for (_uint i = 0; i < TE_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##TE").c_str(), *pTE->Get_ShaderPass_Ptr() == i)) {
			*pTE->Get_ShaderPass_Ptr() = (TRAILEFFECT_PASS_INDEX)i;
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	if (ImGui::Button("Enable Trail"))
	{
		pTE->Set_TrailActive(true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Disable Trail"))
	{
		pTE->Set_TrailActive(false);
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	ImGui::DragFloat("LifeDuration##Trail", pTE->Get_LifeDuration_Ptr(), 0.01f, 0.f, 100.f, "%.2f");
	static _int fNodeIntervalFPS = 60;
	ImGui::DragInt("NodeInterval##Trail", &fNodeIntervalFPS, 1, 1, 1024);
	*pTE->Get_NodeInterval_Ptr() = 1.f / (_float)fNodeIntervalFPS;
	ImGui::DragInt("CatmullRomDiv##Trail", pTE->Get_Subdivisions_Ptr(), 1, 0, 10);

	return S_OK;
}

void CCYTool::Edit_Keyframes(CEffectBase* pEffect)
{
	ImGui::Begin("Edit Keyframes");
	auto& KeyFrames = pEffect->Get_KeyFrames();

	_int iIdx = {};
	for (auto& Keyframe : KeyFrames)
	{
		ImGui::DragFloat(string("KeyFrame" + to_string(iIdx)).c_str(), &Keyframe.fTrackPosition, 1.f, 0.f, static_cast<_float>(*pEffect->Get_Duration_Ptr()/* + *m_pSequence->m_Items[m_iSelected].iStart*/), "%.0f");

		ImGui::DragFloat3(string("Translation##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&Keyframe.vTranslation), 0.1f);

		_float3 vRotAxis = QuaternionToEuler(XMLoadFloat4(&Keyframe.vRotation));
		ImGui::BeginDisabled();
		ImGui::DragFloat3(string("Rotation##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&vRotAxis));
		ImGui::EndDisabled();
		// i'm super ssayeeko effecter CY Park
		ImGui::DragFloat3(string("Scaling##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&Keyframe.vScale), 0.1f);
		ImGui::DragFloat(string("Intensity##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&Keyframe.fIntensity), 0.01f, -10.f, 20.f, "%.2f");

		ImGui::ColorEdit4(string("Color##" + to_string(iIdx)).c_str(), (float*)&Keyframe.vColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_None);
		ImGui::SameLine();
		if (ImGui::RadioButton(("Select Keyframe " + to_string(iIdx)).c_str(), m_iSelectedKeyframe == iIdx))
			m_iSelectedKeyframe = iIdx;
		_int iSelected = Keyframe.eInterpolationType;
		if (ImGui::Combo(string("##interpolation type" + to_string(iIdx)).c_str(), &iSelected, m_InterpolationTypes, IM_ARRAYSIZE(m_InterpolationTypes)))
		{
			Keyframe.eInterpolationType = static_cast<INTERPOLATION>(iSelected);
			//pTS->Set_InterpolationType(iIdx, static_cast<CEffectBase::INTERPOLATION>(iSelected));
			//:3
			//:>
		}

		++iIdx;
		ImGui::Separator();
	}
	if (ImGui::Button("Add Keyframe"))
	{
		pEffect->Add_KeyFrame(CEffectBase::EFFKEYFRAME{});
	}
	if (iIdx > 1 && ImGui::Button("Delete Keyframe"))
	{
		pEffect->Delete_KeyFrame();
	}
	ImGui::End();
}

HRESULT CCYTool::Load_EffectModel()
{
	if (IFILEDIALOG->Display("Mesh Effect Model Select"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// ó��
			// first: ���ϸ�.Ȯ����
			// second: ��ü ��� (���ϸ�����)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{
					Make_EffectModel_Prototypes(FilePath.second);
				}
			}
		}
		IFILEDIALOG->Close();
	}

	return S_OK;
}

HRESULT CCYTool::Make_EffectModel_Prototypes(const string strModelFilePath)
{
	path ModelPath = strModelFilePath;
	path ModelName = ModelPath.stem();
	_wstring strModelTag = ModelName.wstring();

	_matrix		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), _wstring(L"Prototype_Component_Model_") + strModelTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelFilePath.c_str(), PreTransformMatrix))))
		return E_FAIL;

	m_ModelNames.push_back(ModelName.string());
	return S_OK;
}

HRESULT CCYTool::Save_EffectSet()
{
	IGFD::FileDialogConfig config;
	config.path = R"(../Bin/Save/Effect/EffectContainer/)";

	IFILEDIALOG->OpenDialog("SaveEffectsetDialog", "Choose directory to save", ".json", config);

	if (IFILEDIALOG->Display("SaveEffectsetDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path savePath = IFILEDIALOG->GetFilePathName();

			ofstream ofs(savePath);
			if (!ofs.is_open())
				return E_FAIL;
			json jSave;

			jSave["MaxFrame"] = m_pSequence->m_iFrameMax;
			jSave["Loop"] = m_pSequence->m_bLoop;

			auto& Items = m_pSequence->Get_Items();

			for (auto& Item : Items)
			{
				json jItem;
				jItem = Item.pEffect->Serialize();
				jItem["Name"] = Item.strName.data();
				jItem["EffectType"] = Item.iType;
				//jItem["EffectPreferences"].push_back(Item.pEffect->Serialize());

				jSave["EffectObject"].push_back(jItem);
			}
			ofs << setw(4) << jSave;
			ofs.close();
		}
		m_bOpenSaveEffectContainer = false;
		IFILEDIALOG->Close();
	}

	return S_OK;
}

HRESULT CCYTool::Load_EffectSet()
{
	IGFD::FileDialogConfig config;
	config.path = R"(../Bin/Save/Effect/EffectContainer/)";

	json jLoad;
	IFILEDIALOG->OpenDialog("LoadEffectsetDialog", "Choose File to Load", ".json", config);

	if (IFILEDIALOG->Display("LoadEffectsetDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path loadPath = IFILEDIALOG->GetFilePathName();
			string filename = IFILEDIALOG->GetCurrentFileName();
			string prefix = filename.substr(0, 2);

			// put extension '.json'
			if (loadPath.extension().string() != ".json")
				loadPath += ".json";

			// Compare Prefix
			if (prefix != "EC")
			{
				MSG_BOX("Filename should start with EC (EffectContainer)");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return S_OK;
			}

			ifstream ifs(loadPath);
			if (!ifs.is_open())
			{
				MSG_BOX("File open Failed");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return E_FAIL;
			}

			ifs >> jLoad;
			ifs.close();


			if (!jLoad.contains("EffectObject") || !jLoad["EffectObject"].is_array())
			{
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return E_FAIL;
			}

			Safe_Delete(m_pSequence);
			m_pSequence = new CEffectSequence();

			if (jLoad.contains("MaxFrame"))
				m_pSequence->m_iFrameMax = jLoad["MaxFrame"].get<_int>();
			if (jLoad.contains("Loop"))
				m_pSequence->m_bLoop = jLoad["Loop"].get<_bool>();

			for (const auto& jItem : jLoad["EffectObject"])
			{
				// �̸�
				if (jItem.contains("Name"))
					m_strSeqItemName = jItem["Name"].get<string>();

				// Ÿ��
				if (jItem.contains("EffectType"))
					m_eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<_int>());

				{
					/* �� �κе� �����ؾ��ϴµ� �ٸ��� �켱���Ƽ� ����ó�� �� */
					const json& jData = jItem;

					CEffectBase* pInstance = { nullptr };

					switch (m_eEffectType)
					{
					case Client::EFF_SPRITE:
					{
						m_eEffectType = EFF_SPRITE;
						m_iSeqItemColor = D3DCOLOR_ARGB(255, 200, 60, 40);
						CToolSprite::DESC desc = {};
						desc.bTool = true;
						desc.bLoadingInTool = true;
						pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
							PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
						if (pInstance != nullptr)
						{
							pInstance->Deserialize(jData);
							pInstance->Ready_Textures_Prototype_Tool();
							m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
						}
					}
					break;
					case Client::EFF_PARTICLE:
					{
						m_eEffectType = EFF_PARTICLE;
						m_iSeqItemColor = D3DCOLOR_ARGB(255, 60, 200, 80);
						CToolParticle::DESC desc = {};
						desc.fRotationPerSec = XMConvertToRadians(90.f);
						desc.fSpeedPerSec = 5.f;
						desc.bTool = true;
						desc.bLoadingInTool = true;
						pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
							PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"), &desc));
						if (pInstance != nullptr)
						{
							pInstance->Deserialize(jData);
							pInstance->Ready_Textures_Prototype_Tool();
							static_cast<CToolParticle*>(pInstance)->Change_InstanceBuffer(nullptr);
							m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
						}
					}
					break;
					case Client::EFF_MESH:
					{
						m_eEffectType = EFF_MESH;
						m_iSeqItemColor = D3DCOLOR_ARGB(255, 100, 100, 220);
						CToolMeshEffect::DESC desc = {};
						desc.fRotationPerSec = XMConvertToRadians(90.f);
						desc.fSpeedPerSec = 5.f;
						desc.bTool = true;
						desc.bLoadingInTool = true;
						pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
							PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"), &desc));
						if (pInstance != nullptr)
						{
							pInstance->Deserialize(jData);
							pInstance->Ready_Textures_Prototype_Tool();
							static_cast<CToolMeshEffect*>(pInstance)->Change_Model(L"");
							m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
						}
					}
					break;
					case Client::EFF_TRAIL:
					{
						m_eEffectType = EFF_TRAIL;
						m_strSeqItemName = "Trail";
						m_iSeqItemColor = D3DCOLOR_ARGB(255, 170, 80, 250);
					}
					break;
					}

					if (pInstance == nullptr)
					{
						MSG_BOX("Failed to make Effect");
						m_bOpenLoadEffectContainer = false;
						m_iSelected = -1;
						IFILEDIALOG->Close();
						return E_FAIL;
					}
				}

			}
		}
		m_bOpenLoadEffectContainer = false;
		m_iSelected = -1;
		IFILEDIALOG->Close();
	}
	return S_OK;

}

HRESULT CCYTool::Save_Effect()
{
	IGFD::FileDialogConfig config;
	config.path = R"(..\Bin\Save\Effect\)";

	IFILEDIALOG->OpenDialog("SaveEffectonlyDialog", "Choose directory to save", ".json", config);

	if (IFILEDIALOG->Display("SaveEffectonlyDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path savePath = IFILEDIALOG->GetFilePathName();

			// Ȯ���ڰ� ������ .json ���̱�
			if (savePath.extension().string() != ".json")
				savePath += ".json";

			ofstream ofs(savePath);

			if (!ofs.is_open())
				return E_FAIL;

			json jSave = m_pSequence->m_Items[m_iSelected].pEffect->Serialize();
			jSave["Name"] = m_pSequence->m_Items[m_iSelected].strName.data();
			jSave["EffectType"] = m_pSequence->m_Items[m_iSelected].iType;

			ofs << setw(4) << jSave;
			ofs.close();
		}
		m_bOpenSaveEffectOnly = false;
		IFILEDIALOG->Close();
	}

	return S_OK;
}

// ����Ʈ ���� �ε�
HRESULT CCYTool::Load_Effect()
{
	IGFD::FileDialogConfig config;
	config.path = R"(..\Bin\Save\Effect\)";
	json j;
	IFILEDIALOG->OpenDialog("LoadEffectonlyDialog", "Choose File to Load", ".json", config);

	if (IFILEDIALOG->Display("LoadEffectonlyDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path loadPath = IFILEDIALOG->GetFilePathName();
			string filename = IFILEDIALOG->GetCurrentFileName();
			string prefix = filename.substr(0, 2);
			
			// put extension '.json'
			if (loadPath.extension().string() != ".json")
				loadPath += ".json";

			// Compare Prefix for making each effects
			// Check the filenames before saving
			if (prefix == "SE"){

				m_eEffectType = EFF_SPRITE;
				m_strSeqItemName = "Sprite";
				m_iSeqItemColor = D3DCOLOR_ARGB(255, 200, 60, 40);
			}
			else if (prefix == "PE"){
				m_eEffectType = EFF_PARTICLE;
				m_strSeqItemName = "Particle";
				m_iSeqItemColor = D3DCOLOR_ARGB(255, 60, 200, 80);
			}
			else if (prefix == "ME"){
				m_eEffectType = EFF_MESH;
				m_strSeqItemName = "Mesh";
				m_iSeqItemColor = D3DCOLOR_ARGB(255, 100, 100, 220);
			}
			else if (prefix == "TE"){
				m_eEffectType = EFF_TRAIL;
				m_strSeqItemName = "Trail";
				m_iSeqItemColor = D3DCOLOR_ARGB(255, 170, 80, 250);
			}
			else
			{
				MSG_BOX("Filename should start with \"SE / PE / ME / TE\"");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return S_OK;
			}

			ifstream ifs(loadPath);
			if (!ifs.is_open())
			{
				MSG_BOX("File open Failed");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return E_FAIL;
			}

			ifs >> j;
			ifs.close();

			CEffectBase* pInstance = { nullptr };

			switch (m_eEffectType)
			{
			case Client::EFF_SPRITE:
			{
				CToolSprite::DESC desc = {};
				desc.bTool = true;
				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
			}
			break;
			case Client::EFF_PARTICLE:
			{
				CToolParticle::DESC desc = {};
				desc.fRotationPerSec = XMConvertToRadians(90.f);
				desc.fSpeedPerSec = 5.f;
				desc.bTool = true;
				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"), &desc));
			}
			break;
			case Client::EFF_MESH:
			{
				CToolMeshEffect::DESC desc = {};
				desc.fRotationPerSec = XMConvertToRadians(90.f);
				desc.fSpeedPerSec = 5.f;
				desc.bTool = true;
				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"), &desc));
			}
			break;
			case Client::EFF_TRAIL:
			{
				CGameObject* TestWeapon = m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::CY), TEXT("Player_Weapon"), 0);
				CModel* pWeaponModelCom = static_cast<CModel*>(TestWeapon->Get_Component(TEXT("Com_Model")));
				_uint iInnerBoneIdx = pWeaponModelCom->Find_BoneIndex("BN_Blade_B");
				_uint iOuterBoneIdx = pWeaponModelCom->Find_BoneIndex("BN_Blade_End");

				CToolTrail::DESC desc = {};
				desc.bAnimation = true;
				desc.fRotationPerSec = 0.f;
				desc.fSpeedPerSec = 5.f;
				desc.bTool = true;
				desc.pInnerSocketMatrix = const_cast<_float4x4*>(pWeaponModelCom->Get_CombinedTransformationMatrix(iInnerBoneIdx));
				desc.pOuterSocketMatrix = const_cast<_float4x4*>(pWeaponModelCom->Get_CombinedTransformationMatrix(iOuterBoneIdx));
				desc.pParentCombinedMatrix = const_cast<_float4x4*>(static_cast<CWeapon*>(TestWeapon)->Get_CombinedWorldMatrix());

				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolTrailEffect"), &desc));
			}
				break;
			}
			if (pInstance != nullptr)
			{
				pInstance->Deserialize(j);
				pInstance->Ready_Textures_Prototype_Tool();
				if (m_eEffectType == EFF_PARTICLE)
					static_cast<CToolParticle*>(pInstance)->Change_InstanceBuffer(nullptr);
				if (m_eEffectType == EFF_MESH)
					static_cast<CToolMeshEffect*>(pInstance)->Change_Model(L"");
				if (m_eEffectType == EFF_TRAIL)
					static_cast<CToolTrail*>(pInstance)->Change_TrailBuffer(nullptr);

				// �̸�
				if (j.contains("Name"))
					m_strSeqItemName = j["Name"].get<string>();

				m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
			}
			else
			{
				MSG_BOX("Failed to make Effect");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return E_FAIL;
			}
		}
		m_bOpenLoadEffectOnly = false;
		IFILEDIALOG->Close();
	}

	return S_OK;
}

HRESULT CCYTool::Load_Textures()
{
	path TexturePath = R"(../Bin/Resources/Textures/Effect/)";
	if (!exists(TexturePath))
	{
		MSG_BOX("�ؽ��� ��ΰ� �̻��ؿ�");
		return E_FAIL;
	}

	for (const auto& entry : directory_iterator(TexturePath))
	{
		if (entry.is_regular_file() && (entry.path().extension() == L".dds" || entry.path().extension() == L".png"))
		{
			_wstring filePath = entry.path().wstring();
			_wstring stemName = entry.path().stem().wstring();
			_wstring prototypeTag = L"Prototype_Component_Texture_" + stemName;
			if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), prototypeTag,
				CTexture::Create(m_pDevice, m_pContext,
					filePath.c_str(), 1))))
			{
				string msg = "���� ���� ����:\n��� ���: ";

				msg += WStringToString(filePath);

				MessageBoxA(nullptr, msg.c_str(), "����", MB_OK | MB_ICONERROR);
				continue;
			}

			string strRelativePath = filesystem::relative(entry.path(), filesystem::current_path()).string();
			m_Textures.push_back({
				static_cast<CTexture*>(m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::CY), prototypeTag))->Get_SRV(0),
				WStringToString(stemName)
				});
		}
	}

	return S_OK;
}

HRESULT CCYTool::Draw_TextureBrowser(CEffectBase* pEffect)
{
	ImGui::BeginGroup();

	// �ؽ�ó �̸����� ��� (�巡�� ����)
	ImGui::Text("Select a Texture:");
	_int columnCount = 5;

	ImGui::BeginChild("TextureGrid", ImVec2(400.f, 300.f), true);
	ImGui::Columns(columnCount, nullptr, false);

	for (_int i = 0; i < m_Textures.size(); ++i) {
		ImGui::PushID(i);

		// �̹��� ��ư
		if (ImGui::ImageButton("##TexBtn", reinterpret_cast<ImTextureID>(m_Textures[i].pSRV), ImVec2(64.f, 64.f)))
			m_iSelectedTextureIdx = i;

		// �巡�� �ҽ� ���
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("TEXTURE_PAYLOAD", &i, sizeof(_int));
			ImGui::Text("Dragging: %s", m_Textures[i].name.c_str());
			ImGui::EndDragDropSource();
		}

		ImGui::TextWrapped("%s", m_Textures[i].name.c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	ImGui::Columns(1);
	ImGui::EndChild();

	ImGui::EndGroup();
	ImGui::Separator();
	//ImGui::SameLine();

	// ���� UI (Directional Target)
	ImGui::BeginGroup();
	ImGui::Text("Texture Slots:");
	const _char* slotNames[4] = { "Diffuse", "Mask1", "Mask2", "Mask3" };
	ImVec2 slotSize = ImVec2(48.f, 48.f);

	for (_int slotIdx = 0; slotIdx < TU_END; ++slotIdx) {
		if (slotIdx > 0)
			ImGui::SameLine(); // ���� ����
		ImGui::BeginGroup();
		ImGui::PushID(slotIdx);

		ImGui::Text("%s", slotNames[slotIdx]);

		if (m_pSlotSRV[slotIdx])
			ImGui::Image(reinterpret_cast<ImTextureID>(m_pSlotSRV[slotIdx]), slotSize);
		else
			ImGui::Dummy(slotSize);

		// ��� ��� ó��
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_PAYLOAD")) {
				_int draggedIndex = *(const int*)payload->Data;

				m_pSlotSRV[slotIdx] = m_Textures[draggedIndex].pSRV;
				m_SlotTexNames[slotIdx] = m_Textures[draggedIndex].name;

				if (FAILED(pEffect->Change_Texture(StringToWString(m_SlotTexNames[slotIdx]), (TEXUSAGE)slotIdx)))
					return E_FAIL;
			}
			ImGui::EndDragDropTarget();
		}

		// �̸� ��� �� Clear ��ư
		if (!m_SlotTexNames[slotIdx].empty()) {
			ImGui::TextWrapped("(%s)", m_SlotTexNames[slotIdx].c_str());
			if (ImGui::Button("Clear")) {
				m_pSlotSRV[slotIdx] = nullptr;
				m_SlotTexNames[slotIdx].clear();
				pEffect->Delete_Texture((TEXUSAGE)slotIdx);
			}
		}

		//ImGui::Separator();
		ImGui::PopID();
		ImGui::EndGroup();
	}

	ImGui::EndGroup();

	return S_OK;
}

HRESULT CCYTool::Guizmo_Tool()
{
	if (m_isGizmoEnable && m_iSelected != -1 && m_iSelectedKeyframe != -1)
	{
		// ImGui ������ ���ο��� ȣ��

		ImGuizmo::SetOrthographic(false); // ���� ��� ���� (false = Perspective)
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList()); // �⺻ ImGui drawlist ���

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

		auto& curKeyframe = m_pSequence->m_Items[m_iSelected].pEffect->Get_KeyFrames()[m_iSelectedKeyframe];

		_vector vCurKeyframeTranslation =	XMLoadFloat3(&curKeyframe.vTranslation);
		_vector vCurKeyframeRotation =		XMLoadFloat4(&curKeyframe.vRotation);
		_vector vCurKeyframeScale =			XMLoadFloat3(&curKeyframe.vScale);

		_matrix matCurKFWorld = XMMatrixAffineTransformation(
			vCurKeyframeScale,
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			vCurKeyframeRotation,
			vCurKeyframeTranslation
		);

		_float4x4 matWorld = {};
		XMStoreFloat4x4(&matWorld, matCurKFWorld);

		_float4x4 matView = *m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW);
		_float4x4 matProj = *m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ);

		/* ���� ������ ����, ȸ���� 15�� ����*/
		_float snapoffset[3] = {};
		if (KEY_PRESSING(DIK_LSHIFT))
		{
			if (m_eOperation == ImGuizmo::ROTATE)
				snapoffset[0] = 15.f;
			else
				snapoffset[0] = snapoffset[1] = snapoffset[2] = 1.f;
		}

		/* ���� �����ϴ� �κ� */
		ImGuizmo::Manipulate(
			reinterpret_cast<const _float*>(&matView),
			reinterpret_cast<const _float*>(&matProj),
			m_eOperation, m_eMode,
			reinterpret_cast<_float*>(&matWorld),
			nullptr, snapoffset
		);
		XMMatrixDecompose(&vCurKeyframeScale, &vCurKeyframeRotation, &vCurKeyframeTranslation, XMLoadFloat4x4(&matWorld));
		XMStoreFloat3(&curKeyframe.vTranslation, vCurKeyframeTranslation);
		XMStoreFloat4(&curKeyframe.vRotation, vCurKeyframeRotation);
		XMStoreFloat3(&curKeyframe.vScale, vCurKeyframeScale);
	}
	return S_OK;
}


void CCYTool::Key_Input()
{
	if (KEY_DOWN(DIK_SPACE))
	{
		m_bPlaySequence = !m_bPlaySequence;
	}

	
	if (KEY_PRESSING(DIK_LCONTROL))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			if (m_pSequence &&
				!m_pSequence->m_Items.empty() &&
				m_iSelected != -1)
			{
				m_pSequence->Del(m_iSelected);
				m_iSelected = -1;
				m_iSelectedKeyframe = 0;
			}
		}
		if (m_isGizmoEnable)
		{
			if (KEY_DOWN(DIK_W)) {
				m_eOperation = ImGuizmo::TRANSLATE;
			}
			else if (KEY_DOWN(DIK_E)) {
				m_eOperation = ImGuizmo::ROTATE;
			}
			else if (KEY_DOWN(DIK_R)) {
				m_eOperation = ImGuizmo::SCALE;
			}
		}
		if (KEY_DOWN(DIK_Q)) {
			m_isGizmoEnable = !m_isGizmoEnable;
			m_iSelectedKeyframe = 0;
		}
		
		if (KEY_DOWN(DIK_T)) {
			if (m_iSelected != -1)
			{
				CEffectBase::EFFKEYFRAME newKF = {};
				newKF.fTrackPosition = static_cast<_float>(m_iCurFrame - *m_pSequence->m_Items[m_iSelected].iStart);
				m_pSequence->m_Items[m_iSelected].pEffect->Add_KeyFrame(newKF);
			}
		}
	}
}

CCYTool* CCYTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CCYTool* pInstance = new CCYTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CCYTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCYTool::Clone(void* pArg)
{
	CCYTool* pInstance = new CCYTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCYTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CCYTool::Free()
{
	__super::Free();

	Safe_Delete(m_pSequence);

	m_Textures.clear();
}
 