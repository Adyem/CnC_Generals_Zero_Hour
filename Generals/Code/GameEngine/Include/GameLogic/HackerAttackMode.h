
#ifndef _WIN32

// Placeholder hacker attack mode definitions for non-Windows builds.
// The original enumeration is currently unavailable in the open-source
// drop; these values provide a concrete type so code that references
// HackerAttackMode can compile. They should be revisited once the
// definitive enumeration is restored.
enum HackerAttackMode
{
    HACKER_ATTACKMODE_NONE = 0,
    HACKER_ATTACKMODE_COUNT
};

#endif  // !_WIN32
