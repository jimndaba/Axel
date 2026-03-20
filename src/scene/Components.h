#pragma once
#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "core/UUID.h"
#include <string>

namespace Axel
{

	struct IDComponent {
		UUID ID; // Uses our new class
	};

	struct TagComponent
	{
		std::string Name;
	};
}



#endif