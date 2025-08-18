
#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
class CNavigation;
NS_END

NS_BEGIN(Client)

class CUnit;

class CAStar_Manager final : public CBase
{
    DECLARE_SINGLETON(CAStar_Manager)
private:
    CAStar_Manager();
    virtual ~CAStar_Manager() = default;

public:
    virtual HRESULT Initialize(LEVEL eLevel); // �� ���� �׺� ã�ƿͼ� �ھ�
    virtual HRESULT Priority_Update(_float fTimeDelta); // ��� �׾����� üũ��
    virtual HRESULT Update(_float fTimeDelta); // ��� ã��
    virtual HRESULT Late_Update(_float fTimeDelta);
    virtual HRESULT Render();
public:

    HRESULT Replace_NaviMesh(); // ���� �ٲ�� �� ������ �´� �׺� ã�Ƽ� �ھ�
 
    // �����ִ� ���� false
    _bool Find_Route();

    // ��Ʈ�� �� �� �ε巴��? �� �߾��� ���� �ʾƵ� �Ǹ�  �Ȱ�����
    void Adjust_Route();

    


private:
    CGameInstance* m_pGameInstance = { nullptr };

    CNavigation* m_pNaviCom = { nullptr };

public:
    virtual void Free() override;
};

NS_END

