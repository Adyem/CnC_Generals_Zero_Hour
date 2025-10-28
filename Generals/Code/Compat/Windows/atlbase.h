
#ifndef _WIN32

#include "windows_compat.h"

namespace cnc::windows
{
class CComModule
{
public:
    void Init(void*, HINSTANCE) {}
    void Term() {}
};

class CComSingleThreadModel
{
};

template <typename ThreadModel = CComSingleThreadModel>
class CComObjectRootEx
{
public:
    using _ThreadModel = ThreadModel;
};

template <typename Base>
class CComObject : public Base
{
public:
    using _BaseClass = Base;
};

template <typename Interface>
class CComPtr
{
public:
    CComPtr() : m_ptr(nullptr) {}
    explicit CComPtr(Interface* ptr) : m_ptr(ptr) {}

    ~CComPtr()
    {
        if (m_ptr)
        {
            m_ptr->Release();
        }
    }

    Interface* operator->() const { return m_ptr; }
    Interface** operator&() { return &m_ptr; }
    Interface* get() const { return m_ptr; }
    void reset(Interface* ptr = nullptr)
    {
        if (m_ptr)
        {
            m_ptr->Release();
        }
        m_ptr = ptr;
    }

private:
    Interface* m_ptr;
};

template <typename Interface>
class CComQIPtr
{
public:
    CComQIPtr() : m_ptr(nullptr) {}
    explicit CComQIPtr(Interface* ptr) : m_ptr(ptr) {}
    explicit CComQIPtr(IUnknown* unknown)
        : m_ptr(unknown ? static_cast<Interface*>(unknown) : nullptr)
    {
    }

    Interface* operator->() const { return m_ptr; }
    operator Interface*() const { return m_ptr; }

private:
    Interface* m_ptr;
};

template <class T, const IID* piid = nullptr, const CLSID* pclsid = nullptr>
class CComCoClass
{
public:
    using _CreatorClass = T;
};

} // namespace cnc::windows

using cnc::windows::CComModule;
using cnc::windows::CComObject;
using cnc::windows::CComObjectRootEx;
using cnc::windows::CComSingleThreadModel;
using cnc::windows::CComCoClass;
using cnc::windows::CComPtr;
using cnc::windows::CComQIPtr;

#define BEGIN_COM_MAP(x)
#define COM_INTERFACE_ENTRY(x)
#define COM_INTERFACE_ENTRY_AGGREGATE(iid, ptr)
#define END_COM_MAP()

#endif // !_WIN32

