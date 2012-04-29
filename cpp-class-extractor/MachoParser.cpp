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

#include "MachoParser.h"
#include "MachOTypes.hpp"
#include <mach-o/nlist.h>
#include <string>
#include <mach-o/reloc.h>
#include <mach-o/fat.h>
#include <machine/endian.h>

namespace MachO{


MachoParser::MachoParser():
arch64_(false) {

}

MachoParser::~MachoParser() {

}

bool MachoParser::parseHeader(address_t data, bool runtime, bool choose64BitIfPossible) {
    if (*reinterpret_cast<uint32_t *>(data) == htonl(FAT_MAGIC)){
        std::cout << "Fat Binary - choose 64 Bit: " << choose64BitIfPossible << std::endl;
        const struct fat_header* fatHeader = reinterpret_cast<const struct fat_header *>(data);
        const uint32_t numArchs = htonl(fatHeader->nfat_arch);
        const struct fat_arch* fatArch =  reinterpret_cast<const struct fat_arch *>(data + sizeof(struct fat_header));
        bool foundArch = false;
        for(uint32_t i = 0; i < numArchs; ++i){
            if(choose64BitIfPossible && (htonl(fatArch[i].cputype) == CPU_TYPE_X86_64)){
                data+= htonl(fatArch[i].offset);
                arch64_ = true;
                foundArch = true;
                break;
            }else if (!choose64BitIfPossible && (htonl(fatArch[i].cputype) == CPU_TYPE_I386)){
                data+= htonl(fatArch[i].offset);
                foundArch = true;
                break;
            }
        }
        if(!foundArch){
            return false;
        }
    }

    const uint32_t *magic = reinterpret_cast<uint32_t *>(data);
    int numCommands = 0;
    address_t address = 0;

    if (*magic == MH_MAGIC) {
        const struct mach_header *header = reinterpret_cast<const struct mach_header *>(data);
        numCommands = header->ncmds;
        address = data + sizeof(struct mach_header);
    }
#if __LP64__
    else if (*magic == MH_MAGIC_64) {
        arch64_ = true;
        const struct mach_header_64 *header = reinterpret_cast<struct mach_header_64 *>(data);
        numCommands = header->ncmds;
        address = data + sizeof(struct mach_header_64);
    } 
#endif
    else {
        return false;
    }

    const char TextSeg[] = "__TEXT";

    const char LinkeditSeg[] = "__LINKEDIT";

    struct load_command *lc = reinterpret_cast<struct load_command * >(address);
    bool symTabExists = false;
    dynamicSize_ = 0;
    relocation_ = data;
    for (int i = 0; i < numCommands; ++i) {
        switch (lc->cmd) {
            case LC_SEGMENT:
            {
                const struct segment_command *sg = reinterpret_cast<const struct segment_command * >(lc);
                if (strncmp(sg->segname, TextSeg, 6) == 0) {
                    baseOffset_ = sg->vmaddr;
                } else if (strncmp(sg->segname, LinkeditSeg, 10) == 0) {
                    dynamicSize_ = sg->vmaddr - baseOffset_ - sg->fileoff;
                }
                const struct section *sect = reinterpret_cast<const struct section * >(reinterpret_cast<const byte_t * >(sg) + sizeof(segment_command));
                const uint32_t numSections = sg->nsects;
                for (int j = 0; j < numSections; ++j) {
                    sections_.push_back(SectionInfo(
                    {
                        .segName = sect[j].segname,
                        .sectName = sect[j].sectname,
                        .vmAddress = sect[j].addr,
                        .fileOffset = sect[j].offset,
                        .size = sect[j].size
                    }));
                }
                break;
            }
#if __LP64__
            case LC_SEGMENT_64:
            {
                const struct segment_command_64 *sg = reinterpret_cast<const struct segment_command_64 * >(lc);
                if (strncmp(sg->segname, TextSeg, 6) == 0) {
                    baseOffset_ = sg->vmaddr;
                } else if (strncmp(sg->segname, LinkeditSeg, 10) == 0) {
                    dynamicDataSize = sg->vmaddr - baseOffset_ - sg->fileoff;
                }
                const struct section_64 *sect = reinterpret_cast<const struct section_64 * >(reinterpret_cast<const byte_t * >(sg) + sizeof(segment_command_64));
                const uint32_t numSections = sg->nsects;
                for (int j = 0; j < numSections; ++j) {
                    sections_.push_back(SectionInfo(
                    {
                        .segName = sect[j].segname,
                        .sectName = sect[j].sectname,
                        .vmAddress = sect[j].addr,
                        .fileOffset = sect[j].offset,
                        .size = sect[j].size
                    }));
                }
                break;
            }
#endif
            case LC_SYMTAB:
            {
                const struct symtab_command *st = reinterpret_cast<const struct symtab_command * >(lc);
                symbolTable_.offset = st->symoff + relocation_;
                symbolTable_.numberOfSymbols = st->nsyms;
                symbolTable_.stringOffset = st->stroff + relocation_;
                symbolTable_.stringSize = st->strsize;
                if (runtime) {
                    symbolTable_.offset += dynamicSize_;
                    symbolTable_.stringOffset += dynamicSize_;
                }
                symTabExists = true;
                break;
            }
            case LC_DYSYMTAB:
            {
                const struct dysymtab_command *dst = reinterpret_cast<const struct dysymtab_command * >(lc);
                dynamicSymbolTable_.externalRelocationTable = dst->extreloff + relocation_;
                dynamicSymbolTable_.numberOfExternalRelocs = dst->nextrel;
                break;
            }
            default:
                break;
        }
        lc = reinterpret_cast<struct load_command * >(reinterpret_cast<byte_t * >(lc) + lc->cmdsize);
    }
    if (!symTabExists) {
        return false;
    }
    if (runtime) {
        aslrOffset_ = data - baseOffset_;
    }
    return true;
}


std::vector <std::shared_ptr<SymbolInfo > > MachoParser::getSymbolByName(std::string name, bool exact) {
    parseSymbolTable();
    std::vector < std::shared_ptr<SymbolInfo > > symbolOccurences;
    for (auto&symbol : symbols_){
        if (exact) {
            if (name.compare(symbol->name) == 0) {
                symbolOccurences.push_back(symbol);
            }
        } else {
            if (symbol->name.find(name) != std::string::npos) {
                symbolOccurences.push_back(symbol);
            }
        }
    }
    return symbolOccurences;
}

void MachoParser::parseSymbolTable() {
    if (symbols_.capacity() > 0) {
        return;
    }
    int symbolCount = 0;
    char *symPtr = reinterpret_cast<char * >(symbolTable_.offset);
    address_t stringOffset = symbolTable_.stringOffset;
    if (!arch64_) {
        while (symbolCount < symbolTable_.numberOfSymbols) {
            const struct nlist *symbol = reinterpret_cast<struct nlist * >(symPtr);
            const char *currrentSymbolName = reinterpret_cast<const char *>(symbol->n_un.n_strx + stringOffset);
            symbols_.push_back(std::make_shared < SymbolInfo > (SymbolInfo(
            {
                symbol->n_value, currrentSymbolName
            })));
            symPtr += sizeof(struct nlist);
            symbolCount++;
        }
    }
#if __LP64__
    else {
        while (symbolCount < symbolTable_.numberOfSymbols) {
            const struct nlist_64 *symbol = reinterpret_cast<const struct nlist_64 * >(symPtr);
            const char *currrentSymbolName = reinterpret_cast<const char *>(symbol->n_un.n_strx + stringOffset);
            symbols_.push_back(std::make_shared<SymbolInfo>(SymbolInfo({symbol->n_value, currrentSymbolName})));
            symPtr += sizeof(struct nlist_64);
            symbolCount++;
        }        
    }
#endif
}

std::vector <RelocationInfo>  MachoParser::getRelocationEntries() {
    parseSymbolTable();
    std::vector <RelocationInfo> externalRelocs;
    address_t offset = dynamicSymbolTable_.externalRelocationTable;
    for (int i = 0; i < dynamicSymbolTable_.numberOfExternalRelocs; ++i) {
        const struct relocation_info *relocationInfo = reinterpret_cast<relocation_info * >(offset);
        std::shared_ptr <SymbolInfo> symPtr = symbols_[relocationInfo->r_symbolnum];
        RelocationInfo& relocInfo = externalRelocs.back();
        address_t fileOffset = relocationInfo->r_address - baseOffset_;
        if ((externalRelocs.empty()) || (symPtr != relocInfo.symbol)) {
            externalRelocs.push_back(RelocationInfo(
            {
                .symbol = symPtr
            }));
            externalRelocs.back().relocationOffsets.push_back(AddressInfo(
            {
                static_cast<address_t>(relocationInfo->r_address), fileOffset
            }));
        } else {
            relocInfo.relocationOffsets.push_back(AddressInfo(
            {
                static_cast<address_t>(relocationInfo->r_address), fileOffset
            }));
        }
        offset += sizeof(struct relocation_info);
    }
    return externalRelocs;
}


address_t MachoParser::getBaseOffset() {
    return baseOffset_;
}

address_t MachoParser::getSlide() {
    return relocation_;
}

bool MachoParser::isArch64Bit() {
    return arch64_;
}

size_t MachoParser::getDynamicSegmentSize() {
    return dynamicSize_;
}

SectionInfo MachoParser::getSectionInfo(std::string segName, std::string sectName) const {
    for (auto&section : sections_){
        if (segName.compare(section.segName) == 0) {
            if (sectName.compare(section.sectName) == 0) {
                return section;
            }
        }
    }
    throw SectionNotFoundException();
}


}