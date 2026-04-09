#include "axelpch.h"
#include "Material.h"

Axel::Material::Material()
{
}

void Axel::Material::Serialize(IArchive& ar)
{
}

const Axel::MaterialParameter* Axel::MaterialParameterLayout::Find(const std::string& name) const
{
	return nullptr;
}
