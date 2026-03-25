#pragma once
#ifndef ISERIALISABLE_H
#define ISERIALISABLE_H

#include <core/Core.h>

namespace Axel
{
	// Forward declare the Archive so headers don't need its definition
	class IArchive;

	class AX_API  ISerialisable
	{
	public:
		virtual ~ISerialisable() = default;
		virtual void Serialize(IArchive& ar) = 0;
	};
	
}
#endif // !ISERIALISABLE_H
