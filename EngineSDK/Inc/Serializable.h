#pragma once
#include "Engine_Defines.h"
NS_BEGIN(Engine)
class ENGINE_DLL ISerializable
{
public:
	ISerializable() = default;
	virtual ~ISerializable() = default;
	
	virtual json Serialize() = 0;
	virtual void Deserialize(const json& j) = 0;


};
NS_END

