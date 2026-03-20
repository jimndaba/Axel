#include "axelpch.h"
#include "GraphicsDevice.h"

Axel::GraphicsDevice* Axel::GraphicsDevice::s_Instance = nullptr;

Axel::GraphicsDevice::GraphicsDevice()
{
	s_Instance = this;
}
