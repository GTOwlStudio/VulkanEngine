#include "Helper.h"

namespace helper {
	std::string flagsToString(VkBufferUsageFlags flags, std::string sufix, std::string prefix) {
		VkBufferUsageFlagBits fb[10] = { VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
			VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
			VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM};

		std::string fb_string[10] = { "VK_BUFFER_USAGE_TRANSFER_SRC_BIT",
			"VK_BUFFER_USAGE_TRANSFER_DST_BIT",
			"VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT",
			"VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT",
			"VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT",
			"VK_BUFFER_USAGE_STORAGE_BUFFER_BIT",
			"VK_BUFFER_USAGE_INDEX_BUFFER_BIT",
			"VK_BUFFER_USAGE_VERTEX_BUFFER_BIT",
			"VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT",
			"VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM"};

		std::string s = "";
		VkFlags tmp = flags;
		for (size_t i = 0; i < 10;i++) {
			if (tmp<(uint32_t)fb[i]) {
				s += sufix + fb_string[i-1] + prefix;
				tmp -= fb[i-1];
				if (tmp == 0) {	break;	}
				i = 0;
			}
		}
		return s.substr(0,s.size()-prefix.size());
	}
}