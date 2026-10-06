#include "KVStore.h"

RedClone::KVStore::Command RedClone::KVStore::parseCommand(string cmd)
{
    // transforms in the same space
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    // iterator
    auto it = lookup.find(cmd);
    if (it != lookup.end()){
        // returns second element (value in key:value pair)
        return it->second;
    }
    return Command::Unknown;
}

void RedClone::KVStore::set(const string &key, const string &value)
{
    m_store[key] = value;
}

bool RedClone::KVStore::get(const string &key, string &outValue)
{
    auto it = m_store.find(key);
    if (it != m_store.end())
    {
        outValue = it->second;
        return true;
    }
    return false;
}

bool RedClone::KVStore::del(const string &key)
{
    // returns true if number of elements erased are greater than 0
    return m_store.erase(key) > 0;
}
