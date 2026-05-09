#include "axelpch.h"
#include "SceneLoader.h"
#include <scene/Scene.h>
#include <assets/ISerialisable.h>
#include <assets/Serialisation/JsonLoadArchive.h>

std::shared_ptr<Axel::Scene> Axel::SceneLoader::LoadScene(const std::string& path)
{
    auto scene = std::make_shared<Axel::Scene>();
    Axel::JsonLoadArchive ar(path);
    ar.Load();
    scene->Serialize(ar);
    return scene;
}
