#pragma once
#ifndef UUID_H
#define UUID_H

#include "Core.h"
#include <xhash>



namespace Axel
{
   
		
    class AX_API UUID {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }
        std::string ToString() const;

    private:
        uint64_t m_UUID = 0;
    };



}



// Specialization for std::hash so we can use UUID as a key in std::unordered_map
namespace std {
    template <typename T> struct hash;
    template<>
    struct hash<Axel::UUID> {
        std::size_t operator()(const Axel::UUID& uuid) const {
            return (uint64_t)uuid;
        }
    };
}


#endif