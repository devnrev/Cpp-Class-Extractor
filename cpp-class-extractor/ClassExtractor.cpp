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

#include "ClassExtractor.h"
#include "ClassDefinition.h"

#include <future>


namespace MachO{

ClassExtractor::ClassExtractor(address_t relocation) : relocation_(relocation),
BinaryTableSearch() {

}

void ClassExtractor::addClassSkeleton(Modelling::ClassDefinition&& classDef){
    address_t offset = classDef.getTypeOffset();
    addElementToSearch(&offset, sizeof(address_t));
    classGraph_[offset] = std::make_unique<Modelling::ClassDefinition >(std::move(classDef));
}

ClassExtractor::GraphMap ClassExtractor::constructClassGraph(byte_t *startAddress, byte_t *endAddress) {
    using namespace Modelling;
    SearchTable *activeSearchTable = searchTables_.front().get();
    SearchTable *firstTable = activeSearchTable;
    short tableCounter = 0;
    address_t* referenceAddress;
    byte_t *bytePtr = startAddress;
    address_t* lastVtableStart = reinterpret_cast<address_t*>(startAddress);
    bool assignVtableFunctions = false;
    bool findVtableBounds = false;
    while (bytePtr <= endAddress) {
        auto elem = activeSearchTable->entries.find(*bytePtr);
        if (elem != std::end(activeSearchTable->entries)) {
            if (tableCounter == 0) {
                referenceAddress = reinterpret_cast<address_t *>(bytePtr);
            }
//            fprintf(stderr, "%x - %x - %d - %x -%x\n",reinterpret_cast<address_t>(bytePtr)-relocation_,*bytePtr,tableCounter,activeSearchTable,reinterpret_cast<address_t>(referenceAddress)-relocation_);
            ReferenceInfo *reference = elem->second.refPtr;
            if (reference != nullptr) {
//                fprintf(stderr, "%x\n",reinterpret_cast<address_t>(referenceAddress)-relocation_);
                address_t* prevValPtr = referenceAddress - 1;
                if ((*prevValPtr != 0) && ((*prevValPtr) < 0xFFFFFF00)) {
//                    fprintf(stderr, "%x - %x\n", reinterpret_cast<address_t>(prevValPtr)-relocation_,*prevValPtr );
                        do{
                            --prevValPtr;
                            if (prevValPtr<lastVtableStart){
                                prevValPtr = referenceAddress;
                                break;
                            }
                        }while ((*prevValPtr != 0x8));
//                    fprintf(stderr, "--> %x - %x\n",reinterpret_cast<address_t>(prevValPtr)-relocation_,*prevValPtr );
                    auto parentClassIt = classGraph_.find(reference->data);
                    auto currClassIt = classGraph_.find(reinterpret_cast<address_t>(prevValPtr)-relocation_);
                    if ((parentClassIt != std::end(classGraph_)) && (currClassIt != std::end(classGraph_))){
                        currClassIt->second->assignParentClass(parentClassIt->second.get());
                    }
                }else{
                    findVtableBounds = true;
                }
                if(assignVtableFunctions){
                    address_t* vTableEndAddress = prevValPtr;
                    address_t* typeRefAddress = reinterpret_cast<address_t* >((*lastVtableStart) + relocation_);
                    if ((lastVtableStart < typeRefAddress) && (typeRefAddress < vTableEndAddress)){
                        vTableEndAddress = typeRefAddress;
                    }
//                    fprintf(stderr, "### %x - %x - %x\n",reinterpret_cast<address_t>(lastVtableStart)-relocation_,
//                            reinterpret_cast<address_t>(vTableEndAddress)-relocation_,
//                            reinterpret_cast<address_t>(typeRefAddress)-relocation_);
                    auto iter = classGraph_.find(*lastVtableStart);
                    extractClassfunctions(iter->second.get(),lastVtableStart,vTableEndAddress);
                    assignVtableFunctions = false;
                }
                if (findVtableBounds){
                    assignVtableFunctions = true;
                    lastVtableStart = referenceAddress;
                    findVtableBounds = false;
                }

                activeSearchTable = firstTable;
                tableCounter = 0;
            } else {
                activeSearchTable = elem->second.nextTable;
                tableCounter++;
            }
            bytePtr++;
        } else {
            if (tableCounter == 0) {
                bytePtr++;
                continue;
            }
            tableCounter = 0;
            bytePtr = reinterpret_cast<byte_t *>(referenceAddress)+1;
            activeSearchTable = firstTable;
        }
    }
    if(assignVtableFunctions){
        auto iter = classGraph_.find(*lastVtableStart);
        extractClassfunctions(iter->second.get(),lastVtableStart, reinterpret_cast<address_t*>(classGraph_.rbegin()->first+relocation_));
    }
    searchTables_.clear();
    references_.clear();
    return std::move(classGraph_);
}

void ClassExtractor::extractClassfunctions(Modelling::ClassDefinition *classPtr,
        address_t *startAddress,
        address_t *endAddress) {
    auto classGraphEnd = std::end(classGraph_);
    for (address_t *addressPtr = startAddress+1; addressPtr < endAddress; addressPtr++) {
        if(classGraph_.find(reinterpret_cast<address_t>(addressPtr)-relocation_) != classGraphEnd){
            break;
        }
        bool pureVirtualFunc = false;
        if (*addressPtr == 0) {
            if (pureVirtualReferences_.find(
                    reinterpret_cast<address_t >(addressPtr) - relocation_) == std::end(pureVirtualReferences_)) {
                break;
            }
            pureVirtualFunc = true;
        }
//        fprintf(stderr, "-->%x - %x - pureVirtual: %x\n", reinterpret_cast<address_t>(addressPtr) - relocation_,
//                *addressPtr, pureVirtualFunc);
        classPtr->setOffset(reinterpret_cast<address_t>(startAddress)-relocation_);
        classPtr->addMemberFunction(Modelling::FunctionDefinition(
        {
            .offset = *addressPtr,
            .pureVirtual = pureVirtualFunc
        }));

    }
}

void ClassExtractor::setPureVirtualReferences(std::unordered_set<address_t> && virtualRefs){
    pureVirtualReferences_ = std::move(virtualRefs);
}


}
