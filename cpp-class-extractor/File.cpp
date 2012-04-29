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

#include "File.h"

namespace IO{

File::File(std::string fileName, IO::FileAccess accessType) :
fileName_(std::move(fileName)),
accessType_(accessType) {
    std::string accessor = "r";
    switch (accessType) {
        case FILE_WRITE:
            accessor += "+";
            break;
        case FILE_CREATE:
            accessor = "w+";
            break;
        default:
            break;
    }
    fileHandle_ = fopen(fileName_.c_str(), accessor.c_str());
    if (fileHandle_ == 0) {
        throw CouldNotAccessFileException();
    }
    fseek(fileHandle_, 0, SEEK_END);
    fileSize_ = ftell(fileHandle_);
}

File::~File() {
    fclose(fileHandle_);
}

size_t File::getFilSize() {
    return fileSize_;
}

std::unique_ptr<byte_t[]> File::readData(address_t offset, size_t size) const {
    fseek(fileHandle_, offset, SEEK_SET);
    std::unique_ptr<byte_t[]> buf(new byte_t[size]);
    fread(buf.get(), 1, size, fileHandle_);
    return std::move(buf);
}

void File::writeData(address_t offset, void *data, size_t size) {
    fseek(fileHandle_, offset, SEEK_SET);
    appendData(data, size);
}

void File::appendData(void *data, size_t size) {
    fwrite(data, 1, size, fileHandle_);
}

void File::close() {
    fclose(fileHandle_);
}


}