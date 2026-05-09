#pragma once
#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <nlohmann/json.hpp>
#include <core/UUID.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace nlohmann
{
	using json = nlohmann::json;

	inline void to_json(nlohmann::json& j, const Axel::UUID& id) {
		j = (uint64_t)id;
	}
	inline void  to_json(json& j, const glm::vec2& vec)
	{
		j = json{ {"x", vec.x}, {"y", vec.y} };
	}
	inline void  to_json(json& j, const glm::vec3& vec)
	{
		j = json{ {"x", vec.x}, {"y", vec.y},{"z", vec.z} };
	}
	inline void  to_json(json& j, const glm::vec4& vec)
	{
		j = json{ {"x", vec.x}, {"y", vec.y},{"z", vec.z},{"w", vec.w} };
	}
	inline void  to_json(json& j, const glm::quat& vec)
	{
		j = json{ {"x", vec.x}, {"y", vec.y},{"z", vec.z},{"w", vec.w} };
	}

	inline void from_json(const json& j, glm::vec2& vec) {
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
	}
	
	inline void from_json(const json& j, glm::vec3& vec) {
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
		j.at("z").get_to(vec.z);
	}
	inline void from_json(const json& j, glm::vec4& vec) {
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
		j.at("z").get_to(vec.z);
		j.at("w").get_to(vec.w);
	}
	inline void from_json(const json& j, glm::quat& vec) {
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
		j.at("z").get_to(vec.z);
		j.at("w").get_to(vec.w);
	}

	inline void from_json(const nlohmann::json& j, Axel::UUID& id) {
		id = j.get<uint64_t>();
	}
}


#endif