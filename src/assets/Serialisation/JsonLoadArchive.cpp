#include "axelpch.h"
#include "JsonLoadArchive.h"
#include "axelpch.h"
#include "JsonLoadArchive.h"
#include <core/Logger.h>
#include <fstream>

Axel::JsonLoadArchive::JsonLoadArchive(const std::string& filepath)
    : mPath(filepath)
{    
}

void Axel::JsonLoadArchive::Load()
{
    m_InputStream = std::ifstream(mPath);
    if (!m_InputStream.good())
    {
        AXLOG_WARN("Can't read file : {}", mPath);
        return;
    }

    m_InputStream >> Output;
    if (Output.is_null())
    {
        AXLOG_WARN("File is either non-json file or corrupt;");
        return;
    }
    AXLOG_TRACE("Opening File: {}", mPath);
}

uint32_t Axel::JsonLoadArchive::CurrentCollectionIndex()
{
    int index = mCollectionIndex.top();
    AXLOG_TRACE("Collection Index: {}", index);
    return index;
}

uint32_t Axel::JsonLoadArchive::GetCollectionSize()
{
    return (uint32_t)mCollectionStack.top().size();
}

bool Axel::JsonLoadArchive::BeginStruct(const char* name)
{
    if (!Output.is_null())
    {
        int structDepth = mStructStack.size();
        int collectionDepth = mCollectionStack.size();
        int currentdept = structDepth - collectionDepth;
        if (currentdept >= 1)
        {
            if (exists(mStructStack.top(), name))
            {
                auto j = mStructStack.top()[name];
                mStructStack.push(j);
                mStructNameStack.push(name);
                return true;
            }
        }
        if (!mStructStack.empty() && mCollectionStack.empty())
        {
            if (exists(mStructStack.top(), name))
            {
                auto j = mStructStack.top()[name];
                mStructStack.push(j);
                mStructNameStack.push(name);
                return true;
            }
            return false;
        }
        else if (!mCollectionStack.empty())
        {
            auto currentCollection = mCollectionStack.top();
            auto collection_name = mCollectionNameStack.top();
            if (exists(currentCollection[CurrentCollectionIndex()], name))
            {
                auto e = currentCollection[CurrentCollectionIndex()][name];
                if (!e.is_null())
                {
                    mStructNameStack.push(name);
                    mStructStack.push(e);
                    return true;
                }
            }
            return false;
        }
        else
        {
            if (exists(Output, name))
            {
                auto j = Output[name];
                mStructStack.push(j);
                mStructNameStack.push(name);
                return true;
            }
        }
    }
}

void Axel::JsonLoadArchive::EndStruct()
{
    mStructStack.pop();
    mStructNameStack.pop();
}

bool Axel::JsonLoadArchive::BeginCollection(const char* name, uint32_t& size)
{
    if (!mStructStack.empty())
    {
        auto j = mStructStack.top();

        if (!exists(j, name))
            return false;

        mCollectionStack.push(j[name]);
        mCollectionNameStack.push(name);
        mCollectionIndex.push(0);
        size = (uint32_t)mCollectionStack.top().size();
        return true;
    }
    if (!Output.is_null())
    {
        if (!exists(Output, name))
            return false;

        auto j = Output[name];
        mCollectionStack.push(j);
        mCollectionNameStack.push(name);
        mCollectionIndex.push(0);
        size = (uint32_t)mCollectionStack.top().size();
        return true;
    }
}

bool Axel::JsonLoadArchive::HasNext()
{   
    return CurrentCollectionIndex() < GetCollectionSize();
}

void Axel::JsonLoadArchive::NextItem()
{
    mCollectionIndex.top()++;
}

void Axel::JsonLoadArchive::EndCollection()
{
    mCollectionNameStack.pop();
    mCollectionStack.pop();
    mCollectionIndex.pop();
}

void Axel::JsonLoadArchive::Binary(const char* name, std::span<std::byte> data)
{
}

// --- Universal Property Loader ---

template<typename T>
void Axel::JsonLoadArchive::Property(const char* name, T& value)
{
    // Case 1: Standard Struct Property (e.g., TransformComponent -> Position)
    // We are deeper in a struct than the current collection level
    if (mStructStack.size() > mCollectionStack.size())
    {
        const auto& j = mStructStack.top();
        if (j.contains(name))
        {
            value = j[name].get<T>();
        }
    }
    // Case 2: Property inside a Collection (The Fracture Way)
    // We are inside an array and need to access the item at the current index
    else if (!mCollectionStack.empty() && (mCollectionStack.size() >= mStructStack.size()))
    {
        const auto& j = mCollectionStack.top();
        uint32_t index = mCollectionIndex.top();

        if (index < j.size())
        {
            const auto& item = j.at(index);
            // In Fracture, array items are often wrapped: { "Entity": { ... } }
            // If the item is an object and contains the key, read from it
            if (item.is_object() && item.contains(name))
            {
                value = item[name].get<T>();
            }
            else
            {
                // Otherwise, read the raw value at this index
                value = item.get<T>();
            }
        }
    }
}
void Axel::JsonLoadArchive::Property(const char* name, float& value) { Property<float>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, bool& value) { Property<bool>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, std::string& value) { Property<std::string>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, uint64_t& value) { Property<uint64_t>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, uint32_t& value) { Property<uint32_t>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, int32_t& value) { Property<int32_t>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, UUID& value) { Property<UUID>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, double& value){ Property<double>(name, value); }
void Axel::JsonLoadArchive::Property(const char* name, int64_t& value){ Property<int64_t>(name, value); }