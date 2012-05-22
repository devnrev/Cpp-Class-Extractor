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

#ifndef symboltable_parser_ClassDefinition_h
#define symboltable_parser_ClassDefinition_h

#include <vector>
#include "DualTree.hpp"
#include "Types.h"
#include <string>

namespace Modelling{

struct FunctionDefinition{
    address_t offset;
    bool pureVirtual;
};

enum ClassTypeE{
    CT_BASE_CLASS,
    CT_SI_CLASS,
    CT_MI_CLASS
};


class ClassDefinition{
public:
    ClassDefinition(){};
    ClassDefinition(std::string name,address_t typeOffset, ClassTypeE classType);
    ClassDefinition(ClassDefinition&& obj);

    void addMemberFunction(FunctionDefinition&& func);
    std::vector<FunctionDefinition> const & getMemberFunctions() const;
    ClassTypeE getClassType() const;
    address_t getTypeOffset() const;
    address_t getOffset() const;
    void setOffset(address_t offset);
    std::string const& getName() const;
    void assignParentClass(ClassDefinition* classDef);
    void assignSubClass(ClassDefinition* classDef);
    std::vector<ClassDefinition *> const& getParentClasses();
    std::vector<ClassDefinition *> const& getSubClasses();

private:    
    std::vector<FunctionDefinition> functions_;
    ClassTypeE classType_;
    address_t typeOffset_;
    address_t offset_;
    std::string name_;
    DualTreeNode<ClassDefinition> relations_;
};

}

#endif
