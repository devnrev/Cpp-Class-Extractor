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

#ifndef CLASS_EXTRACTOR_H
#define CLASS_EXTRACTOR_H

#include "BinaryTableSearch.hpp"
#include <unordered_map>
#include <unordered_set>
#include "DualTree.hpp"
#include "ClassDefinition.h"
#include <map>

class Modelling::ClassDefinition;

namespace MachO{



class ClassNotFoundException{};

class ClassExtractor : BinaryOperations::BinaryTableSearch<address_t> {
public:
    typedef std::map<address_t, std::unique_ptr<Modelling::ClassDefinition > > GraphMap;
public:
    ClassExtractor(address_t relocation);
    void addClassSkeleton(Modelling::ClassDefinition&& classDef);
    void setPureVirtualReferences(std::unordered_set<address_t>&& virtualRefs);
    GraphMap constructClassGraph(byte_t* startAddress,byte_t* endAddress);

private:
    void extractClassfunctions(Modelling::ClassDefinition* classPtr, address_t* startAddress,address_t* endAddress);

private:
    GraphMap classGraph_;
    std::unordered_set<address_t> pureVirtualReferences_;
    address_t relocation_;


};



}

#endif