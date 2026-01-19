#pragma once

#include "InputSource.h"

#include <cstddef>
#include <functional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#ifdef DROPLET_WITH_QT6
#include <QString>
#include <QWidget>
#else
class QString {
public:
    QString() = default;
    QString(const QString&) = default;
    QString(QString&&) noexcept = default;
    QString& operator=(const QString&) = default;
    QString& operator=(QString&&) noexcept = default;
    ~QString() = default;
};

class QWidget {
public:
    QWidget() = default;
    QWidget(const QWidget&) = default;
    QWidget(QWidget&&) noexcept = default;
    QWidget& operator=(const QWidget&) = default;
    QWidget& operator=(QWidget&&) noexcept = default;
    virtual ~QWidget() = default;
};
#endif

struct AnalysisResults;

using ProgressCallback =
    std::function<void(std::size_t current, std::size_t total, const std::string& status)>;

class IAnalysisModule {
public:
    virtual ~IAnalysisModule() = default;

    virtual QString getName() const = 0;
    virtual QString getDisplayName() const = 0;

    virtual void configure(const nlohmann::json& params) = 0;
    virtual void run(InputSource& input, AnalysisResults& output, ProgressCallback callback) = 0;

    virtual QWidget* createControlPanel() = 0;
    virtual void exportResults(const AnalysisResults& results, const QString& path) = 0;
};

