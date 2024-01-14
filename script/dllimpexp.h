#pragma once

#ifdef GUI_EXPORTS
#define DLLSCRIPT __declspec(dllexport)
#else
#define DLLSCRIPT __declspec(dllimport)
#endif