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

#ifndef _StdExtension____FILEEXTENSION___
#define _StdExtension____FILEEXTENSION___

namespace std{

template <typename T, typename ...Args >
std::unique_ptr<T> make_unique(Args&& ...args){
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}


#endif