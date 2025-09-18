#include "EntryPointLifecycle.h"

#include "WinMain.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/Version.h"
#include "BuildVersion.h"
#include "GeneratedVersion.h"

void InitializeEntryPoint(const EntryPointConfig& config) {
    ApplicationGraphicsBackend = config.graphicsBackend;

    DEBUG_INIT(DEBUG_FLAGS_DEFAULT);
    initMemoryManager();

    if (TheVersion != NULL) {
        delete TheVersion;
        TheVersion = NULL;
    }

    TheVersion = NEW Version;
    TheVersion->setVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
                           AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
                           AsciiString(__TIME__), AsciiString(__DATE__));
}

void ShutdownEntryPoint() {
    if (TheVersion != NULL) {
        delete TheVersion;
        TheVersion = NULL;
    }

    shutdownMemoryManager();
    DEBUG_SHUTDOWN();
}

EntryPointScope::EntryPointScope(const EntryPointConfig& config)
    : m_active(true) {
    InitializeEntryPoint(config);
}

EntryPointScope::~EntryPointScope() {
    if (m_active) {
        ShutdownEntryPoint();
    }
}

void EntryPointScope::dismiss() {
    if (m_active) {
        ShutdownEntryPoint();
        m_active = false;
    }
}

