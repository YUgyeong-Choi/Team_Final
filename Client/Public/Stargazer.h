#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CStargazer : public CGameObject
{
public:
	enum class STARGAZER_STATE { DESTROYED, FUNCTIONAL, END };
	// 대화 가 있으면 이걸로 넣기
	typedef struct tagTalkDatas
	{
		string strSoundTag;
		string strSpeaker;
		string strSoundText;
	}TALKDATA;

public:
	typedef struct tagStargazerDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
		STARGAZER_TAG eStargazerTag = { STARGAZER_TAG::END };
	}STARGAZER_DESC;

	STARGAZER_STATE Get_State() { return m_eState; }

protected:
	CStargazer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStargazer(const CStargazer& Prototype);
	virtual ~CStargazer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void LoadAnimDataFromJson(CModel* pModel, CAnimator* pAnimator);
	void Find_Player();
	void Register_Events();
	void Teleport_Stargazer(STARGAZER_TAG eTag);
	_bool Check_Player_Close();

	// 필요한 데이터를 로드함
	void LoadScriptData();

private:


private:
	STARGAZER_STATE m_eState = { STARGAZER_STATE::END };
	

private:
	class CPlayer* m_pPlayer = { nullptr };

private:
	STARGAZER_TAG m_eStargazerTag = { STARGAZER_TAG::END };

private:    /* [ 컴포넌트 ] */
	CModel* m_pModelCom[ENUM_CLASS(STARGAZER_STATE::END)] = {nullptr};
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator[ENUM_CLASS(STARGAZER_STATE::END)] = { nullptr };

	// 대화가 있으면 따로 추가하기, npc처럼
	vector<TALKDATA> m_eScriptDatas = { };

	_int m_iScriptIndex = {  };

	_bool m_bTalkActive = { false };
	_bool m_bAutoTalk = {};

	class CUI_Script_StarGazer* m_pScript = { nullptr };
	vector<class CUI_Button_Script*> m_pSelectButtons;

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();

public:
	static CStargazer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END