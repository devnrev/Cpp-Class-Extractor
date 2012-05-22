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

Information:

Cpp-Class-Extractor tries to visualize the internal class hierarchy of a machO binary.
It is based on the information given by virtual tables, which the compiler generates for classes with virtual functions. The output contains class names and member functions of each class.

Usage of the standalone commandline program:

cpp-class-extractor argument1 argument2

argument1 - REQUIRED - : 

path to the file you would like to analyze


argument2 - OPTIONAL - : 

specify a path were cpp-class-extractor should create a file  based output. Note that nothing will be printed into the console.


This is an early version so it will likely contain some bugs. It is planned for the future to act as an plugin for IDA. Also there will be graphical representation of the class structure at some time.

Contact:

Twitter: (at) devnrev 

Version:
    0.1     - 32 Bit support
            - basic textual output


