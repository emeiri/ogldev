#pragma once


#ifdef DEMOLITION_EXPORTS
#define DEMOLITION_API __declspec(dllexport)
#else
#define DEMOLITION_API __declspec(dllimport)
#endif