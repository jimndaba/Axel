#pragma once
#ifndef JSONLOADARCHIVE_H
#define JSONLOADARCHIVE_H

#include "JsonUtils.h"
#include <assets/Serialisation/Archive.h>
#include <stack>
#include <fstream>



namespace Axel
{
	using json = nlohmann::json;

	class JsonLoadArchive : public IArchive
	{

	public:
		JsonLoadArchive(const std::string& filepath);

		virtual ArchiveModeOptions GetMode() const override { return ArchiveModeOptions::Load; }

		// Primitives
		virtual void Property(const char* name, float& value) override;
		virtual void Property(const char* name, bool& value) override;
		virtual void Property(const char* name, std::string& value) override;
		virtual void Property(const char* name, uint64_t& value) override;
		virtual void Property(const char* name, uint32_t& value) override;
		virtual void Property(const char* name, int32_t& value) override;
		virtual void Property(const char* name, double& value) override;
		virtual void Property(const char* name, int64_t& value) override;
		virtual void Property(const char* name, UUID& value) override;

		// Structure handling
		virtual bool BeginStruct(const char* name) override;
		virtual void EndStruct() override;

		// Collection handling (e.g., arrays, vectors)
		virtual bool BeginCollection(const char* name, uint32_t& size) override;
		virtual bool HasNext() override;
		virtual void NextItem()  override;
		virtual void EndCollection()  override;

		// Binary blob (for raw data, e.g., texture bytes)
		virtual void Binary(
			const char* name,
			std::span<std::byte> data
		) override;


		template<typename T>
		void Property(const char* name, T& value);

		void Load();

		bool exists(const json& j, const std::string& key)
		{
			return j.find(key) != j.end();
		}
		uint32_t CurrentCollectionIndex();
		uint32_t GetCollectionSize();

	private:
		std::string mPath;
		json Output;
		std::ifstream m_InputStream;
		std::stack<json> mStructStack;
		std::stack<std::string> mStructNameStack;

		std::stack<json::array_t> mCollectionStack;
		std::stack<std::string> mCollectionNameStack;
		std::stack<unsigned int> mCollectionIndex;
	};



}


#endif