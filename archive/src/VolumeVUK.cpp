#include "VolumeVUK.h"

std::vector<VUK_Level> VolumeVUK::analyze(const std::vector<Bar>& bars, long long volumeThreshold) {
    std::vector<VUK_Level> levels;
    if (bars.empty()) return levels;

    for (size_t i = 1; i < bars.size() - 1; ++i) {
        if (bars[i].tick_volume >= volumeThreshold) {
            VUK_Level lvl;
            lvl.index = i;
            lvl.price = (bars[i].high + bars[i].low) / 2.0;
            lvl.volume = bars[i].tick_volume;
            lvl.isSupport = (bars[i].close >= bars[i].open);
            levels.push_back(lvl);
        }
    }
    return levels;
}
