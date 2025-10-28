
#include "GraphicsBackend.h"

struct EntryPointConfig {
    GraphicsBackend graphicsBackend;
};

void InitializeEntryPoint(const EntryPointConfig& config);
void ShutdownEntryPoint();

class EntryPointScope {
public:
    explicit EntryPointScope(const EntryPointConfig& config);
    ~EntryPointScope();

    EntryPointScope(const EntryPointScope&) = delete;
    EntryPointScope& operator=(const EntryPointScope&) = delete;

    void dismiss();

private:
    bool m_active;
};

