#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CMapLoader final :public CBase
{
private:
	CMapLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMapLoader() = default;

public:
	HRESULT Initialize();
	_bool Check_MapLoadComplete();

public:
	/*
	 ����ƽ �޽� �ε�, ��ġ
	 �� ���� ���
	 �׺�, ��Į, �μ����� �޽�, ������, ��Ÿ������ ���...
	*/
	HRESULT Ready_Map_Async();

	HRESULT Load_Ready_Nav_All(_uint iLevelIndex);

	HRESULT Load_Ready_All_Etc(_uint iLevelIndex);

	HRESULT Ready_Etc(const _char* Map);

public:
	//���⼭ �ʿ� �ʿ��Ѱ͵� ��� �ε�(��, ��Į, �׺� ���...)
	HRESULT Load_Map(_uint iLevelIndex, const _char* Map);

	//<��>�� �ʿ��� �޽����� �ε��Ѵ�.
	HRESULT Loading_Meshs(_uint iLevelIndex, const _char* Map);
	HRESULT Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex);

	//�׺���̼��� �ε��Ѵ�.
	HRESULT Loading_Navigation(_uint iLevelIndex, const _char* Map/*, _bool bForTool = false*/);

	//�ʿ��� ��Į �ؽ��ĸ� �ε��Ѵ�.
	HRESULT Loading_Decal_Textures(_uint iLevelIndex, const _char* Map);

	//�μ��� �� �ִ� �޽��� �ε��Ѵ�.
	HRESULT Loading_Breakable(_uint iLevelIndex, const _char* Map);

public:
	//�� ��ȯ(�޽�, �׺�, ��Į ��...)
	HRESULT Ready_Map(_uint iLevelIndex, const _char* Map);

	//�޽� ��ȯ
	HRESULT Ready_Meshs(_uint iLevelIndex, const _char* Map);
	HRESULT Ready_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map);
	HRESULT Ready_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map);

	//�׺���̼� ��ȯ
	HRESULT Ready_Nav(const _wstring strLayerTag, _uint iLevelIndex, const _char* Map);

	//����ƽ ��Į�� ��ȯ�Ѵ�. (true�� �׽�Ʈ ��Į ��ȯ)
	HRESULT Ready_Static_Decal(_uint iLevelIndex, const _char* Map);

	//�μ��� �� �ִ� �޽��� ��ȯ�Ѵ�.
	HRESULT Ready_Breakable(_uint iLevelIndex, const _char* Map);

public:
	HRESULT Add_MapActor(const _char* Map); //�� ���� �߰�(�ݶ��̴� Ȱ��ȭ)


public:
	HRESULT Ready_Monster();
	HRESULT Ready_Monster(const _char* Map);//Ư�� ���� ���͸� ��ȯ�Ѵ�. �� ���� �׺���̼��� ������Ų��.

	HRESULT Ready_Stargazer();
	HRESULT Ready_Stargazer(const _char* Map);

	HRESULT Ready_ErgoItem();
	HRESULT Ready_ErgoItem(const _char* Map);

	HRESULT Ready_Breakable();
	HRESULT Ready_Breakable(const _char* Map);

private:
	vector<const _char*> m_Maps = { "HOTEL", "OUTER"/*FireEater*/ };

private:
	// �Ϸ�� �� �̸��� �����ϴ� ������ ���� ť
	std::queue<const char*> m_ReadyQueue;
	std::mutex m_QueueMutex;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance* m_pGameInstance = { nullptr };

public:
	static CMapLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
NS_END