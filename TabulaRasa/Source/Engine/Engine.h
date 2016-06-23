#pragma once

#ifdef _WIN32
#define FORCEINLINE __forceinline
#elif __APPLE__
#define FORCEINLINE inline
#endif