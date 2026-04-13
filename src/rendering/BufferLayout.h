#pragma once
#ifndef BUFFERLAYOUT_H
#define BUFFERLAYOUT_H

#include "GraphicsCore.h"
#include <core/Core.h>

namespace Axel
{

    struct AX_API BufferElement {
        std::string Name = "";
        ShaderDataType Type = ShaderDataType::None;
        uint32_t Size = 0;
        uint32_t Offset = 0;
        uint32_t Location = 0;

        BufferElement(ShaderDataType type, const std::string& name, uint32_t location = 0)
            : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Location(location) {
        }

        static uint32_t ShaderDataTypeSize(ShaderDataType type) {
            switch (type) {
            case ShaderDataType::Float:    return 4;
            case ShaderDataType::Float2:   return 4 * 2;
            case ShaderDataType::Float3:   return 4 * 3;
            case ShaderDataType::Float4:   return 4 * 4;
            case ShaderDataType::Mat3:     return 4 * 3 * 3;
            case ShaderDataType::Mat4:     return 4 * 4 * 4;
            case ShaderDataType::Int:      return 4;
            case ShaderDataType::Int2:     return 4 * 2;
            case ShaderDataType::Int3:     return 4 * 3;
            case ShaderDataType::Int4:     return 4 * 4;
            case ShaderDataType::Bool:     return 1;
            default: break;
            }

            // You should use your Axel::Logger here
            // Logger::Error("Unknown ShaderDataType!");
            return 0;
        }
    };

    class AX_API BufferLayout {
    public:
        // Add this line!
        BufferLayout() = default;
        BufferLayout(std::vector<BufferElement> elements) :
            m_Elements(elements){
            CalculateOffsetsAndStride();
        }


        BufferLayout(const std::initializer_list<BufferElement>& elements)
            : m_Elements(elements) {
            CalculateOffsetsAndStride();
        }

        uint32_t GetStride() const { return m_Stride; }
        const std::vector<BufferElement>& GetElements() const { return m_Elements; }

        std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
        std::vector<BufferElement>::iterator end() { return m_Elements.end(); }

    private:
        void CalculateOffsetsAndStride() {
            uint32_t offset = 0;
            m_Stride = 0;
            for (auto& element : m_Elements) {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }
        std::vector<BufferElement> m_Elements;
        uint32_t m_Stride = 0;
    };

}


#endif
