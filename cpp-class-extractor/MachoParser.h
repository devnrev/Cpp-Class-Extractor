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

#ifndef CMachoParser_H
#define CMachoParser_H

#include <mach-o/loader.h> 
#include "MachOTypes.hpp"
#include <vector>
#include <memory>
#include <map>

namespace MachO{

typedef std::vector<std::shared_ptr<SymbolInfo> >::iterator SymbolIterator;

class SectionNotFoundException{};

class MachoParser {
public:
    MachoParser();
    ~MachoParser();
    
    bool parseHeader(address_t data,bool runtime);
    std::vector<std::shared_ptr<SymbolInfo> > getSymbolByName(std::string name, bool exact);
    std::vector<RelocationInfo> getRelocationEntries();
    SectionInfo getSectionInfo(std::string segName, std::string sectName) const;

	bool isArch64Bit();
    address_t getSlide();
    address_t getBaseOffset();
    size_t getDynamicSegmentSize();
private:
    void parseSymbolTable();

   
private:  
    SymbolTableInfo symbolTable_;
    DynamicSymbolTable dynamicSymbolTable_;
    std::vector<std::shared_ptr<SymbolInfo> > symbols_;
    std::vector<SectionInfo> sections_;
    address_t relocation_;    
	address_t aslrOffset_;
    address_t baseOffset_;
    size_t dynamicSize_;
	bool arch64_;
    
};

}
#endif
