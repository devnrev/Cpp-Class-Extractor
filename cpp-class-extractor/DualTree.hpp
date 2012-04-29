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

#ifndef __DualTree_H_
#define __DualTree_H_

#include <vector>


template<class T>
class DualTreeNode {
public:
    DualTreeNode(){}

    DualTreeNode(DualTreeNode<T>&& node): parents_(std::move(node.parents_)),children_(std::move(node.children_)){}

    void addParentNode(T* parentNode){
        parents_.push_back(parentNode);
    }

    void addChildNode(T* childNode){
        children_.push_back(childNode);
    }

    std::vector<T*> const & getParentNodes() const{
        return parents_;
    }

    std::vector<T*> const & getChildNodes() const {
        return children_;
    }

private:
    std::vector<T*> parents_;
    std::vector<T*> children_;
};

#endif //__DualTree_H_
