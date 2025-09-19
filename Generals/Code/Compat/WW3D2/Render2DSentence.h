#pragma once

// Minimal placeholder for the legacy WW3D2 text rendering helper.  The real
// implementation depends on Direct3D and other Windows-specific facilities,
// which are not available in the cross-platform build.  The SFML bootstrap
// only requires the type declarations in order to compile the headers that
// reference Render2DSentenceClass.

#ifndef _WIN32

class Render2DSentenceClass
{
public:
        Render2DSentenceClass() = default;
        ~Render2DSentenceClass() = default;

        void Reset() {}
        void Release() {}
};

#endif  // !_WIN32

