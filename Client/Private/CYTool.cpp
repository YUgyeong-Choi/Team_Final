#include "CYTool.h"
#include "GameInstance.h"
#include "EffectSequence.h"
#include "ToolParticle.h"

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

}

void CCYTool::Update(_float fTimeDelta)
{
}

void CCYTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CCYTool::Render()
{
	if (FAILED(Render_EffectTool()))
		return E_FAIL;
	
	if (FAILED(Test_Sequence()))
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

HRESULT CCYTool::Test_Sequence()
{
	ImGui::Begin("Effect Sequence");

	static _int iCurFrame = {};
	static _bool bExpanded = { true };
	static _int iSelected = { -1 };


	if (Button("ADD"))
	{

	}

	ImSequencer::Sequencer(m_pSequence, &iCurFrame, &bExpanded, &iSelected, nullptr, ImSequencer::SEQUENCER_EDIT_STARTEND);


	ImGui::End();
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

}
