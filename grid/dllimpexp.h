#pragma once

#ifdef GRIDEXPORTS
#define DLLGRID __declspec(dllexport)
#else
#define DLLGRID __declspec(dllimport)
#endif