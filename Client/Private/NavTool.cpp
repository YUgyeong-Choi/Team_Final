#include "GameInstance.h"

#include "NavTool.h"
#include "Cell.h"

CNavTool::CNavTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CYWTool(pDevice, pContext)
{
}

CNavTool::CNavTool(const CNavTool& Prototype)
	:CYWTool(Prototype)
{
}

HRESULT CNavTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNavTool::Initialize(void* pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CNavTool::Priority_Update(_float fTimeDelta)
{
}

void CNavTool::Update(_float fTimeDelta)
{
	m_pNavigationCom->Update(XMMatrixIdentity());


	Control(fTimeDelta);
}

void CNavTool::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CNavTool::Render()
{
#ifdef _DEBUG
	m_pNavigationCom->Render();
#endif

	return S_OK;
}

HRESULT	CNavTool::Render_ImGui()
{
	Render_CellList();

	return S_OK;
}

void CNavTool::Control(_float fTimeDelta)
{
	if (GetForegroundWindow() != g_hWnd)
		return;

	//Test T
	if (m_pGameInstance->Key_Down(DIK_T))
	{
		/*if (FAILED(m_pNavigationCom->Set_Index(1)))
			return;*/
	}

	//Ctrl + S 저장
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_S))
	{
		if (FAILED(m_pNavigationCom->Save()))
		{
			MSG_BOX("네비게이션 저장 실패");
		}
		else
		{
			MSG_BOX("네비게이션 저장 성공");
		}
	}

	//셀 선택
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
	{
		_float4 WorldPos = {};
		if (m_pGameInstance->Picking(&WorldPos))
		{
			m_pNavigationCom->Select_Cell(XMLoadFloat4(&WorldPos));
		}
	}

	//셀 삭제
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		m_pNavigationCom->Delete_Cell();
	}

	//Ctrl + 클릭(점 찍기)
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Mouse_Down(DIM::LBUTTON))
	{
		Add_Point();	
	}

}

void CNavTool::Render_CellList()
{
	if (ImGui::Begin("Cell List"))
	{
		for (size_t i = 0; i < m_pNavigationCom->Get_Cells().size(); ++i)
		{
			bool isSelected = (m_pNavigationCom->Get_Index() == static_cast<_int>(i));
			if (ImGui::Selectable(std::to_string(i).c_str(), isSelected))
			{
				m_pNavigationCom->Set_Index(static_cast<_int>(i));
			}
		}
	}
	ImGui::End();


}

void CNavTool::Add_Point()
{
	_float4 WorldPos = {};
	if (m_pGameInstance->Picking(&WorldPos))
	{
		_float3 NewPoint = { WorldPos.x, WorldPos.y, WorldPos.z };

		//이 포인트를 스냅해야한다.
		//모든 셀을 순회하여 가장 가까운 점으로 변경

		//처음 그리는 셀이면 스냅 기능 끄기
		if (m_pNavigationCom->Get_Cells().size() != 0)
		{
			//스냅
			m_pNavigationCom->Snap(&NewPoint, 1.f);
		}

		//모두 스냅하되 일정 거리 이하로 가까워 졌을 때만(스냅할 때 같은점이 들어오면 안됨)
		for (_float3& Point : m_Points)
		{
			if (XMVector3NearEqual(XMLoadFloat3(&Point), XMLoadFloat3(&NewPoint), XMVectorReplicate(0.0001f)))
			{
				MSG_BOX("이미 같은 점 있음");
				return;
			}

		}


		m_Points.push_back(NewPoint);

		if (m_Points.size() == 3)
		{
			Make_Clockwise(m_Points.data());

			if (FAILED(m_pNavigationCom->Add_Cell(m_Points.data())))
			{
				MSG_BOX("셀 추가 실패");
				return;
			}
				
			m_Points.clear();
		}
	}
}

void CNavTool::Make_Clockwise(_float3* Points)
{
	_vector AB =  XMLoadFloat3(&Points[1]) - XMLoadFloat3(&Points[0]);
	_vector AC = XMLoadFloat3(&Points[2]) - XMLoadFloat3(&Points[0]);
	_vector normal = XMVector3Cross(AB, AC);

	// Y축 (0,1,0) 기준 내적
	if (XMVectorGetX(XMVector3Dot(normal, { 0.f, 1.f, 0.f })) < 0)
	{
		swap(Points[1], Points[2]);
	}
}

HRESULT CNavTool::Ready_Components()
{
	/* For.Com_Navigation */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom))))
		return E_FAIL;

	return S_OK;
}


CNavTool* CNavTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CNavTool* pInstance = new CNavTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CNavTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNavTool::Clone(void* pArg)
{
	CNavTool* pInstance = new CNavTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNavTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNavTool::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);

}
