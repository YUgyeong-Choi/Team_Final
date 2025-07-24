#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#include <MySequence.h>
#undef USE_IMGUI

NS_BEGIN(Engine)
class CModel;
class CAnimation;
class CAnimator;
class CTransform;
class CAnimController;
class CShader;
NS_END

NS_BEGIN(Client)

class CAnimTool final : public CGameObject
{
	enum class Operation { TRANSLATE, ROTATE, SCALE };
private:
	CAnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimTool(const CAnimTool& Prototype);
	virtual ~CAnimTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Render_AnimationSequence();
	HRESULT Render_AnimStatesByNode();
	HRESULT Render_Loaded_Models();
	HRESULT Render_Load_Model();

	HRESULT Bind_Shader();

	void UpdateCurrentModel(_float fTimeDelta);
	void SelectAnimation();
	void CreateModel(const string& fileName,const string& filePath);

	void Manipulate(
		Operation op,
		const _float snapT[3] = nullptr,   // �̵��� ���� (unit)
		const _float snapR[3] = nullptr,   // ȸ���� ���� (deg)
		const _float snapS[3] = nullptr    // �����Ͽ� ���� (factor)
	);

private:
	CAnimator* m_pCurAnimator = nullptr; // ���� ���� �ִϸ�����
	CAnimation* m_pCurAnimation = nullptr; // ���� �ִϸ��̼�
	CModel* m_pCurModel = nullptr; // ���� ��
	string m_ModelFilePath;
	class CGameInstance* m_pGameInstance = nullptr;


	CShader* m_pAnimShader = nullptr; // �ִϸ��̼� ���̴�
	unordered_map<string, class CModel*> m_LoadedModels; // �ε�� �𵨵�
	unordered_map<string, class CAnimator*> m_LoadedAnimators; // ���� ���� �ִϸ�����
	unordered_map<string, vector<class CAnimation*>> m_LoadedAnimations; // ���� ���� �ִϸ��̼ǵ�
	string m_stSelectedModelName; // ���� ���õ� �� �̸�
	// �� �ҷ����� ��
	_bool m_bActiveLoadModel = false;

	// �𵨿� ���� ��Ʈ����
	_float4x4 m_ModelWorldMatrix{};
	CTransform* m_pTransformComForModel = nullptr; // ���� Ʈ������ ������Ʈ

	_bool m_bRenerLevel = false;

	_bool m_bUseAnimSequence = false; // �ִϸ��̼� ������ ��� Ȱ��ȭ

	// �ִϸ��̼� ��������

	CMySequence* m_pMySequence = nullptr;
	_bool m_bIsPlaying = false;
	_bool  m_bUseSequence = false;   // ������ ��� Ȱ��ȭ �÷���
	_int   m_iSequenceFrame = 0;       // �������� ������ ���� ������
	_int   m_iFirstFrame = 0;       // ������ ���� ù ������ (�ʿ��ϴٸ�)
	_int   m_playSpeed = 1;       // ��� �ӵ�
	_float m_fTimeAcc = 0.0f;
	class CEventMag* m_pEventMag = nullptr; // �̺�Ʈ �Ŵ���

public:
	static CAnimTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END