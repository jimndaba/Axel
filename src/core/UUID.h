#pragma once
#ifndef UUID_H
#define UUID_H

#include "Core.h"
#include <xhash>
#include <yaml-cpp/yaml.h>



namespace Axel
{
   

		
    class AX_API UUID {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&);
                

        // Assignment from raw uint64_t
        UUID& operator=(uint64_t uuid) {
            m_UUID = uuid;
            return *this;
        }

        inline UUID operator = (const UUID& other) { m_UUID = other.m_UUID; return *this; }
        inline UUID operator = (const int& other) { m_UUID = other; return *this; }
        inline UUID operator = (const UUID& other) const { m_UUID = other.m_UUID;  return *this; }


        operator uint64_t() { return m_UUID; }
        operator const uint64_t() const { return m_UUID; }
        std::string ToString() const;

        static const uint64_t Invalid = 0;
    private:
        mutable uint64_t m_UUID = 0;
    };



}

namespace YAML {
    template<>
    struct convert<Axel::UUID> {
        static Node encode(const Axel::UUID& uuid) {
            // Convert UUID to the underlying type (e.g., uint64_t)
            return Node((uint64_t)uuid);
        }

        static bool decode(const Node& node, Axel::UUID& uuid) {
            if (!node.IsScalar()) {
                return false;
            }

            // Read as uint64_t and assign to UUID
            uuid = node.as<uint64_t>();
            return true;
        }
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