#pragma once

#include <memory>

struct ResourceHeader {
    uint32_t t;            // ???
    uint32_t inSize;
    uint32_t outSize;
};

using ResourcePtr = std::unique_ptr<unsigned char[]>; //std::unique_ptr<std::byte[]>;

struct ResourceLoader {
	virtual ResourcePtr LoadImprint() = 0;
	virtual ResourcePtr LoadDefaultPalette() = 0;
	virtual ~ResourceLoader() {}
};
