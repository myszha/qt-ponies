#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <vector>
#include <QString>
#include <QList>
#include <QVariant>
#include <unordered_map>

class CSVParser {
public:
    typedef std::vector<std::pair<std::string, QVariant::Type>> ParseTypes;
    static void ParseLine(std::vector<QVariant> &record, const QString& line, QChar delimiter);
    static void AddParseTypes(const QString& key, const std::vector<std::pair<std::string, QVariant::Type>> & types);

private:
    CSVParser();
    static std::unordered_map<QString, const ParseTypes &> parse_types;
};

#endif
