#pragma once
#include "YWTool.h"

#include "Client_Defines.h"

NS_BEGIN(Client)

class CMonsterTool final : public CYWTool
{
private:
	CMonsterTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonsterTool(const CMonsterTool& Prototype);
	virtual ~CMonsterTool() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;
	virtual HRESULT	Render_ImGui() override;
	virtual HRESULT Save(const _char* Map) override; //void Arg로 바꿀지는 아직 고민
	virtual HRESULT Load(const _char* Map) override;

private:
	//툴 조작
	void Control(_float fTimeDelta);
private:
	void Picking();
	void Focus();
	void SnapTo();
	void Duplicate();

private:
	void Clear();
	HRESULT Ready_Texture(const _char* Map);

private:
	//몬스터 소환
	HRESULT Spawn_MonsterToolObject();

	//삭제
	void Delete_FocusObject();
private:
	void Render_Detail();
	void Detail_Transform();
	void Detail_SpawnType();

private:
#pragma region 몬스터 종류
	enum class Monster {
		Buttler_Basic,
		Buttler_Train,
		Buttler_Range,
		WatchDog,
		Elite_Police,
		FestivalLeader,
		FireEater,
		END
	};

	const string m_strMonsters[ENUM_CLASS(Monster::END)] = {
	"Buttler_Basic",
	"Buttler_Train",
	"Buttler_Range",
	"WatchDog",
	"Elite_Police",
	"FestivalLeader",
	"FireEater"
	};

#pragma endregion

	//enum class SPAWN_TYPE {IDLE, PATROL, STAND, SIT, END};

#pragma region 스폰 타입
	const string m_strSpawnTypes[static_cast<_int>(SPAWN_TYPE::END)] = {
	"IDLE",
	"PATROL",
	"STAND",
	"SIT",
	};

#pragma endregion
	_int m_iMonsterIndex = static_cast<_int>(Monster::Buttler_Train);

private:
	ImGuizmo::OPERATION m_currentOperation = { ImGuizmo::TRANSLATE };

private:
	class CMonsterToolObject* m_pFocusObject = { nullptr };

//private:
//	_int m_iID = { -1 }; //-1, -2 몬스터 아이디는 음수로하자

public:
	static CMonsterTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END