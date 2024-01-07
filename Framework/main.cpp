#if defined(_DEBUG) || defined(DEBUG)
//メモリーリーク検出
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "App.h"

int wmain(int argc, wchar_t** argv, wchar_t** evnp)
{
#if defined(_DEBUG) || defined(DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	//アプリケーション実行
	App app(960, 540);
	app.Run();

	return 0;
}