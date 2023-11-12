#ifndef MacResourceLoader_h
#define MacResourceLoader_h

#include "ResourceLoader.h"

class MacResourceLoader: public ResourceLoader {
public:
    ResourcePtr LoadImprint() override;
    ResourcePtr LoadDefaultPalette() override;
};

#endif /* MacResourceLoader_h */
