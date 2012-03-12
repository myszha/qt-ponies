#include <QPoint>

#include <iostream>

#include "csv_parser.h"

namespace std
{
        template <>
        struct hash<QString>
        {
            size_t operator()(const QString& s) const
            {
                return qHash(s);
            }
        };
}

std::unordered_map<QString, const CSVParser::ParseTypes &> CSVParser::parse_types;

static QVariant convert_type(std::pair<std::string, QVariant::Type> type, QString value)
{
    switch(type.second){
        case QVariant::Type::String: {
            return QVariant(value);
        }
        case QVariant::Type::Int: {
            bool ok = 0;
            int v = value.toInt(&ok);
            if(ok == false){
                std::cerr << "ERROR parsing variable '"<< type.first << "' value: '" << value.toStdString() << "' to Int." << std::endl;
            }
            return QVariant(v);
        }
        case QVariant::Type::Bool: {
            QVariant v(value);
            if(value.compare("true", Qt::CaseInsensitive) != 0 && value.compare("false", Qt::CaseInsensitive) != 0 ) {
                std::cerr << "ERROR parsing variable '"<< type.first << "' value: '" << value.toStdString() << "' to Bool." << std::endl;
            }
            if(v.convert(QVariant::Type::Bool) == false){
                std::cerr << "ERROR parsing variable '"<< type.first << "' value: '" << value.toStdString() << "' to Bool." << std::endl;
            }
            return v;
        }
        case QVariant::Type::Double: {
            bool ok = 0;
            float v = value.toFloat(&ok);
            if(ok == false){
                std::cerr << "ERROR parsing variable '"<< type.first << "' value: '" << value.toStdString() << "' to Float." << std::endl;
            }
            return QVariant(v);
        }
        case QVariant::Type::Point: {
            int x=0,y=0;
            bool ok = 0;
            x = value.section(',',0,0).toInt(&ok);
            if(ok == false){
                std::cerr << "ERROR parsing x of variable '"<< type.first << "' value: '" << value.toStdString() << "' to Point." << std::endl;
            }
            y = value.section(',',1,1).toInt();
            if(ok == false){
                std::cerr << "ERROR parsing y of variable "<< type.first << "' value: '" << value.toStdString() << "' to Point." << std::endl;
            }
            return QVariant(QPoint(x,y));
        }
        default: {
            std::cerr << "ERROR: unknown type " << type.second << " while parsing value '" << value.toStdString() << "' for variable "<< type.first <<"."<< std::endl;
            return QVariant(value);
        }
    }
}

void CSVParser::AddParseTypes(const QString& key, const std::vector<std::pair<std::string, QVariant::Type>> & types){
    parse_types.insert({key.toLower(), types});
}

