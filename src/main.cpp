#include <iostream>
#include <vector>
#include <memory>
#include "core/Types.h"
#include "core/IModule.h"
#include "indicators/IndicatorProvider.h"
#include "logic/BumpDetector.h"
#include "logic/TRStateMachine.h"
int main() {
    std::cout << "=== SRP Architecture Skeleton v1.0 ===" << std::endl;
    std::vector<std::unique_ptr<IModule>> modules;
    modules.push_back(std::make_unique<IndicatorProvider>());
    modules.push_back(std::make_unique<BumpDetector>());
    modules.push_back(std::make_unique<TRStateMachine>());
    // Имитация Replay Engine
    IndicatorSnapshot dummy_bar{};
    dummy_bar.close_time = std::chrono::system_clock::now();
    std::cout << "Running Replay Loop..." << std::endl;
    for (auto& module : modules) {
        module->OnBarClose(dummy_bar);
    }
    std::cout << "Done." << std::endl;
    return 0;
}
