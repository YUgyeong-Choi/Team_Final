#include "YGTool.h"
#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감
#include "Camera.h"
#include "Camera_Manager.h"

CYGTool::CYGTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CYGTool::CYGTool(const CYGTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CYGTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CYGTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	return S_OK;
}

void CYGTool::Priority_Update(_float fTimeDelta)
{

}

void CYGTool::Update(_float fTimeDelta)
{
}

void CYGTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CYGTool::Render()
{
	if (FAILED(Render_CaemraTool()))
		return E_FAIL;

	return S_OK;
}


HRESULT CYGTool::Render_CaemraTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("YG Tools", &open, NULL);

    if (ImGui::CollapsingHeader(u8"카메라 위치/방향 제어"))
    {
        if (CCamera* pCam = CCamera_Manager::Get_Instance()->GetCurCam())
        {

        }
        else
        {
            ImGui::Text("현재 활성화된 카메라가 없습니다.");
        }
    }

	ImGui::End();
	return S_OK;
}

CYGTool* CYGTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CYGTool* pInstance = new CYGTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CYGTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGTool::Clone(void* pArg)
{
	CYGTool* pInstance = new CYGTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CYGTool::Free()
{
	__super::Free();

}