void CSVParser::ParseLine(std::vector<QVariant> &record, const QString& line, QChar delimiter)
{
    int linepos=0;
    int inquotes=false;
    int inbrackets=false;
    QChar c;
    int linemax=line.length();
    QString curstring;
    QList<QVariant> temp_list;
    int types_size = 0;
    int type_pos = 0;
    QString line_type;
    record.clear();

    while(linepos < linemax)
    {
        c = line[linepos];

        if (!inquotes && curstring.length()==0 && c=='"')
        {
            //beginquotechar
            inquotes=true;
        }
        else if (!inquotes && curstring.length()==0 && c=='{')
        {
            //beginlistchar
            inbrackets = true;
        }
        else if (!inquotes && inbrackets && c==delimiter)
        {
            // end of field in list
            if(!line_type.isEmpty() && types_size > type_pos) {
                temp_list.push_back(convert_type(parse_types.at(line_type)[type_pos],curstring));
            }else{
                temp_list.push_back(curstring);
            }
            curstring="";

        }
        else if (inquotes && c=='"')
        {
            //quotechar
            if ( (linepos+1 <linemax) && (line[linepos+1]=='"') )
            {
                //encountered 2 double quotes in a row (resolves to 1 double quote)
                curstring.push_back(c);
                linepos++;
            }
            else
            {
                //endquotechar
                inquotes=false;
            }
        }
        else if (!inquotes && inbrackets && c=='}')
        {
            // end of list
            inbrackets = false;

            if(!line_type.isEmpty() && types_size > type_pos) {
                temp_list.push_back(convert_type(parse_types.at(line_type)[type_pos],curstring));
            }else{
                temp_list.push_back(curstring);
            }
            // next type
            type_pos++;

            curstring = "";
            record.push_back(temp_list);
            temp_list.clear();
            linepos++; // skip the delimiter that follows '}'
        }
        else if (!inquotes && !inbrackets && c==delimiter)
        {
            //end of field
            if(!line_type.isEmpty() && types_size > type_pos) {
                record.push_back(convert_type(parse_types.at(line_type)[type_pos],curstring));
            }else{
                record.push_back(curstring);
            }

            // we now know our linetype
            // only take the first argument as type
            if(type_pos == 0 && line_type.isEmpty() && (parse_types.find(curstring.toLower()) != parse_types.end())){
                line_type = curstring.toLower();
                types_size = parse_types.at(line_type).size();
            }

            type_pos++;
            curstring="";
        }
        else if (!inquotes && /*(c=='\r' || c=='\n')*/ linepos+1 == linemax)
        {
            // end of line
            curstring.push_back(c);

            if(!line_type.isEmpty() && types_size > type_pos) {
                record.push_back(convert_type(parse_types.at(line_type)[type_pos],curstring));
            }else{
                record.push_back(curstring);
            }

            type_pos++;
            return;
        }
        else
        {
            curstring.push_back(c);
        }
        linepos++;
    }
    if(!line_type.isEmpty() && types_size > type_pos) {
        record.push_back(convert_type(parse_types.at(line_type)[type_pos],curstring));
    }else{
        record.push_back(curstring);
    }

    return;
}

void CSVParser::ParseLine(std::vector<QVariant> &record, const QString& line, QChar delimiter, const ParseTypes &types)
{
    int linepos=0;
    int inquotes=false;
    int inbrackets=false;
    QChar c;
    int linemax=line.length();
    QString curstring;
    QList<QVariant> temp_list;
    int types_size = types.size();
    int type_pos = 0;
    record.clear();

    while(linepos < linemax)
    {
        c = line[linepos];

        if (!inquotes && curstring.length()==0 && c=='"')
        {
            //beginquotechar
            inquotes=true;
        }
        else if (!inquotes && curstring.length()==0 && c=='{')
        {
            //beginlistchar
            inbrackets = true;
        }
        else if (!inquotes && inbrackets && c==delimiter)
        {
            // end of field in list
            if(types_size > type_pos) {
                temp_list.push_back(convert_type(types[type_pos],curstring));
            }else{
                temp_list.push_back(curstring);
            }
            curstring="";

        }
        else if (inquotes && c=='"')
        {
            //quotechar
            if ( (linepos+1 <linemax) && (line[linepos+1]=='"') )
            {
                //encountered 2 double quotes in a row (resolves to 1 double quote)
                curstring.push_back(c);
                linepos++;
            }
            else
            {
                //endquotechar
                inquotes=false;
            }
        }
        else if (!inquotes && inbrackets && c=='}')
        {
            // end of list
            inbrackets = false;

            if(types_size > type_pos) {
                temp_list.push_back(convert_type(types[type_pos],curstring));
            }else{
                temp_list.push_back(curstring);
            }
            // next type
            type_pos++;

            curstring = "";
            record.push_back(temp_list);
            temp_list.clear();
            linepos++; // skip the delimiter that follows '}'
        }
        else if (!inquotes && !inbrackets && c==delimiter)
        {
            //end of field
            if(types_size > type_pos) {
                record.push_back(convert_type(types[type_pos],curstring));
            }else{
                record.push_back(curstring);
            }

            type_pos++;
            curstring="";
        }
        else if (!inquotes && /*(c=='\r' || c=='\n')*/ linepos+1 == linemax)
        {
            // end of line
            curstring.push_back(c);

            if(types_size > type_pos) {
                record.push_back(convert_type(types[type_pos],curstring));
            }else{
                record.push_back(curstring);
            }

            type_pos++;
            return;
        }
        else
        {
            curstring.push_back(c);
        }
        linepos++;
    }
    if(types_size > type_pos) {
        record.push_back(convert_type(types[type_pos],curstring));
    }else{
        record.push_back(curstring);
    }

    return;
}
