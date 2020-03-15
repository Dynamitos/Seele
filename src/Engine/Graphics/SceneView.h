#pragma once
#include "View.h"
namespace Seele
{
	class SceneView : public View
	{
	public:
		SceneView(PGraphics graphics);
		~SceneView();
	};
}