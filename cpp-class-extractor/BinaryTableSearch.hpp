//Copyright (c) 2012, devnrev (Axel)
//
//This file is part of Cpp-Class-Extractor.
//
//Cpp-Class-Extractor is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//Foobar is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with Cpp-Class-Extractor.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BinaryTableSearch_H
#define BinaryTableSearch_H

#include <vector>
#include <tuple>
#include <string>
#include <unordered_map>
#include "StdExtension.h"
namespace BinaryOperations{

template<class T>
struct TypeSeletion {
    typedef T *ArgumentType;
    typedef T ValueType;

};

template<>
struct TypeSeletion<std::vector<byte_t > > {
    typedef std::vector<byte_t > ValueType;
    typedef ValueType& ArgumentType;


};


template<class T>
class BinaryTableSearch {
    private:

    typedef typename TypeSeletion<T>::ArgumentType argType;
    typedef typename TypeSeletion<T>::ValueType valueType;

    public:

    struct ReferenceInfo {
        valueType data;
        std::vector<address_t> references;

    };

    protected:

    struct SearchTable;
    struct SearchTableValue {
            SearchTable *nextTable;
            ReferenceInfo *refPtr;
        };

    typedef std::unordered_map<byte_t, SearchTableValue> SearchMap;
    struct SearchTable {
        SearchMap entries;
    };

    public:

    BinaryTableSearch();
    void addElementToSearch(const argType data, size_t length);
    std::vector<std::unique_ptr<ReferenceInfo> > performSearch(byte_t* startAddress, byte_t* endAddress);

    protected:

    SearchTable *addDataToTable(byte_t data, SearchTable* currentTable, bool lastElement);

    protected:
    std::vector<std::unique_ptr<ReferenceInfo> > references_;
    std::vector<std::unique_ptr<SearchTable> > searchTables_;
};

template <class T>
BinaryTableSearch<T>::BinaryTableSearch() {
    searchTables_.push_back(std::make_unique<SearchTable>());
}

template <class T>
typename BinaryTableSearch<T>::SearchTable *BinaryTableSearch<T>::addDataToTable(byte_t data, SearchTable* currentTable, bool lastElement) {
    auto iterator = currentTable->entries.find(data);
    if (iterator != std::end(currentTable->entries)) {
        if (lastElement) {
            iterator->second.refPtr = references_.back().get();
            return searchTables_.front().get();
        } else {
            SearchTable *nextTable = iterator->second.nextTable;
            if (nextTable != nullptr) {
                return nextTable;
            } else {
                auto nextTable = std::make_unique<SearchTable>();
                SearchTable* nextTablePtr = nextTable.get();
                searchTables_.push_back(std::move(nextTable));
                return nextTablePtr;
            }
        }
    }
    if (!lastElement) {
        auto nextTable  = std::make_unique<SearchTable>();
        SearchTable* nextTablePtr = nextTable.get();
        searchTables_.push_back(std::move(nextTable));
        currentTable->entries[data] = SearchTableValue(
        {
            .nextTable = nextTablePtr, .refPtr = nullptr});

        return nextTablePtr;
    }
    currentTable->entries[data] =  SearchTableValue(
    {
        .nextTable = nullptr, .refPtr = references_.back().get()
    });

    return searchTables_.front().get();

}

template <class T>
void BinaryTableSearch<T>::addElementToSearch(const typename TypeSeletion<T>::ArgumentType data, size_t length) {
    references_.push_back(std::unique_ptr<ReferenceInfo>(new ReferenceInfo({.data = *data})));
    SearchTable* activeTable = searchTables_.front().get();
    if (length > 0) {
        for (int i = 0; i < length-1; ++i) {
            activeTable= addDataToTable(reinterpret_cast<byte_t* >(data)[i],activeTable,false);
        }
        addDataToTable(reinterpret_cast<byte_t* >(data)[length - 1], activeTable, true);
    }
}

template <>
inline void BinaryTableSearch<std::vector<byte_t> >::addElementToSearch(const TypeSeletion<std::vector<byte_t> >::ArgumentType data, size_t length) {
    references_.push_back(std::unique_ptr<ReferenceInfo>(new ReferenceInfo({.data = data})));
    SearchTable* activeTable = searchTables_.front().get();
    if (length > 0) {
        for (int i = 0; i < length - 1; ++i) {
            activeTable= addDataToTable(data[i],activeTable, false);
        }
        addDataToTable(data[length - 1], activeTable, true);
    }
}


template <class T>
std::vector<std::unique_ptr<typename BinaryTableSearch<T>::ReferenceInfo> > BinaryTableSearch<T>::performSearch(byte_t* startAddress, byte_t* endAddress) {
    SearchTable* activeSearchTable = searchTables_.front().get();
    SearchTable* firstTable = activeSearchTable;
    short tableCounter = 0;
    address_t referenceAddress = 0;
    byte_t* bytePtr = startAddress;
    while(bytePtr <= endAddress){
        auto elem = activeSearchTable->entries.find(*bytePtr);
        if (elem != std::end(activeSearchTable->entries)){
            if (tableCounter == 0){
                referenceAddress = reinterpret_cast<address_t >(bytePtr);
            }
            ReferenceInfo* reference = elem->second.refPtr;
            if (reference != nullptr){
                reference->references.push_back(referenceAddress);
                activeSearchTable = firstTable;
                tableCounter = 0;
            }else{
                activeSearchTable = elem->second.nextTable;
                tableCounter++;
            }
            bytePtr++;
        }else{
            if (tableCounter == 0){
                bytePtr++;
                continue;
            }
            tableCounter = 0;
            activeSearchTable = firstTable;
        }
    }
    searchTables_.clear();
    return std::move(references_);
}

}
#endif
