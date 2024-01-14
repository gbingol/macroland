#pragma once

#ifdef GUI_EXPORTS
#define DLLGRID __declspec(dllexport)
#else
#define DLLGRID __declspec(dllimport)
#endif