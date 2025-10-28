/*
**      Command & Conquer Generals(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2001-2003 Electronic Arts Inc.
//
////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
*
* NAME
*     $Archive:  $
*
* DESCRIPTION
*     Web Browser
*
* PROGRAMMER
*     Bryan Cleveland
*     $Author:  $
*
* VERSION INFO
*     $Revision:  $
*     $Modtime:  $
*
******************************************************************************/


#ifndef __WEBBROWSER_H__
#define __WEBBROWSER_H__

#include "Common/SubsystemInterface.h"
#include <Common/GameMemory.h>

class GameWindow;

#ifdef _WIN32

#include <atlbase.h>
#include <windows.h>
#include "EABrowserDispatch/BrowserDispatch.h"
#include "FEBDispatch.h"

class WebBrowserURL : public MemoryPoolObject
{
        MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WebBrowserURL, "WebBrowserURL" )

public:

        WebBrowserURL();
        // virtual destructor prototype defined by memory pool object

        const FieldParse *getFieldParse( void ) const { return m_URLFieldParseTable; }

        AsciiString m_tag;
        AsciiString m_url;

        WebBrowserURL *m_next;

        static const FieldParse m_URLFieldParseTable[];         ///< the parse table for INI definition

};



class WebBrowser :
                public FEBDispatch<WebBrowser, IBrowserDispatch, &IID_IBrowserDispatch>,
                public SubsystemInterface
        {
        public:
                void init( void );
                void reset( void );
                void update( void );

                // Create an instance of the embedded browser for Dune Emperor.
                virtual Bool createBrowserWindow(char *tag, GameWindow *win) = 0;
                virtual void closeBrowserWindow(GameWindow *win) = 0;

                WebBrowserURL *makeNewURL(AsciiString tag);
                WebBrowserURL *findURL(AsciiString tag);

        protected:
                // Protected to prevent direct construction via new, use CreateInstance() instead.
                WebBrowser();
                virtual ~WebBrowser();

                // Protected to prevent copy and assignment
                WebBrowser(const WebBrowser&);
                const WebBrowser& operator=(const WebBrowser&);

//              Bool RetrievePageURL(const char* page, char* url, int size);
//              Bool RetrieveHTMLPath(char* path, int size);

        protected:
                ULONG mRefCount;
                WebBrowserURL *m_urlList;

        //---------------------------------------------------------------------------
        // IUnknown methods
        //---------------------------------------------------------------------------
        protected:
                HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
                ULONG STDMETHODCALLTYPE AddRef(void);
                ULONG STDMETHODCALLTYPE Release(void);

        //---------------------------------------------------------------------------
        // IBrowserDispatch methods
        //---------------------------------------------------------------------------
        public:
                STDMETHOD(TestMethod)(Int num1);
        };

extern CComObject<WebBrowser> *TheWebBrowser;

#else  // !_WIN32

class WebBrowserURL : public MemoryPoolObject
{
        MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WebBrowserURL, "WebBrowserURL" )

public:
        WebBrowserURL() : m_next(NULL) {}

        const FieldParse *getFieldParse( void ) const { return NULL; }

        AsciiString m_tag;
        AsciiString m_url;
        WebBrowserURL *m_next;
};

class WebBrowser : public SubsystemInterface
{
public:
        void init( void ) {}
        void reset( void ) {}
        void update( void ) {}

        virtual Bool createBrowserWindow(char *, GameWindow *) { return FALSE; }
        virtual void closeBrowserWindow(GameWindow *) {}

        WebBrowserURL *makeNewURL(AsciiString) { return NULL; }
        WebBrowserURL *findURL(AsciiString) { return NULL; }
};

extern WebBrowser *TheWebBrowser;

#endif  // _WIN32

#endif // __WEBBROWSER_H__
