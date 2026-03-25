#pragma once
#ifndef YAMLARCHIVE_H
#define YAMLARCHIVE_H

#include "Archive.h"

namespace Axel
{

	class AX_API  YAMLArchive : public IArchive
	{
	public:
		YAMLArchive(ArchiveModeOptions mode,const std::string& filepath);
		~YAMLArchive();
		virtual ArchiveModeOptions GetMode() const override { return m_Mode; }
		
		// Primitives
		virtual void Property(const char* name, float& value) override;
		virtual void Property(const char* name, int& value) override;
		virtual void Property(const char* name, bool& value) override;
		virtual void Property(const char* name, std::string& value) override;
		virtual void Property(const char* name, uint64_t& value) override;
		virtual void Property(const char* name, UUID& value) override;

		//Const versions for primitives (for read-only properties)
		virtual void Property(const char* name, const float& value) override;
		virtual void Property(const char* name, const int& value) override;
		virtual void Property(const char* name, const bool& value) override;
		virtual void Property(const char* name, const std::string& value) override;
		virtual void Property(const char* name, const uint64_t& value) override;
		virtual void Property(const char* name, const UUID& value) override;

		
		// Complex Axel types (Agnostic)
		virtual void Property(const char* name, Vec3& value) override;

		//Const version for complex types
		virtual void Property(const char* name, const Vec3& value) override;
		
		// Structure handling
		virtual bool BeginStruct(const char* name) override;
		virtual void EndStruct() override;

		// Collection handling (e.g., arrays, vectors)
		virtual bool BeginCollection(const char* name, uint32_t& size) override;
		virtual void NextItem()  override;
		virtual void EndCollection()  override;

		// Binary blob (for raw data, e.g., texture bytes)
		virtual void BinaryBlob(const char* name, void* data, size_t size) override;

	private:
		ArchiveModeOptions m_Mode;
		struct YAMLData;
		YAMLData* m_Data;
	
	};
}


#endif