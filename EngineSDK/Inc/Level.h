#pragma once

#include "Base.h"

/* 클라 개발자가 만들 레벨클래스들의 부모가 되는 클래스다. */

NS_BEGIN(Engine)

class ENGINE_DLL CLevel abstract : public CBase
{
protected:
	CLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel() = default;

public:
	virtual HRESULT Initialize();	
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Reset() { return S_OK; }

	void Start_BGM(string soundName);
	void Change_BGM(string soundName);
	void Change_BGM(string soundNoLoopName, string soundName);
	
	class CSound_Core* Get_BGM() { return m_pBGM; }
	void Stop_BGM();
protected:
	void HoldMouse();

	/* 현재 BGM은 1~0으로, 다음 BGM은 0~1로*/
	void Update_ChangeBGM(_float fTimeDelta);
protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };

	class CSound_Core* m_pBGM = { nullptr };
	
	string m_CurBGMName = "";
	string m_BGMNext = "";
	// NoLoop BGM일때 이게 끝나면 재생할 BGM
	string m_BGMQueued = "";

	_float m_fBGMVolume = 1.f;


	_bool m_bBGMToZero = false;
	_bool m_bBGMToVolume = false;
public:	
	virtual void Free() override;

};

NS_END