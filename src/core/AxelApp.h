#pragma once
#ifndef AXELAPP_H
#define AXELAPP_H

#include "Application.h"

namespace Axel
{

	class AxelApp : public Application
	{
	public:
			// Overridable by the specific game (e.g., MyGame : public Application)
			void OnStart();
			void OnUpdate(float deltaTime);
			void OnShutdown();
	};





}


#endif