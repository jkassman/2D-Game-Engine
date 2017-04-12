#include "jacobJSON.hpp"

#include <iostream>

using namespace std;

//destroys original list
//expects every item in the list to be surrounded by {}
void parseJsonList(string *jsonString, vector<string> *toFill)
{
    if (jsonString->at(0) != '[')
    {
        cerr << "Invalid JSON passed to parseJsonList" << endl;
        return;
    }
    jsonString->erase(0, 1); //erase the [

    while (jsonString->at(0) != ']')
    {
        if (jsonString->at(0) == ',')
        {
            jsonString->erase(0, 1);
        }
        toFill->push_back(grabJsonObject(jsonString));
        //cout << toFill->back() << endl;
    }
    jsonString->erase(0, 1); //erase the ]
}

//object must be surrounded by {} or []
string grabJsonObject(string *jsonString)
{
    if ((jsonString->at(0) != '{') && (jsonString->at(0) != '['))
    {
        cerr << "Invalid JSON passed to grabJsonObject!" << endl;
        return *jsonString;
    }
    int levelsDown = 0;
    string toReturn;
    do 
    {
        toReturn.push_back(jsonString->at(0));
        if ((jsonString->at(0) == '}') || (jsonString->at(0) == ']'))
        {
            levelsDown--;
        }
        else if ((jsonString->at(0) == '{') || (jsonString->at(0) == '['))
        {
            levelsDown++;
        }
        jsonString->erase(0, 1);
    } while (levelsDown > 0);
    return toReturn;
}

//does NOT modify jsonString
//returns "" if it cannot find the value in the dict
string grabJsonValue(string jsonString, string value)
{
    //scan to find a match
    unsigned int i, valueIndex;
    valueIndex = 0;
    for (i = 0; i < jsonString.length(); i++)
    {
        if (jsonString[i] == value[valueIndex])
        {
            valueIndex++;
        }
        else
        {
            valueIndex = 0;
        }
        if (valueIndex == value.length())
        {
            //found a match!
            break;
        }
    }

    if (valueIndex != value.length())
    {
        //could not find that value
        return "";
    }

    if ((jsonString[i + 1] != '\"') || (jsonString[i + 2] != ':'))
    {
        cerr << "Invalid JSON found in grabJsonValue" << endl;
        return "";
    }

    i += 3;

    int levelsDown = 0;
    string toReturn = "";

    while (true)
    {
        if (levelsDown == 0)
        {
            if ((jsonString[i] == '}') || (jsonString[i] == ','))
            {
                break;
            }
        }
        if ((jsonString[i] == '{') || (jsonString[i] == '['))
        {
            levelsDown++;
        }
        else if ((jsonString[i] == '}') || (jsonString[i] == ']'))
        {
            levelsDown--;
        }

        toReturn += jsonString[i];
        i++;
    }
    return toReturn;
}


