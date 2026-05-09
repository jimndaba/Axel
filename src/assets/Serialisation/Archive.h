#pragma once
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <core/Core.h>
#include <core/UUID.h>
#include <math/Math.h>
#include <span>

namespace Axel
{
    enum class ArchiveModeOptions { Save, Load,Editor };

    class IArchive {
    public:
        virtual ~IArchive() = default;
        virtual ArchiveModeOptions GetMode() const = 0;

        // Primitive values only
        virtual void Property(const char* name, float& value) = 0;
        virtual void Property(const char* name, double& value) = 0;
        virtual void Property(const char* name, bool& value) = 0;
        virtual void Property(const char* name, int32_t& value) = 0;
        virtual void Property(const char* name, uint32_t& value) = 0;
        virtual void Property(const char* name, int64_t& value) = 0;
        virtual void Property(const char* name, uint64_t& value) = 0;
        virtual void Property(const char* name, std::string& value) = 0;
        virtual void Property(const char* name, UUID& value) = 0;
        virtual void Binary(
            const char* name,
            std::span<std::byte> data
        ) = 0;

        // Structure handling
        virtual bool BeginStruct(const char* name) = 0;
        virtual void EndStruct() = 0;

		// Collection handling (e.g., arrays, vectors)
        virtual bool BeginCollection(const char* name, uint32_t& size) = 0;
        virtual void NextItem() = 0;
        virtual bool HasNext() = 0;
        virtual void EndCollection() = 0;

        template<typename T>
        static constexpr bool SupportsDirectProperty = false;

    };
}



#endif // !ARCHIVE_H
