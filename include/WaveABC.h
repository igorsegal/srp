#ifndef WAVE_ABC_H
#define WAVE_ABC_H

#include <vector>
#include <string>

// Bar определяется один раз — в DataManager.h.
// Здесь его определять нельзя: двойное определение даёт ошибку C2011
// и каскад ошибок C2027/C2397 в main.cpp и BacktestEngine.cpp.
#include "DataManager.h"

namespace WaveABC {

    using ::Bar;

    struct ABC_Pattern {
        bool isValid;
        bool isBullish;

        double pointA;
        double pointB;
        double pointC;
        double priceA;
        double priceB;
        double priceC;

        int barIndexA;
        int barIndexB;
        int barIndexC;
        int indexA;
        int indexB;
        int indexC;
    };

    enum class ShiftType {
        LEFT,
        EXACT,
        RIGHT
    };

    enum class WaveCStatus {
        ACTIVE,
        EXTENDING,
        COMPLETED
    };

    struct WaveABCData {
        bool isValid;
        ShiftType tfShift;
        WaveCStatus cStatus;
        double startPoint;
        double targetPrice100;
        double targetPrice162;
        bool lowPotentialBroken;
    };

    std::vector<ABC_Pattern> detect(const std::vector<Bar>& bars);
    std::vector<ABC_Pattern> detect(const std::vector<Bar>& bars, double param1, double param2);

    class Analyzer {
    private:
        int maxAllowedBarShift;

    public:
        Analyzer(int barShift = 4) : maxAllowedBarShift(barShift) {}

        WaveABCData AnalyzeTimeframeSynthesis(int higherTF_TP_bar, int lowerTF_WaveA_bar, double startPrice) {
            WaveABCData data;
            data.isValid = true;
            data.startPoint = startPrice;
            data.lowPotentialBroken = false;
            data.cStatus = WaveCStatus::ACTIVE;

            int barDifference = higherTF_TP_bar - lowerTF_WaveA_bar;
            if (barDifference < -1) {
                data.tfShift = ShiftType::LEFT;
            } else if (barDifference > 1) {
                data.tfShift = ShiftType::RIGHT;
            } else {
                data.tfShift = ShiftType::EXACT;
            }
            return data;
        }

        void UpdateWaveCStatus(WaveABCData& data, double currentPrice, double oppositeTP2, bool isLowPotentialHit) {
            data.lowPotentialBroken = isLowPotentialHit;
            if (currentPrice <= oppositeTP2 || isLowPotentialHit) {
                data.cStatus = WaveCStatus::COMPLETED;
            } else if (currentPrice > data.startPoint && data.cStatus != WaveCStatus::COMPLETED) {
                data.cStatus = WaveCStatus::EXTENDING;
            }
        }

        void CalculateTargets(WaveABCData& data, double waveALength) {
            data.targetPrice100 = data.startPoint + waveALength * 1.00;
            data.targetPrice162 = data.startPoint + waveALength * 1.62;
        }
    };

} // namespace WaveABC

#endif // WAVE_ABC_H