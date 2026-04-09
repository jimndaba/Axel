#pragma once
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <core/Core.h>
#include <core/UUID.h>
#include <math/Math.h>

namespace Axel
{
    enum class ArchiveModeOptions { Save, Load };

    class IArchive {
    public:
        virtual ~IArchive() = default;
        virtual ArchiveModeOptions GetMode() const = 0;

        // Primitives
        virtual void Property(const char* name, float& value) = 0;
        virtual void Property(const char* name, int& value) = 0;
        virtual void Property(const char* name, bool& value) = 0;
        virtual void Property(const char* name, std::string& value) = 0;
        virtual void Property(const char* name, uint64_t& value) = 0;
        virtual void Property(const char* name, UUID& value) = 0;
        virtual void Property(const char* name, std::vector<UUID>& container) = 0;

		//Const versions for primitives (for read-only properties)
		virtual void Property(const char* name, const float& value) = 0;
		virtual void Property(const char* name, const int& value) = 0;
        virtual void Property(const char* name, const bool& value) = 0;
        virtual void Property(const char* name, const std::string& value) = 0;
        virtual void Property(const char* name, const uint64_t& value) = 0;
        virtual void Property(const char* name, const UUID& value) = 0;
        virtual void Property(const char* name, const std::vector<UUID>& container) =0;
        // 



        // Complex Axel types (Agnostic)
        virtual void Property(const char* name,Vec3& value) = 0;

		//Const version for complex types
		virtual void Property(const char* name, const Vec3& value) = 0;


		// Binary blob (for raw data, e.g., texture bytes)
        virtual void BinaryBlob(const char* name, void* data, size_t size) = 0;


        // Structure handling
        virtual bool BeginStruct(const char* name) = 0;
        virtual void EndStruct() = 0;

		// Collection handling (e.g., arrays, vectors)
        virtual bool BeginCollection(const char* name, uint32_t& size) = 0;
        virtual void NextItem() = 0;
        virtual void EndCollection() = 0;


    };
}



#endif // !ARCHIVE_H
