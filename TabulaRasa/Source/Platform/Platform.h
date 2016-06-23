#pragma once

#ifdef _WIN32
#include "WindowsPlatformWindow.h"
#include "WindowsPlatformFilesystem.h"
#elif __APPLE__
#include "OSXPlatformWindow.h"
#include "OSXPlatformFilesystem.h"
#endif 