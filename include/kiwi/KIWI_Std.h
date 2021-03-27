#pragma once

#include <assert.h>

#ifndef DECLSPEC
	#if defined(_WIN32) || defined(__WIN32__) || defined(__WINRT__) || defined(__CYGWIN__)
		#ifdef KIWI_DLL_EXPORT
			#define DECLSPEC __declspec(dllexport)
		#else
			#define DECLSPEC
		#endif
	#endif
#endif

#define KIWI_ASSERT(exp) assert(exp)

#define KIWI_ALLOW_UNUSED(var) (void)var
