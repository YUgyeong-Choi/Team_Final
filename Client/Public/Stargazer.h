#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CSoundController;
class CPhysXStaticActor;
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
		/*_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);*/
		STARGAZER_TAG eStargazerTag = { STARGAZER_TAG::END };
	}STARGAZER_DESC;

	STARGAZER_STATE Get_State() { return m_eState; }
	STARGAZER_TAG Get_Tag() { return m_eStargazerTag; }

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


	//이제 버튼 에서 만든 ui에서 특정 행동을 안하고 다시 돌아오면, 버튼이랑 스크립트 다시 키기, 
	//만든 ui에서 이제 부르면 될듯?
	void Script_Activate();

	// 특정 행동을 하면, 버튼이랑 스크립트를 지운다.
	void Delete_Script();

	void Teleport_Stargazer(STARGAZER_TAG eTag);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

private:
	void LoadAnimDataFromJson(CModel* pModel, CAnimator* pAnimator);
	void Find_Player();
	void Register_Events();

	// 필요한 데이터를 로드함
	void LoadScriptData();
	// 키 입력에 따라 선택하는 버튼을 바꿈
	void Update_Button();
	// 선택한 버튼이 가지고 있는 이벤트에 따라 만들 ui를 정하고 만들어준다.
	void Button_Interaction();

	HRESULT Ready_Script();

	HRESULT Ready_EffectSet();


private:
	class CStargazerEffect* m_pEffectSet = { nullptr };

private:
	STARGAZER_STATE m_eState = { STARGAZER_STATE::END };
	
private:
	_float m_fTeleportTime = {};

private:
	class CPlayer* m_pPlayer = { nullptr };

private:
	STARGAZER_TAG m_eStargazerTag = { STARGAZER_TAG::END };

private:    /* [ 컴포넌트 ] */
	CModel* m_pModelCom[ENUM_CLASS(STARGAZER_STATE::END)] = {nullptr};
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator[ENUM_CLASS(STARGAZER_STATE::END)] = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };

	// 상호작용 용도
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	

	// 대화가 있으면 따로 추가하기, npc처럼
	vector<TALKDATA> m_eScriptDatas = { };

	_int m_iScriptIndex = {  };

	_bool m_bTalkActive = { false };
	_bool m_bAutoTalk = {};

	// 대화가 없으면 스크립트를 띄운다.
	class CUI_Script_StarGazer* m_pScript = { nullptr };
	// 스크립트와 같이 띄울 버튼과, 선택 되 있는 버튼 인덱스
	vector<class CUI_Button_Script*> m_pSelectButtons;
	_int m_iSelectButtonIndex = {-1};

	vector<string> m_ButtonEvents = {};

	_bool m_isCollison = {};
	// 동작 분기를 위해 bool로 구분 하려고 함
	_bool m_bDoOnce = {};
	_bool m_bChange = {};
	_bool m_bUseScript = {};

	_bool m_bUseTeleport = {};

	_bool m_bUseOtherUI = {};
	

	//vector<CUIObject*> m_
	class CUI_Guide* m_pGuide = {nullptr};
	_bool m_isMakeGuide = {};

	
	_bool m_bIsRotatingToStargazer = false;
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