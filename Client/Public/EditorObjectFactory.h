#pragma once
#include "Base.h"
#include "Serializable.h"
#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)
class CEditorObjectFactory : public CBase, public ISerializable
{
	using Creator = function<CGameObject* (void* pArg)>;
private:
	CEditorObjectFactory(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditorObjectFactory(const CEditorObjectFactory& Prototype) = delete;
	virtual ~CEditorObjectFactory() = default;

public:
	template<class TObject>
	HRESULT RegisterObject(const _wstring& strObjectTag, void* pArg = nullptr)
	{
		if(pArg)
			m_ObjectArgs[strObjectTag] = pArg; // ��ü ���� �� ���� �����
		m_ObjectCreators[strObjectTag] = [this](void* pArg) -> CGameObject*
			{
				return NewObject<TObject>(pArg); // �Լ� ������ ���
			};
		return S_OK;
	}
	CGameObject* CreateObject(const wstring& strObjectTag)
	{
		auto iter = m_ObjectCreators.find(strObjectTag);
		if (iter == m_ObjectCreators.end())
		{

			_wstring objectTag = L"Failed to Find Object + " + strObjectTag;
			MessageBox(nullptr, objectTag.c_str(), L"error", MB_OK);
			return nullptr;
		}
		void* pArg = nullptr; // ���� �ʱ�ȭ
		if (m_ObjectArgs.find(strObjectTag) != m_ObjectArgs.end())
			pArg = m_ObjectArgs[strObjectTag]; // ���� ��������
		return iter->second(pArg);
	}
private:
	template<class TObject>
	TObject* NewObject(void* pArg = nullptr) // �Լ� ������
	{
		TObject* pInstance = TObject::Create(m_pDevice, m_pContext);
		if (FAILED(pInstance->Initialize(pArg)))
		{
			_wstring objectTag = L"Failed to Initialized Object + " + StringToWString(typeid(TObject).name());
			MessageBox(nullptr, objectTag.c_str(), L"error", MB_OK);
			return nullptr;
		}
		return pInstance;
	}

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	unordered_map<_wstring, Creator> m_ObjectCreators;
	unordered_map<_wstring, void* > m_ObjectArgs; // ��ü ���� �� ���� �����
public:
	static CEditorObjectFactory* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

	// ISerializable��(��) ���� ��ӵ�
	json Serialize() override;
	void Deserialize(const json& j) override;
};
NS_END

