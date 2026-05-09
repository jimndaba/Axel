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
	std::stack<YAMLContextType> ContextStack;
	std::stack<ArchiveFrame> FrameStack;
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
	Property<float>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, bool& value)
{
	Property<bool>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, std::string& value)
{
	Property<std::string>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, uint64_t& value)
{
	Property<uint64_t>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, uint32_t& value) {
	Property<uint32_t>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, int32_t& value) {
	Property<int32_t>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, double& value)
{
	Property<double>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, int64_t& value)
{
	Property<int64_t>(name, value);
}

void Axel::YAMLArchive::Property(const char* name, UUID& value)
{
	Property<UUID>(name, value);
}

bool Axel::YAMLArchive::BeginStruct(const char* name)
{
	if (m_Mode == ArchiveModeOptions::Save)
	{
		if (name && strlen(name) > 0)
		{
			m_Data->Emitter << YAML::Key << name;
			m_Data->Emitter << YAML::Value; // Bridging Key to the upcoming Map
		}

		m_Data->Emitter << YAML::BeginMap;
		m_Data->FrameStack.push({ NodeType::Struct });
		return true;
	}
	else
	{
		if (m_Data->NodeStack.empty()) return false;
		auto& node = m_Data->NodeStack.top();

		YAML::Node target;

		if (name && strlen(name) > 0)
		{
			if (!node[name]) return false;
			target = node[name];
		}
		else if (node.IsSequence()) // Handle unnamed structs in a list
		{
			uint32_t index = m_Data->IndexStack.top();
			if (index >= node.size()) return false;
			target = node[index];
		}
		else
		{
			return false;
		}

		m_Data->NodeStack.push(target);
		m_Data->FrameStack.push({ NodeType::Struct });
		return true;
	}
}

void Axel::YAMLArchive::EndStruct()
{
	if (m_Mode == ArchiveModeOptions::Save)
	{
		m_Data->Emitter << YAML::EndMap;
	}
	else
	{
		m_Data->NodeStack.pop();
	}

	m_Data->FrameStack.pop();
}

bool Axel::YAMLArchive::BeginCollection(const char* name, uint32_t& size)
{
	
	if (m_Mode == ArchiveModeOptions::Save)
	{
		if (name && strlen(name) > 0)
		{
			m_Data->Emitter << YAML::Key << name;
			m_Data->Emitter << YAML::Value; // Bridging Key to the upcoming Seq
		}

		m_Data->Emitter << YAML::BeginSeq;
		m_Data->FrameStack.push({ NodeType::Sequence });
		return true;
	}
	else
	{
		auto& node = m_Data->NodeStack.top();
		YAML::Node target;

		if (name && strlen(name) > 0)
		{
			if (!node[name] || !node[name].IsSequence()) return false;
			target = node[name];
		}
		else { return false; }

		m_Data->NodeStack.push(target);
		size = (uint32_t)target.size();
		m_Data->IndexStack.push(0);
		m_Data->FrameStack.push({ NodeType::Sequence });
		return true;
	}
}

bool Axel::YAMLArchive::HasNext()
{
	if (m_Mode == ArchiveModeOptions::Load)
	{
		auto& seq = m_Data->NodeStack.top();
		uint32_t index = m_Data->IndexStack.top();
		return index < seq.size();
	}

	return false;
}

void Axel::YAMLArchive::NextItem()
{
	if (m_Mode == ArchiveModeOptions::Load)
		m_Data->IndexStack.top()++;
}

void Axel::YAMLArchive::EndCollection()
{
	if (m_Mode == ArchiveModeOptions::Save)
	{
		m_Data->Emitter << YAML::EndSeq;
	}
	else
	{
		m_Data->NodeStack.pop();
		m_Data->IndexStack.pop();
	}

	m_Data->FrameStack.pop();
	
}

void Axel::YAMLArchive::Binary(const char* name, std::span<std::byte> data)
{
}


template<typename T>
void Axel::YAMLArchive::Property(const char* name, T& value)
{
	if (m_Mode == ArchiveModeOptions::Save)
	{
		if (name && strlen(name) > 0)
		{
			m_Data->Emitter << YAML::Key << name << YAML::Value << value;
		}
		else
		{
			// Raw value emission for collection items
			m_Data->Emitter << value;
		}
	}
	else // Load
	{
		auto& node = m_Data->NodeStack.top();
		if (name && strlen(name) > 0)
		{
			if (node[name]) value = node[name].as<T>();
		}
		else if (node.IsSequence())
		{
			uint32_t index = m_Data->IndexStack.top();
			if (index < node.size()) value = node[index].as<T>();
		}
	}

}