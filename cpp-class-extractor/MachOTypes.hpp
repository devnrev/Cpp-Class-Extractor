//Copyright (c) 2012, devnrev (Axel)
//
//This file is part of Cpp-Class-Extractor.
//
//Cpp-Class-Extractor is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//Cpp-Class-Extractor is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with Cpp-Class-Extractor.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MachOTypes_H
#define MachOTypes_H

#include <vector>
#include <string>

namespace MachO{

struct SymbolTableInfo{
    address_t    offset;
    int     numberOfSymbols;
    address_t    stringOffset;
    size_t  stringSize;
    
};

struct DynamicSymbolTable{
    address_t externalRelocationTable;
    int     numberOfExternalRelocs;

};

struct SymbolInfo{
    address_t    offset;
    std::string name;
        
};

struct AddressInfo {
    address_t vmOffset;
    address_t fileOffset;
};

struct RelocationInfo{
    std::shared_ptr<SymbolInfo> symbol;
    std::vector<AddressInfo> relocationOffsets;     
};

struct SectionInfo{
    std::string segName;
    std::string sectName;
    address_t vmAddress;
    address_t fileOffset;
    size_t size;
};



}
#endif
