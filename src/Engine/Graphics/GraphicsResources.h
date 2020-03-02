#pragma once

namespace Seele
{
	struct GraphicsInitializer
	{
		const char* windowLayoutFile;
	};
	struct WindowCreateInfo
	{
		int32 width;
		int32 height;
		const char* title;
		bool bFullscreen;
	};

}