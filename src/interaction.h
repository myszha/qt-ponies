#ifndef INTERACTION_H
#define INTERACTION_H

#include <QVariant>
#include <QString>

#include <vector>

#include "csv_parser.h"

class Interaction
{
public:
    Interaction(const std::vector<QVariant> &options);

    static const CSVParser::ParseTypes OptionTypes;

    const QString select_behavior();

    QString name;
    QString pony;
    float probability;
    int distance;
    QList<QVariant> targets;
    bool random;
    QList<QVariant> behaviors;
    int reactivation_delay;
};

#endif // INTERACTION_H
