#pragma once
#include "MinimalEngine.h"
#include "GraphicsResources.h"

namespace Seele {
	class Window;
	class Graphics
	{
	public:
		static Graphics& getInstance()
		{
			static Graphics instance;

			return instance;
		}
		void init(GraphicsInitializer initializer);
		void beginFrame();
		void endFrame();
	
	//Singleton
	private:
		Graphics();
		Graphics(Graphics const&) = delete;
		void operator=(Graphics const&) = delete;
		~Graphics();

	};
}