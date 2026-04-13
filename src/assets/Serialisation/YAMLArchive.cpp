#include "axelpch.h"
#include "YAMLArchive.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <stack>

// Internal structure hidden from Axel's frontend
struct Axel::YAMLArchive::YAMLData {
	YAML::Emitter Emitter;
	YAML::Node RootNode;
	std::stack<YAML::Node> NodeStack;
	std::stack<int> IndexStack;
	std::string FilePath;
	int CollectionIndex = 0; // For tracking position in collections
};

Axel::YAMLArchive::YAMLArchive(ArchiveModeOptions mode, const std::string& filepath):
	m_Mode(mode)
{
	m_Data = new YAMLData();
	m_Data->FilePath = filepath;

	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::BeginMap;
	}
	else {
		m_Data->RootNode = YAML::LoadFile(filepath);
		m_Data->NodeStack.push(m_Data->RootNode);
	}
}

Axel::YAMLArchive::~YAMLArchive()
{
	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::EndMap; // Close the Root Map
		std::ofstream fout(m_Data->FilePath);
		fout << m_Data->Emitter.c_str();
	}
	delete m_Data;
}

#pragma region Primitive Properties
void Axel::YAMLArchive::Property(const char* name, float& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<float>();
	}
}

void Axel::YAMLArchive::Property(const char* name, bool& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<bool>();
	}
}

void Axel::YAMLArchive::Property(const char* name, std::string& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<std::string>();
	}
}

void Axel::YAMLArchive::Property(const char* name, uint64_t& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<uint64_t>();
	}
}

void Axel::YAMLArchive::Property(const char* name, UUID& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << (uint64_t)value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<uint64_t>();
	}
}

void Axel::YAMLArchive::Property(const char* name, std::vector<UUID>& container)
{
	if (m_Mode == ArchiveModeOptions::Save)
	{
		m_Data->Emitter << YAML::Key << name;
		m_Data->Emitter << YAML::BeginSeq;
		for (const auto& id : container)
		{
			// Cast UUID to uint64_t for the emitter
			m_Data->Emitter << (uint64_t)id;
		}
		m_Data->Emitter << YAML::EndSeq;
	}
	else // ArchiveModeOptions::Load
	{
		auto& node = m_Data->NodeStack.top();
		if (node[name] && node[name].IsSequence())
		{
			container.clear();
			for (auto item : node[name])
			{
				// Extract as uint64_t and convert back to UUID
				container.emplace_back(item.as<uint64_t>());
			}
		}
	}
}

void Axel::YAMLArchive::Property(const char* name, Vec3& value)
{
	if (BeginStruct(name)) {
		Property("x", value.x);
		Property("y", value.y);
		Property("z", value.z);
		EndStruct();
	}
}

void Axel::YAMLArchive::Property(const char* name, uint32_t& value) {
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<int>();
	}
}
void Axel::YAMLArchive::Property(const char* name, int32_t& value) {
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
	else {
		auto& node = m_Data->NodeStack.top();
		if (node[name]) value = node[name].as<int>();
	}
}

void Axel::YAMLArchive::Property(const char* name, Vec2& value) {}
void Axel::YAMLArchive::Property(const char* name, Vec4& value) {}

#pragma endregion

#pragma region Primitive Const Properties
void Axel::YAMLArchive::Property(const char* name, const float& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
}

void Axel::YAMLArchive::Property(const char* name, const bool& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
}

void Axel::YAMLArchive::Property(const char* name, const std::string& nameValue)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << nameValue;
}

void Axel::YAMLArchive::Property(const char* name, const uint64_t& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
}

void Axel::YAMLArchive::Property(const char* name, const UUID& value)
{
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << (uint64_t)value;
}

void Axel::YAMLArchive::Property(const char* name, const Vec3& value)
{
	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::Key << name << YAML::BeginMap;
		m_Data->Emitter << YAML::Key << "x" << YAML::Value << value.x;
		m_Data->Emitter << YAML::Key << "y" << YAML::Value << value.y;
		m_Data->Emitter << YAML::Key << "z" << YAML::Value << value.z;
		m_Data->Emitter << YAML::EndMap;
	}
}

void Axel::YAMLArchive::Property(const char* name, const Vec2& value) {}
void Axel::YAMLArchive::Property(const char* name, const Vec4& value) {}
void Axel::YAMLArchive::Property(const char* name, const std::vector<UUID>& container)
{
	if (m_Mode == ArchiveModeOptions::Save)
	{
		m_Data->Emitter << YAML::Key << name;
		m_Data->Emitter << YAML::BeginSeq;
		for (const auto& id : container)
		{
			// Cast UUID to uint64_t for the emitter
			m_Data->Emitter << (uint64_t)id;
		}
		m_Data->Emitter << YAML::EndSeq;
	}
}
void Axel::YAMLArchive::Property(const char* name,const uint32_t& value) {
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
}
void Axel::YAMLArchive::Property(const char* name,const int32_t& value) {
	if (m_Mode == ArchiveModeOptions::Save) m_Data->Emitter << YAML::Key << name << YAML::Value << value;
}


#pragma endregion

bool Axel::YAMLArchive::BeginStruct(const char* name)
{
	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::Key << name << YAML::BeginMap;
	}
	else {
		auto& currentNode = m_Data->NodeStack.top();
		m_Data->NodeStack.push(currentNode[name]);
	}
	return true;
}

void Axel::YAMLArchive::EndStruct()
{
	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::EndMap;
	}
	else {
		m_Data->NodeStack.pop();
	}
}

bool Axel::YAMLArchive::BeginCollection(const char* name, uint32_t& size)
{
	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::Key << name << YAML::BeginSeq;
		return true;
	}
	else {
		auto& currentNode = m_Data->NodeStack.top();
		if (currentNode[name] && currentNode[name].IsSequence()) {
			m_Data->NodeStack.push(currentNode[name]);
			m_Data->IndexStack.push(0);
			size = (uint32_t)m_Data->NodeStack.top().size();
			return true;
		}
		return false;
	}
}

void Axel::YAMLArchive::NextItem()
{
	if (m_Mode == ArchiveModeOptions::Load) {
		m_Data->IndexStack.top()++;
	}
	// Save mode: YAML::Emitter automatically handles the next sequence entry
}

void Axel::YAMLArchive::EndCollection()
{
	if (m_Mode == ArchiveModeOptions::Save) {
		m_Data->Emitter << YAML::EndSeq;
	}
	else {
		m_Data->NodeStack.pop();
		m_Data->IndexStack.pop();
	}
}

void Axel::YAMLArchive::BinaryBlob(const char* name, void* data, size_t size)
{
}
