#pragma once

#include "ResourceLoader.h"

class WinResourceLoader: public ResourceLoader {
public:
	ResourcePtr LoadImprint() override;
	ResourcePtr LoadDefaultPalette() override;
};