#include <vector>
#include <string>

void parseJsonList(std::string *jsonString, std::vector<std::string> *toFill);
std::string grabJsonObject(std::string *jsonString);
std::string grabJsonValue(std::string jsonString, std::string value);
