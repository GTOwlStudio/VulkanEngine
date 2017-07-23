#pragma once

#include <string>
#include <vector>
#include <vulkan\vulkan.h>

namespace helper {
	
	std::string flagsToString(VkBufferUsageFlags flags, std::string sufix = "", std::string prefix = " | ");
	bool nameUsed(std::string nameProposed, std::vector<std::string> collection); //Check if a named is already used or not
	uint32_t stringToId(std::string id, std::vector<std::string> collection);
	uint32_t hexaToInt(std::string hexa);
	uint32_t countCharacter(std::string lookInto, char characterToCount);
	size_t find(std::string str, std::vector<std::string> strArr);

	template<typename T>
	T iterate(std::string str, std::vector< T> collection, std::vector<std::string> namesCollection) {
		if (collection.size()!=namesCollection.size()) {
			return nullptr;
		}
		for (size_t i = 0; i < namesCollection.size();i++) {
			if (namesCollection[i]==str) {
				return collection[i];
			}
		}
		return nullptr;
	}

	template<typename T> //Does col contains obj
	bool contains(T obj, std::vector<T> col) {
		for (size_t i = 0; i < col.size(); i++) {
			if (col[i] == obj) {
				return true;
			}
		}
		return false;
	}
}