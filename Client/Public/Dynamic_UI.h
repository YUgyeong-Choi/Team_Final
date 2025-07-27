#pragma once

#include "Client_Defines.h"
#include "UIObject.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END


NS_BEGIN(Client)

class CDynamic_UI : public CUIObject
{
public:
	typedef struct tagDynamicUIDesc : public CUIObject::UIOBJECT_DESC
	{
		_bool isFromTool = {false};
		// 
		_int iPassIndex = { 0 };
		_int iTextureIndex = { 0 };
		_wstring strTextureTag;
		_wstring strProtoTag; // ���� ��ӹ����� ä���
		_float fDuration = {0.016f}; // �� �� �������� �Ұ���
		_float4 vColor = { 1.f,1.f,1.f,1.f };


		vector<UI_FEATRE_DESC*> FeatureDescs;

	}DYNAMIC_UI_DESC;


	_wstring& Get_StrTextureTag() { return m_strTextureTag; }

	DYNAMIC_UI_DESC Get_Desc();
	

	

	vector<class CUI_Feature*>& Get_Features();

protected:
	CDynamic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDynamic_UI(const CDynamic_UI& Prototype);
	virtual ~CDynamic_UI() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Update_UI_From_Tool(_int& iCurrentFrame);
	void Update_UI_From_Tool(DYNAMIC_UI_DESC eDesc);
	void Reset() ;

	HRESULT Ready_Components(const wstring& strTextureTag);
	HRESULT Bind_ShaderResources();

	HRESULT Add_Feature( _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg);

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	vector<class CUI_Feature*> m_pUIFeatures = {};

private:

	_bool    m_isFromTool = {false};

	_wstring m_strTextureTag = {};

	_int   m_iPassIndex = {};
	_int   m_iTextureIndex = {};

	_int   m_iCurrentFrame = {};

	_wstring    m_strProtoTag = {TEXT("Prototype_GameObject_Dynamic_UI")};

	// �ϴ� �ð�����, �ٸ� ������ �����ؾ� �Ǹ� ��ӹ޾Ƽ� �ٸ� ������ �������� �����ϸ� �ɵ�?
	_float m_fDuration = {0.016f};
	_float m_fElapsedTime = {};

private:
	

public:
	static CDynamic_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END