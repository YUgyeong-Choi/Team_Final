#include "CYTool.h"
#include "GameInstance.h"
#include "EffectSequence.h"
#include "ToolParticle.h"
#include "ToolSprite.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

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


	return S_OK;
}

void CCYTool::Priority_Update(_float fTimeDelta)
{
	Key_Input();
	if (m_bPlaySequence)
	{
		m_fTimeAcc += fTimeDelta;
		if (m_fTimeAcc >= 1.f/60.f)
		{
			++m_iCurFrame;
			if (m_iCurFrame > m_pSequence->GetFrameMax())
				m_iCurFrame = m_pSequence->GetFrameMin();
			m_fTimeAcc = 0.f;
		}
		//for (auto& pItem : m_pSequence->m_Items)
		//{
		//	if (m_iCurFrame >= pItem.iStart && m_iCurFrame <= pItem.iEnd)
		//	{
		//		pItem.pEffect->Priority_Update(fTimeDelta);
		//		// 이펙트를 재생
		//		//PlaySpriteEffect(pItem.iType, m_iCurFrame - pItem.iStart);
		//	}
		//}
	}
}

void CCYTool::Update(_float fTimeDelta)
{
	//if (m_bPlaySequence)
	//{
	//	for (auto& pItem : m_pSequence->m_Items)
	//	{
	//		if (m_iCurFrame >= pItem.iStart && m_iCurFrame <= pItem.iEnd)
	//		{
	//			pItem.pEffect->Update(fTimeDelta);
	//			// 이펙트를 재생
	//			//PlaySpriteEffect(pItem.iType, m_iCurFrame - pItem.iStart);
	//		}
	//	}
	//}
}

void CCYTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CCYTool::Render()
{
	//if (FAILED(Render_EffectTool()))
	//	return E_FAIL;
	
	if (FAILED(SequenceWindow()))
		return E_FAIL;

	if (FAILED(Edit_Preferences()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCYTool::Render_EffectTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("CY Tools", &open, NULL);

	IGFD::FileDialogConfig config;
	if (Button(u8"안녕"))
	{

	}

	if (IFILEDIALOG->Display("FBXDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
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

HRESULT CCYTool::Make_Particles()
{
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

	if (ImGui::Button("Add to Sequence"))
	{
		CToolSprite* pInstance = { nullptr };
		CToolSprite::DESC desc = {};

		desc.bAnimation = true;
		desc.fRotationPerSec = 0.f;
		desc.fSpeedPerSec = 5.f;
		desc.iUVHeight = 8.f;
		desc.iUVWidth = 8.f;
		pInstance = dynamic_cast<CToolSprite*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
		m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
	}

	ImSequencer::Sequencer(m_pSequence, &m_iCurFrame, &m_bExpanded, &m_iSelected, &m_iFirstFrame, ImSequencer::SEQUENCER_EDIT_ALL);

	ImGui::End();
	return S_OK;
}

HRESULT CCYTool::Edit_Preferences()
{
	ImGui::Begin("Edit Item Preferences");
	if (m_pSequence == nullptr || m_pSequence->m_Items.empty())
	{
		ImGui::End();
		return S_OK;
	}
	switch (m_pSequence->m_Items[m_iSelected].iType)
	{
	case Client::CCYTool::EFF_SPRITE:
		if (FAILED(Window_Sprite()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	case Client::CCYTool::EFF_PARTICLE:
		if (FAILED(Window_Particle()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	case Client::CCYTool::EFF_MESH:
		if (FAILED(Window_Mesh()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	}
	ImGui::SameLine();


	ImGui::End();


	return S_OK;
}

HRESULT CCYTool::Window_Sprite()
{
	CToolSprite* pTS = dynamic_cast<CToolSprite*>(m_pSequence->m_Items[m_iSelected].pEffect);

	// pTS-> 
	// 속성 imgui로 조정하기
	// 재생 시키기 

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::SeparatorText("Sprite Preferences");
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::PushItemWidth(100);
	ImGui::Text("U Count");
	ImGui::SameLine();
	ImGui::InputInt("##Grid_U", &m_iGridWidthCnt);
	ImGui::SameLine();
	ImGui::Text("V Count");
	ImGui::SameLine();
	ImGui::InputInt("##Grid_V", &m_iGridHeightCnt);
	ImGui::PopItemWidth();

	ImGui::Checkbox("Animation", &m_bAnimateSprite);




	return S_OK;
}

HRESULT CCYTool::Window_Particle()
{
	return S_OK;
}

HRESULT CCYTool::Window_Mesh()
{
	return S_OK;
}

HRESULT CCYTool::Save_Particles()
{
	return S_OK;
}

HRESULT CCYTool::Load_Particles()
{
	return S_OK;
}

void CCYTool::Key_Input()
{
	if (KEY_DOWN(DIK_SPACE))
	{
		m_bPlaySequence = !m_bPlaySequence;
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
}
 