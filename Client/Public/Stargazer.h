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
	// ��ȭ �� ������ �̰ɷ� �ֱ�
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


	//���� ��ư ���� ���� ui���� Ư�� �ൿ�� ���ϰ� �ٽ� ���ƿ���, ��ư�̶� ��ũ��Ʈ �ٽ� Ű��, 
	//���� ui���� ���� �θ��� �ɵ�?
	void Script_Activate();

	// Ư�� �ൿ�� �ϸ�, ��ư�̶� ��ũ��Ʈ�� �����.
	void Delete_Script();

	void Teleport_Stargazer(STARGAZER_TAG eTag);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

private:
	void LoadAnimDataFromJson(CModel* pModel, CAnimator* pAnimator);
	void Find_Player();
	void Register_Events();

	// �ʿ��� �����͸� �ε���
	void LoadScriptData();
	// Ű �Է¿� ���� �����ϴ� ��ư�� �ٲ�
	void Update_Button();
	// ������ ��ư�� ������ �ִ� �̺�Ʈ�� ���� ���� ui�� ���ϰ� ������ش�.
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

private:    /* [ ������Ʈ ] */
	CModel* m_pModelCom[ENUM_CLASS(STARGAZER_STATE::END)] = {nullptr};
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator[ENUM_CLASS(STARGAZER_STATE::END)] = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };

	// ��ȣ�ۿ� �뵵
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	

	// ��ȭ�� ������ ���� �߰��ϱ�, npcó��
	vector<TALKDATA> m_eScriptDatas = { };

	_int m_iScriptIndex = {  };

	_bool m_bTalkActive = { false };
	_bool m_bAutoTalk = {};

	// ��ȭ�� ������ ��ũ��Ʈ�� ����.
	class CUI_Script_StarGazer* m_pScript = { nullptr };
	// ��ũ��Ʈ�� ���� ��� ��ư��, ���� �� �ִ� ��ư �ε���
	vector<class CUI_Button_Script*> m_pSelectButtons;
	_int m_iSelectButtonIndex = {-1};

	vector<string> m_ButtonEvents = {};

	_bool m_isCollison = {};
	// ���� �б⸦ ���� bool�� ���� �Ϸ��� ��
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