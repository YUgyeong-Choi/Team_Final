#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)

class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, atomic<float>& fRatio);
	virtual ~CLoader() = default;

public:
	_bool isFinished() const {
		return m_isFinished;
	}

public:
	HRESULT Initialize(LEVEL eNextLevelID);
	HRESULT Loading();

	void Output_LoadingText() {
		SetWindowText(g_hWnd, m_szLoadingText);
	}
	

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	LEVEL						m_eNextLevelID = { LEVEL::END };
	_bool						m_isFinished = { false };
	CGameInstance*				m_pGameInstance = { nullptr };

private:
	HANDLE						m_hThread = {};
	CRITICAL_SECTION			m_CriticalSection = {};
	_tchar						m_szLoadingText[MAX_PATH] = {};

	atomic<float>&             m_fRatio;
public:
	HRESULT Loading_For_Logo();
	HRESULT Loading_For_Static(); // ��� ����ϴ� �͵� Logo���� ó������ ���� 
	HRESULT Loading_For_KRAT_CENTERAL_STATION();
	HRESULT Loading_For_KRAT_HOTEL();
	HRESULT Loading_For_DH();
	HRESULT Loading_For_JW();
	HRESULT Loading_For_GL();

#pragma region YW
	HRESULT Loading_For_YW();

	//<��>�� �ʿ��� �𵨵��� �ε��Ѵ�.
	HRESULT Loading_Models(_uint iLevelIndex, const _char* Map);
	HRESULT Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex);

	//�׺���̼��� �ε��Ѵ�.
	HRESULT Loading_Navigation(_uint iLevelIndex, const _char* Map, _bool bForTool = false);

	//�ʿ��� ��Į �ؽ��ĸ� �ε��Ѵ�.
	HRESULT Loading_Decal_Textures(_uint iLevelIndex, const _char* Map);

#pragma endregion
	
#pragma region GL
	HRESULT Loading_For_UI_Texture();
#pragma endregion

	HRESULT Loading_For_CY();
	HRESULT Loading_For_YG();

	static _bool m_bLoadStatic;
public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID, atomic<float>& fRatio);
	virtual void Free() override;
};

NS_END