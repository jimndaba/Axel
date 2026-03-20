#pragma once
#ifndef BUFFERLAYOUT_H
#define BUFFERLAYOUT_H

#include <string>
#include <vector>

namespace Axel
{
	enum class ShaderDataType {
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};
    
    struct BufferElement {
        std::string Name;
        ShaderDataType Type;
        uint32_t Size;
        uint32_t Offset;

        BufferElement(ShaderDataType type, const std::string& name)
            : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0) {}

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

    class BufferLayout {
    public:
        BufferLayout(const std::initializer_list<BufferElement>& elements)
            : m_Elements(elements) {
            CalculateOffsetsAndStride();
        }

        uint32_t GetStride() const { return m_Stride; }
        const std::vector<BufferElement>& GetElements() const { return m_Elements; }

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
