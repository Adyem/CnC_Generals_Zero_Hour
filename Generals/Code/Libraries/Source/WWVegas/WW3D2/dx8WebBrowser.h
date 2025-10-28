#ifndef GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WW3D2_DX8WEBBROWSER_H
#define GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WW3D2_DX8WEBBROWSER_H

#ifdef _UNIX
#include "Compat/Windows/windows_compat.h"

using cnc::windows::LONG;
using cnc::windows::LPDISPATCH;

#ifndef ENABLE_EMBEDDED_BROWSER
#define ENABLE_EMBEDDED_BROWSER 0
#endif

#ifndef BROWSEROPTION_SCROLLBARS
#define BROWSEROPTION_SCROLLBARS 0x0001
#endif
#ifndef BROWSEROPTION_3DBORDER
#define BROWSEROPTION_3DBORDER 0x0002
#endif

class DX8WebBrowser {
public:
    static bool Initialize(const char* /*badpageurl*/ = nullptr,
                           const char* /*loadingpageurl*/ = nullptr,
                           const char* /*mousefilename*/ = nullptr,
                           const char* /*mousebusyfilename*/ = nullptr)
    {
        return false;
    }

    static void Shutdown() {}
    static void Update() {}
    static void Render(int /*backbufferindex*/) {}
    static void CreateBrowser(const char* /*browsername*/, const char* /*url*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/,
                              int /*updateticks*/ = 0, LONG /*options*/ = BROWSEROPTION_SCROLLBARS | BROWSEROPTION_3DBORDER,
                              LPDISPATCH /*gamedispatch*/ = nullptr)
    {
    }
    static void DestroyBrowser(const char* /*browsername*/) {}
    static bool Is_Browser_Open(const char* /*browsername*/) { return false; }
    static void Navigate(const char* /*browsername*/, const char* /*url*/) {}
};
#else
#include "dx8webbrowser.h"
#endif

#endif // GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WW3D2_DX8WEBBROWSER_H
