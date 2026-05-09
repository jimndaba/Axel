#include "axelpch.h"
#include "JsonSaveArchive.h"
#include <core/Logger.h>


Axel::JsonSaveArchive::JsonSaveArchive(const std::string& filepath):
    mPath(filepath)
{    
}

void Axel::JsonSaveArchive::Property(const char* name, float& value)
{
    Property<float>(name,value);
}

void Axel::JsonSaveArchive::Property(const char* name, bool& value)
{
    Property<bool>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, std::string& value)
{
    Property<std::string>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, uint64_t& value)
{
    Property<uint64_t>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, uint32_t& value)
{
    Property<uint32_t>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, int32_t& value)
{
    Property<int32_t>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, double& value)
{
    Property<double>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, int64_t& value)
{
    Property<int64_t>(name, value);
}

void Axel::JsonSaveArchive::Property(const char* name, UUID& value)
{
    Property<UUID>(name, value);
}

bool Axel::JsonSaveArchive::BeginStruct(const char* name)
{
    mStructStack.push(json());
    mStructNameStack.push(name);
    return true;
}

void Axel::JsonSaveArchive::EndStruct()
{
    if (!mCollectionNameStack.empty() && !mStructStack.empty())
    {
        auto collectionDepth = mCollectionNameStack.size();
        auto structDepth = mStructStack.size();
        int current_depth = structDepth - collectionDepth;
        if (current_depth > 1)
        {
            auto current_struct = mStructStack.top();
            auto current_structname = mStructNameStack.top();          
            mStructStack.pop();
            mStructNameStack.pop();

            auto& next_struct = mStructStack.top();
            next_struct[current_structname] = current_struct;

                  
        }
        else
        {
            auto& current_collection = mCollectionStack.top();
            auto& collection_name = mCollectionNameStack.top();

            auto& current_struct = mStructStack.top();
            auto& current_structname = mStructNameStack.top();

            json j;
            j[current_structname] = current_struct;
            current_collection.push_back(j);

            mStructStack.pop();
            mStructNameStack.pop();
        }
        
    }
    else if (!mStructStack.empty() && mCollectionNameStack.empty())
    {
        auto current_struct = mStructStack.top();
        auto structname = mStructNameStack.top();
        mStructStack.pop();
        mStructNameStack.pop();


        if (!mStructStack.empty())
        {
            auto& next_struct = mStructStack.top();
            next_struct[structname] = current_struct;
        }
        else
        {
            Output[structname] = current_struct;
        }
    }
}

bool Axel::JsonSaveArchive::BeginCollection(const char* name, uint32_t& size)
{
    mCollectionStack.push(json::array_t());
    mCollectionNameStack.push(name);
    return true;
}

bool Axel::JsonSaveArchive::HasNext()
{
    return false;
}

void Axel::JsonSaveArchive::NextItem()
{
}

void Axel::JsonSaveArchive::EndCollection()
{
    if (mStructStack.empty() && !mCollectionStack.empty())
    {
        auto& current_collection = mCollectionStack.top();
        auto& current_collectionName = mCollectionNameStack.top();

        Output[current_collectionName] = current_collection;

        mCollectionStack.pop();
        mCollectionNameStack.pop();

    }
    if (!mStructStack.empty())
    {
        auto& current_struct = mStructStack.top();
        auto& current_structName = mStructNameStack.top();
        auto& collection_name = mCollectionNameStack.top();
        auto& current_collection = mCollectionStack.top();
        current_struct[collection_name] = current_collection;

        mCollectionNameStack.pop();
        mCollectionStack.pop();
    }
}

void Axel::JsonSaveArchive::Binary(const char* name, std::span<std::byte> data)
{
}

void Axel::JsonSaveArchive::Save()
{
    m_OutputStream.open(mPath, std::ios::out);

    if (!m_OutputStream.is_open())
    {
        AXLOG_ERROR("Failed to create file: {}", mPath);
        return;
    }

    m_OutputStream << Output.dump(4);

    m_OutputStream.close();
}




template<typename T>
void Axel::JsonSaveArchive::JsonSaveArchive::Property(const char* name, T& value)
{
    if (mStructStack.size() > mCollectionStack.size())
    {
        auto& j = mStructStack.top();
        j[name] = value;
    }
    else if (!mCollectionStack.empty() && (mCollectionStack.size() <= mStructStack.size()))
    {
        auto& j = mCollectionStack.top();
        j.push_back(value);
    }
}
