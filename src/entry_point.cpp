#include "axelpch.h"

#include "core/AxelApp.h"

int main()
{
	std::unique_ptr<Axel::AxelApp> app = std::make_unique<Axel::AxelApp>();
	app->Run();
	app->Quit();

	return 0;
}