#pragma once

#include "VIBuffer_Point_Instance.h"
#include "VIBuffer_Rect_Instance.h"
#include "VIBuffer_Terrain.h"
#include "VIBuffer_Cell.h"
#include "VIBuffer_Cube.h"
#include "VIBuffer_Rect.h"
#include "Navigation.h"
#include "Transform.h"
#include "PipeLine.h"
#include "Collider.h"
#include "Texture.h"
#include "Shader.h"
#include "Model.h"

#include "PhysXStaticActor.h"
#include "PhysXDynamicActor.h"
#include "PhysXController.h"

#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"

#include "SoundController.h"
#include "Sound_Core.h"

/* 1. ������ü�� �����Ѵ�. */
/* 1_1. ������ü�� Ÿ��( CGameObject, CComponent )�� ũ�� ������ ���� �ʴ´�. */
/* 1_2. �����Ҷ� ������( + ��緹��) �� �����Ͽ� �����Ұž�. */

NS_BEGIN(Engine)

class CPrototype_Manager final : public CBase
{
private:
	CPrototype_Manager();
	virtual ~CPrototype_Manager() = default;

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, class CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE ePrototypeType, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg);
	void Clear(_uint iLevelIndex);

	//������Ÿ���� �����ϴ��� Ȯ���ϱ� ���� public���� �÷�����
	class CBase* Find_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);

	const map<const _wstring, class CBase*>* Get_Prototypes() { return m_pPrototypes; }
private:
	_uint										m_iNumLevels = {};
	map<const _wstring, class CBase*>*			m_pPrototypes = { nullptr };
	typedef map<const _wstring, class CBase*>	PROTOTYPES;


public:
	static CPrototype_Manager* Create(_uint iNumLevels);
	virtual void Free() override;
};

NS_END