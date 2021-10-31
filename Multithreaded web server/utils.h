#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <unordered_map>
#include "my_timestamp.h"

typedef std::unordered_map<std::string, std::string> Section;
typedef std::unordered_map<std::string, Section> INIFile;

bool hasEnding(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

std::vector<std::string> SplitSanjay(const std::string &str, const char delim)
{
    std::vector<std::string> retVal;
    size_t start = 0;
    size_t delimLoc = str.find_first_of(delim, start);
    while (delimLoc != std::string::npos)
    {
        retVal.emplace_back(str.substr(start, delimLoc - start));
        start = delimLoc + 1;
        delimLoc = str.find_first_of(delim, start);
    }
    retVal.emplace_back(str.substr(start));
    return retVal;
}

INIFile ParseINI(const std::string &filename)
{
    INIFile result;
    ifstream file(filename);
    string line;
    string sectionName;
    while (getline(file >> ws, line))
    {
        if (line[0] == ';')
        {
            continue;
        }
        else if (line[0] == '[')
        {
            sectionName = line.substr(1, line.size() - 2);
            result.emplace(sectionName, Section());
        }
        else if (!line.empty())
        {
            Section &currSection = result[sectionName];
            auto parsedLine = SplitSanjay(line, '=');
            currSection.emplace(move(parsedLine[0]), move(parsedLine[1]));
        }
    }
    return result;
}

bool FileExist(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

string GetTimeFormatted()
{
    return '[' + get_timestamp_now() + ']';
}
