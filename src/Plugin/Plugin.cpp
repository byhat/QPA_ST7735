/**
 * @file Plugin.cpp
 * @brief Реализация фабрики плагина.
 */

#include "Plugin.hpp"
#include "Integration.hpp"

QPlatformIntegration *QpaSt7735Plugin::create(const QString &key,
                                              const QStringList &parameters)
{
    Q_UNUSED(key);
    Q_UNUSED(parameters);
    return new QpaSt7735Integration();
}
