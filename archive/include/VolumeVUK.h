#ifndef VOLUME_VUK_H
#define VOLUME_VUK_H

#include <vector>
#include "DataManager.h"

struct VUK_Level {
    size_t index;
    double price;
    long long volume;
    bool isSupport;
};

class VolumeVUK {
public:
    static std::vector<VUK_Level> analyze(const std::vector<Bar>& bars, long long volumeThreshold = 4000);
};

#endif
