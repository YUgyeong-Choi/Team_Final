#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CSound_Core;
NS_END

NS_BEGIN(Client)

class CLevel_Logo final : public CLevel
{
private:
	CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Logo() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Video();

	HRESULT Ready_Menu();
	// ��ü�� �ٽ� ���� ����
	HRESULT Ready_Guide();

	void Check_Button();
	void Interation_Button(_int& iIndex);


private:
	class CUI_Video* m_pMainUI = {};

	_bool			 m_isReady = { false };
	LEVEL			 m_eNextLevel = {LEVEL::END};

	_float           m_fDelay = 0.3f;
	

	// �̷� ������ ��ü ���� �κ��丮�� �ٸ� ȭ�� �����ϸ� �ɵ�? 
	// �̰Ŵ� �����ؼ� �������θ�
	_int			 m_iButtonIndex = {0};
	vector <class CUI_Button* > m_pButtons = {};
	class CDynamic_UI*	m_pSelectUI = {nullptr};

	_bool m_isOpen = { false };

	class CUI_Container* m_pTeammate = {nullptr};
	class CGameObject* m_pGuideBack = { nullptr };


	// �̰Ŵ� ���߿� ����
	class CUI_Guide* m_pGuide = { nullptr };
private: /* [ ���� ] */
	CSound_Core* m_pBGM = { nullptr };

public:
	static CLevel_Logo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END