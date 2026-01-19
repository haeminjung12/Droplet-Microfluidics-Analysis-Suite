#include <gtest/gtest.h>

#include "IAnalysisModule.h"

#include <cstddef>
#include <string>
#include <type_traits>

namespace {
class DummyAnalysisModule final : public IAnalysisModule {
public:
    QString getName() const override { return {}; }
    QString getDisplayName() const override { return {}; }

    void configure(const nlohmann::json&) override {}

    void run(InputSource&, AnalysisResults&, ProgressCallback callback) override {
        if(callback) {
            callback(0, 0, "ok");
        }
    }

    QWidget* createControlPanel() override { return nullptr; }
    void exportResults(const AnalysisResults&, const QString&) override {}
};

static_assert(std::is_abstract_v<IAnalysisModule>);

static_assert(std::is_same_v<decltype(&IAnalysisModule::getName), QString (IAnalysisModule::*)() const>);
static_assert(std::is_same_v<decltype(&IAnalysisModule::getDisplayName), QString (IAnalysisModule::*)() const>);
static_assert(std::is_same_v<decltype(&IAnalysisModule::configure), void (IAnalysisModule::*)(const nlohmann::json&)>);
static_assert(std::is_same_v<decltype(&IAnalysisModule::run),
    void (IAnalysisModule::*)(InputSource&, AnalysisResults&, ProgressCallback)>);
static_assert(std::is_same_v<decltype(&IAnalysisModule::createControlPanel), QWidget* (IAnalysisModule::*)()>);
static_assert(std::is_same_v<decltype(&IAnalysisModule::exportResults),
    void (IAnalysisModule::*)(const AnalysisResults&, const QString&)>);

static_assert(std::is_base_of_v<IAnalysisModule, DummyAnalysisModule>);
}  // namespace

TEST(SmokeTests, Compiles) { SUCCEED(); }

TEST(IAnalysisModuleSignatures, DummyModuleInstantiates) {
    DummyAnalysisModule module;
    (void)module;
    SUCCEED();
}
