
#include <cstddef>

using persisttype_t = int;

constexpr persisttype_t pd_public_rw = 0;
constexpr int GE_NOERROR = 0;

extern char gcd_gamename[64];
extern char gcd_secret_key[64];

using persistAuthCallback = void(*)(int localid, int profileid, int authenticated, char* errmsg, void* instance);
using persistGetCallback = void(*)(int localid, int profileid, persisttype_t type, int index, int success, char* data, int len, void* instance);
using persistSetCallback = void(*)(int localid, int profileid, persisttype_t type, int index, int success, void* instance);

int InitStatsConnection(int productID);
void CloseStatsConnection();
bool IsStatsConnected();
void PersistThink();

const char* GetChallenge(const char* seed);
void GenerateAuth(const char* challenge, const char* password, char out[33]);

void PreAuthenticatePlayerPM(int localid, int profileid, const char* validate, persistAuthCallback callback, void* instance);

void GetPersistDataValues(int localid,
                          int profileid,
                          persisttype_t type,
                          int index,
                          const char* keys,
                          persistGetCallback callback,
                          void* instance);

void SetPersistDataValues(int localid,
                          int profileid,
                          persisttype_t type,
                          int index,
                          char* values,
                          persistSetCallback callback,
                          void* instance);
