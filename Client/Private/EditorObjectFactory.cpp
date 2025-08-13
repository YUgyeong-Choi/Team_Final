#include "EditorObjectFactory.h"
#include "GameInstance.h"

CEditorObjectFactory::CEditorObjectFactory(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice), m_pContext(pContext)
	,m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CEditorObjectFactory* CEditorObjectFactory::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEditorObjectFactory* pInstance = new CEditorObjectFactory(pDevice, pContext);
	return pInstance;
}
void CEditorObjectFactory::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

}

json CEditorObjectFactory::Serialize()
{
	return json();
}

void CEditorObjectFactory::Deserialize(const json& j)
{
}
