#include "interaction.h"

#include <QDateTime>

#include <random>
#include <iostream>

const CSVParser::ParseTypes Interaction::OptionTypes {
    {              "InteractionName", QVariant::Type::String },
    {                     "PonyName", QVariant::Type::String },
    {                  "probability", QVariant::Type::Double },
    {"proximity_activation_distance", QVariant::Type::Int    },
    {                    "{Targets}", QVariant::Type::String },
    {                "random_or_all", QVariant::Type::Bool   },
    {                  "{behaviors}", QVariant::Type::String },
    {           "reactivation_delay", QVariant::Type::Int    }
};


Interaction::Interaction(const std::vector<QVariant> &options)
{
    /*
        InteractionName = 0
        PonyName = 1
        probability = 2
        proximity_activation_distance = 3
        {Targets} = 4
        random_or_all = 5
        {behaviors} = 6
        reactivation_delay = 7
    */

    if(options.size() != 8) {
        std::cout << "ERROR: interaction contains wrong number of options" << std::endl;
        throw std::exception();
    }

    name = options[0].toString().toLower();
    pony = options[1].toString();
    probability = options[2].toFloat();
    distance = options[3].toFloat();
    targets = qVariantValue<QList<QVariant>>(options[4]);
    random = options[5].toBool();
    behaviors = qVariantValue<QList<QVariant>>(options[6]);
    reactivation_delay = options[7].toBool();
}

const QString Interaction::select_behavior()
{
    std::mt19937 gen(QDateTime::currentMSecsSinceEpoch());
    std::uniform_int_distribution<> dis(0, behaviors.count()-1);
    return behaviors[dis(gen)].toString();
}
