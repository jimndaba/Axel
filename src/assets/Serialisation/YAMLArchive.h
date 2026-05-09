#pragma once
#ifndef YAMLARCHIVE_H
#define YAMLARCHIVE_H

#include "Archive.h"

namespace Axel
{
	enum class YAMLContextType
	{
		Map,
		Sequence
	};

	enum class NodeType
	{
		Struct,
		Sequence
	};

	struct ArchiveFrame
	{
		NodeType Type;
	};

	class AX_API  YAMLArchive : public IArchive
	{
	public:
		YAMLArchive(ArchiveModeOptions mode,const std::string& filepath);
		~YAMLArchive();
		virtual ArchiveModeOptions GetMode() const override { return m_Mode; }
		
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
	


		virtual void AssetProperty(const char* name, UUID& value) {};

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

	private:
		ArchiveModeOptions m_Mode;
		struct YAMLData;
		YAMLData* m_Data;	
	};
}


#endif