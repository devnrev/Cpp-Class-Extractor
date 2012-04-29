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

#ifndef __File_H_
#define __File_H_

#include <string>

namespace IO{

class CouldNotAccessFileException{};

enum FileAccess{
    FILE_READ,
    FILE_WRITE,
    FILE_CREATE
};

class File {
public:
    File(std::string fileName,FileAccess accessType);
    ~File();
    size_t getFilSize();
    std::unique_ptr<byte_t[]> readData(address_t offset, size_t size) const;
    void writeData(address_t offset, void * data,size_t size);
    void appendData(void * data,size_t size);
    void close();
protected:
    FileAccess accessType_;
    std::string fileName_;
    FILE* fileHandle_;
    size_t fileSize_;

private:


};

}


#endif //__File_H_
