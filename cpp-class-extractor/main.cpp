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

#include "MachoParser.h"
#include <vector>
#include "ClassDefinition.h"
#include "BinaryTableSearch.hpp"
#include "ClassExtractor.h"
#include <unordered_set>
#include <fstream>
#include <streambuf>
#include "File.h"

void printClassWithSubClasses(Modelling::ClassDefinition *classDef, short indention, std::ostream& output) {
    std::string indent = "|";
    short maxIndent = indention;
    if (indention == 0) {
        indent = "";
        --maxIndent;
    }
    for (short i = 0; i < maxIndent; ++i) {
        indent += " ";
    }
    const char *indentChar = indent.c_str();
    auto& subClasses = classDef->getSubClasses();
    auto& parentClasses = classDef->getParentClasses();
    output  << indentChar << "|-> " << classDef->getName().c_str() << " - parent classes: "
            << std::dec << parentClasses.size() << " - sub classes: " << subClasses.size()
            << " - type offset: 0x" << std::hex << classDef->getTypeOffset()
            << " - offset: 0x" << classDef->getOffset()
            << std::endl;
    if (parentClasses.size() > 0) {
        output << indentChar << "|      Parent classes:" << std::endl;
        for (auto& parent : parentClasses){
            output << indentChar << "|      |-> " << parent->getName().c_str() << " - type offset: 0x"
                    << std::hex << parent->getTypeOffset() << " - offset: 0x" << parent->getOffset()
                    << std::endl;
        }
        output << indentChar << "|" << std::endl;
    }

    auto& memberFuncs = classDef->getMemberFunctions();
    if (memberFuncs.size() > 0) {
        output << indentChar << "|      Member functions:" << std::endl;
        for (auto& func: memberFuncs){
            if (func.pureVirtual == true) {
                output << indentChar << "|      |-> " << "pure virtual" << std::endl;
            } else {
                output << indentChar << "|      |-> 0x" << std::hex << func.offset << std::endl;
            }
        }
        output << indentChar << "|" << std::endl;
    }
    for (auto& subClass: subClasses){
        printClassWithSubClasses(subClass, indention + 4, output);
    }
}


int main(int argc, const char *argv[]) {
    if (argv[1] == nullptr) {
        std::cout << "First argument: file to analyze, second argument optional for text outputpath." << std::endl;
        return 1;
    }
    std::string fileName(argv[1]);
    std::unique_ptr<IO::File> filePtr;
    try{
        filePtr = std::make_unique<IO::File>(fileName, IO::FILE_READ);
    }catch(IO::CouldNotAccessFileException const& e){
        std::cout << "Could not open target binary!" << std::endl;
        return 1;
    }
    std::string outputFilePath = "";
    if (argv[2] != nullptr) {
        outputFilePath = argv[2];
        size_t fileNamePos = fileName.find_last_of("/\\");           
        outputFilePath.append("/").append(fileName.substr(fileNamePos + 1).append(".classes.txt"));    
    }
    const size_t fileSize = filePtr->getFilSize();
    std::unique_ptr<byte_t[]> buff(filePtr->readData(0, fileSize));
    filePtr->close();

    MachO::MachoParser mp;
    if (mp.parseHeader(reinterpret_cast<address_t>(buff.get()), false,false)) {
        if (!mp.isArch64Bit()) {
            MachO::SectionInfo sectConst;
            MachO::SectionInfo sectData;
            try {
                sectConst = mp.getSectionInfo("__DATA", "__const");
                sectData = mp.getSectionInfo("__DATA", "__data");
            } catch(MachO::SectionNotFoundException const& e) {
                fprintf(stderr, "Section(s) not found\n");
                return 1;
            }

            auto relocEntries = mp.getRelocationEntries();
            const address_t slide = mp.getSlide();
            const address_t baseOffset = mp.getBaseOffset();
            const address_t baseSlide = slide - baseOffset;
            MachO::ClassExtractor refSearch(baseSlide);
            std::unordered_set<address_t> pureVirtualRefs;
            bool useDataSection = false;
            const address_t dataEnd = sectData.vmAddress + sectData.size;

            for (auto&entry : relocEntries){
                std::string& symname = entry.symbol->name;
                if (symname.find("class_type_info") != std::string::npos) {
                    for (auto&relocAddress : entry.relocationOffsets){
                        Modelling::ClassTypeE type;
                        if (symname.find("__class") != std::string::npos) {
                            type = Modelling::CT_BASE_CLASS;
                        } else if (symname.find("_si_class") != std::string::npos) {
                            type = Modelling::CT_SI_CLASS;
                        }
                        else {
                            type = Modelling::CT_MI_CLASS;
                        }
                        if ((!useDataSection) && (relocAddress.vmOffset > sectData.vmAddress) && (relocAddress.vmOffset < dataEnd)){
                            useDataSection = true;
                        }
                        Modelling::ClassDefinition classDef((*reinterpret_cast<char **>(relocAddress.fileOffset +
                                sizeof(address_t) + slide) + baseSlide), relocAddress.vmOffset, type);
                        refSearch.addClassSkeleton(std::move(classDef));
                    }
                } else if (symname.find("_pure_virtual")) {
                    for (auto&relocAddress : entry.relocationOffsets){
                        pureVirtualRefs.insert(relocAddress.vmOffset);
                    }
                }
            }
            refSearch.setPureVirtualReferences(std::move(pureVirtualRefs));
            address_t startAddress = sectConst.fileOffset;
            address_t endAddress = sectConst.fileOffset + sectConst.size;
            if (useDataSection){
                std::cout<<"use data"<<std::endl;
                if (sectData.fileOffset < sectConst.fileOffset) {
                    startAddress = sectData.fileOffset;
                }else{
                    endAddress = dataEnd;
                }
            }

            auto res = refSearch.constructClassGraph(reinterpret_cast<byte_t *>(startAddress + slide),
                    reinterpret_cast<byte_t *>(endAddress + slide));
            std::streambuf *buf;
            std::ofstream of;
            if (!outputFilePath.empty()) {
                of.open(outputFilePath);
                buf = of.rdbuf();
            } else {
                buf = std::cout.rdbuf();
            }

            std::ostream out(buf);
            for (auto it = std::begin(res); it != std::end(res); ++it){
                Modelling::ClassDefinition *classDef = it->second.get();
                if (classDef->getParentClasses().size() == 0) {
                    printClassWithSubClasses(classDef, 0, out);
                }
            }
            out << std::endl << "Total classes: " << std::dec << res.size() << std::endl;
            if (of.is_open()) {
                of.close();
            }

        }
    }
    return 0;
}

