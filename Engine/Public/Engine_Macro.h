#pragma once

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define NS_BEGIN(NAMESPACE) namespace NAMESPACE { 
#define NS_END }

#define IFILEDIALOG ImGuiFileDialog::Instance()

#define ENUM_CLASS(ENUM) static_cast<unsigned int>(ENUM)

#define SQUARE(X)   (X)*(X)
#define G_FORCE     9.80665f
#define PERCENT(X)	((X) - rand()%100 > 0)
#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))
#define FLT_EQUAL(a, b) (fabsf((a) - (b)) < FLT_EPSILON)

/* RGBA Color Float4 */
#define RGBA_WHITE		_float4{1.f, 1.f, 1.f, 1.f}
#define RGBA_BLACK		_float4{0.f, 0.f, 0.f, 1.f}
#define RGBA_RED		_float4{1.f, 0.f, 0.f, 1.f}
#define RGBA_GREEN		_float4{0.f, 1.f, 0.f, 1.f}
#define RGBA_BLUE		_float4{0.f, 0.f, 1.f, 1.f}
#define RGBA_YELLOW		_float4{1.f, 1.f, 0.f, 1.f}
#define RGBA_MAGENTA	_float4{1.f, 0.f, 1.f, 1.f}
#define RGBA_CYAN		_float4{0.f, 1.f, 1.f, 1.f}

#define KEY_DOWN        m_pGameInstance->Key_Down
#define KEY_PRESSING    m_pGameInstance->Key_Pressing
#define KEY_UP            m_pGameInstance->Key_Up

#define MOUSE_DOWN        m_pGameInstance->Mouse_Down
#define MOUSE_PRESSING    m_pGameInstance->Mouse_Pressing
#define MOUSE_UP        m_pGameInstance->Mouse_Up

#ifdef ENGINE_EXPORTS
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)
#endif

#define MSG_BOX(message)								\
::MessageBox(nullptr, TEXT(message), L"error", MB_OK)

#define NO_COPY(ClassName)								\
ClassName(const ClassName&) = delete;					\
ClassName& operator=(const ClassName&) = delete;

#define DECLARE_SINGLETON(ClassName)					\
		NO_COPY(ClassName)								\
public :												\
	static ClassName* Get_Instance();					\
	static unsigned int Destroy_Instance();				\
private:												\
	static ClassName* m_pInstance;

#define IMPLEMENT_SINGLETON(ClassName)					\
ClassName* ClassName::m_pInstance = nullptr;			\
ClassName* ClassName::Get_Instance()					\
{														\
	if (nullptr == m_pInstance)							\
		m_pInstance = new ClassName;					\
	return m_pInstance;									\
}														\
unsigned int ClassName::Destroy_Instance()				\
{														\
	unsigned int iRefCnt = {};							\
	if(nullptr != m_pInstance)							\
	{													\
		iRefCnt = m_pInstance->Release();				\
		if(0 == iRefCnt)								\
			m_pInstance = nullptr;						\
	}													\
	return iRefCnt;										\
}

#define ADD_SOUND(Name,Path)																						\
if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Sound_"))+L###Name ,	\
CSoundController::Create(Path))))																					\
return E_FAIL

#define ADD_SOUND_EX(Name,Path,...)																					\
if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Sound_"))+L###Name ,	\
CSoundController::Create(Path, __VA_ARGS__))))																		\
return E_FAIL

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#define MSG(desc) message(__FILE__"("STR(__LINE__)"):"#desc)
#define FixLater(desc) __pragma(MSG(desc))
