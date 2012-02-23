#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>

void csvline_populate(std::vector<std::string> &record, const std::string& line, char delimiter)
{
    int linepos=0;
    int inquotes=false;
    char c;
    int linemax=line.length();
    std::string curstring;
    record.clear();
       
    while(line[linepos]!=0 && linepos < linemax)
    {
       
        c = line[linepos];
       
        if (!inquotes && curstring.length()==0 && c=='"')
        {
            //beginquotechar
            inquotes=true;
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
        else if (!inquotes && c==delimiter)
        {
            //end of field
            record.push_back( curstring );
            curstring="";
        }
        else if (!inquotes && (c=='\r' || c=='\n') )
        {
            record.push_back( curstring );
            return;
        }
        else
        {
            curstring.push_back(c);
        }
        linepos++;
    }
    record.push_back( curstring );
    return;
}

#endif
