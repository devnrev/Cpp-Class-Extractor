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


#include "ClassDefinition.h"

namespace Modelling{

ClassDefinition::ClassDefinition(std::string name,address_t typeOffset, ClassTypeE classType) :
    name_(name),
    typeOffset_(typeOffset),
    classType_(classType),
    offset_(0),
    relations_(){

}

address_t ClassDefinition::getOffset() const {
    return offset_;
}

void ClassDefinition::setOffset(address_t offset) {
    offset_ = offset;
}

std::string const & ClassDefinition::getName() const {
    return name_;
}

void ClassDefinition::assignParentClass(ClassDefinition *classDef) {
    classDef->assignSubClass(this);
    relations_.addParentNode(classDef);
}

void ClassDefinition::assignSubClass(ClassDefinition *classDef) {
    relations_.addChildNode(classDef);
}

std::vector<ClassDefinition *> const & ClassDefinition::getParentClasses() {
    return relations_.getParentNodes();
}

std::vector<ClassDefinition *> const & ClassDefinition::getSubClasses() {
    return relations_.getChildNodes();
}

ClassDefinition::ClassDefinition(ClassDefinition&& obj) :
    name_(std::move(obj.name_)),
    typeOffset_(obj.typeOffset_),
    offset_(obj.offset_),
    functions_(std::move(obj.functions_)),
    classType_(obj.classType_),
    relations_(std::move(obj.relations_)){

}

void ClassDefinition::addMemberFunction(FunctionDefinition&& func) {
    functions_.push_back(std::move(func));
}

std::vector<FunctionDefinition> const & ClassDefinition::getMemberFunctions() const{
    return functions_;
}

ClassTypeE ClassDefinition::getClassType() const {
    return classType_;
}

address_t ClassDefinition::getTypeOffset() const {
    return typeOffset_;
}



}