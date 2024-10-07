#include "OAHashTable.h"

struct OAHTSlot;

template <class T>
OAHashTable<T>::OAHashTable(const OAHTConfig& Config) : Config_(Config)
{
}

template <class T>
OAHashTable<T>::~OAHashTable()
{
}

template <class T>
void OAHashTable<T>::insert(const char* Key, const T& Data)
{
    OAHTSlot* slot = nullptr;
    int index = IndexOf(Key, slot);
    if (index == -1)
    {
        GrowTable();
        index = IndexOf(Key, slot);
    }
    slot->Key = Key;
    slot->Data = Data;
    slot->State = OAHTSlot::OAHTSlotState::OCCUPIED;
    Config_.NumOccupiedSlots++;
}

template <typename T>
void OAHashTable<T>::remove(const char* Key)
{
    OAHTSlot* slot = nullptr;
    int index = IndexOf(Key, slot);
    if (index != -1)
    {
        slot->State = OAHTSlot::OAHTSlotState::DELETED;
        Config_.NumOccupiedSlots--;
    }
}

template <typename T>
const T& OAHashTable<T>::find(const char* Key) const
{
    OAHTSlot* slot = nullptr;
    int index = IndexOf(Key, slot);
    if (index != -1)
    {
        return slot->Data;
    }
    return nullptr;
}

template <typename T>
void OAHashTable<T>::clear()
{
    Config_.NumOccupiedSlots = 0;
    InitTable();
}

template <typename T>
OAHTStats OAHashTable<T>::GetStats() const
{
    OAHTStats stats;
    stats.NumOccupiedSlots = Config_.NumOccupiedSlots;
    stats.TableSize = Config_.TableSize;
    return stats;
}

template <typename T>
const OAHTSlot* OAHashTable<T>::GetTable() const
{
    return Config_.Table;
}

template <typename T>
void OAHashTable<T>::InitTable()
{
    Config_.NumOccupiedSlots = 0;
    Config_.TableSize = Config_.InitialTableSize;
    Config_.Table = new OAHTSlot[Config_.TableSize];
    for (int i = 0; i < Config_.TableSize; i++)
    {
        Config_.Table[i].State = OAHTSlot::OAHTSlotState::EMPTY;
    }
}

template <typename T>
void OAHashTable<T>::GrowTable()
{
    OAHTSlot* oldTable = Config_.Table;
    int oldTableSize = Config_.TableSize;
    Config_.TableSize = Config_.TableSize * 2;
    Config_.Table = new OAHTSlot[Config_.TableSize];
    for (int i = 0; i < Config_.TableSize; i++)
    {
        Config_.Table[i].State = OAHTSlot::OAHTSlotState::EMPTY;
    }
    for (int i = 0; i < oldTableSize; i++)
    {
        if (oldTable[i].State == OAHTSlot::OAHTSlotState::OCCUPIED)
        {
            OAHTSlot* slot = nullptr;
            int index = IndexOf(oldTable[i].Key, slot);
            slot->Key = oldTable[i].Key;
            slot->Data = oldTable[i].Data;
            slot->State = OAHTSlot::OAHTSlotState::OCCUPIED;
        }
    }
    delete[] oldTable;
}

template <typename T>
int OAHashTable<T>::IndexOf(const char* Key, OAHTSlot*& Slot) const
{
    int index = Hash(Key) % Config_.TableSize;
    int i = 0;
    while (i < Config_.TableSize)
    {
        if (Config_.Table[index].State == OAHTSlot::OAHTSlotState::EMPTY)
        {
            Slot = &Config_.Table[index];
            return index;
        }
        if (Config_.Table[index].State == OAHTSlot::OAHTSlotState::OCCUPIED &&
            strcmp(Config_.Table[index].Key, Key) == 0)
        {
            Slot = &Config_.Table[index];
            return index;
        }
        index = (index + 1) % Config_.TableSize;
        i++;
    }
    return -1;
}
