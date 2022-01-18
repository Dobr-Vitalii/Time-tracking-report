#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <mutex>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>


//method for selecting a string into substrings through a delimiter...
std::vector<std::string> splitDelim(const std::string& str, const std::string& delim) {
   std::vector<std::string> tokens;
   size_t prev = 0, pos = 0;

   do {
      pos = str.find(delim, prev);

      if (pos == std::string::npos)
         pos = str.length();

      const std::string& token = str.substr(prev, pos - prev);
      if (!token.empty())
         tokens.push_back(token);

      prev = pos + delim.length();
   } while (pos < str.length() && prev < str.length());
   return tokens;
}
//--------------------------------------------------------------------------------------------------------------
// go by latin characters
bool isSymb(char symb) {
   return (symb >= (char)'a' && symb <= (char)'z' || symb >= (char)'A' && symb <= (char)'Z');
}
//--------------------------------------------------------------------------------------------------------------
//read that this is a string
bool isStr(const std::string& str_name) {
   for (size_t index = 0; index < str_name.length(); index++) {
      if (!isSymb(str_name[index]))
         return false;
   }
   return true;
}
//--------------------------------------------------------------------------------------------------------------
bool isFullName(const std::string& full_name) {
   if (!std::regex_match(full_name, std::regex("([A-Z][a-z]+[\\s]?){2}"))) {
      throw std::invalid_argument("invalid input name");
   }
   return std::regex_match(full_name, std::regex("([A-Z][a-z]+[\\s]?){2}"));
}
//--------------------------------------------------------------------------------------------------------------
bool isDate(const std::string& date) {
   return std::regex_match(date, std::regex("(\d{4}-\d{2}-\d{2})"));
}
//--------------------------------------------------------------------------------------------------------------
bool isTask(const std::string& date) {
   return std::regex_match(date, std::regex("([A-Za-z]{3,}+)-\d{4})"));
}
//--------------------------------------------------------------------------------------------------------------
bool isMail(const std::string& mail) {
   /*auto at = std::find(mail.begin(), mail.end(), '@');
   auto dot = std::find(at, mail.end(), '.');
   return (at != mail.end()) && (dot != mail.end());*/
   return std::regex_match(mail, std::regex("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"));
}
//--------------------------------------------------------------------------------------------------------------
bool isNumber(const std::string& mail) {
   for (const char& c : mail) {
      if (!std::isdigit(c))
         return false;
   }
   return true;
}
//--------------------------------------------------------------------------------------------------------------
std::atomic<int> countTime{ 0 };

void addTimeToSum(int time) {
   countTime += time;
}
//--------------------------------------------------------------------------------------------------------------
int getTimeToSum() {
   return countTime;
}
//--------------------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<unsigned int>> mapTime;
std::mutex save;

int get_mon(const std::string& str_date) {
   std::tm tm = {};
   ::strptime(str_date.c_str(), "%Y-%m-%d", &tm);
   return tm.tm_mon();
}
//--------------------------------------------------------------------------------------------------------------
void setTimeForNameOnMap(const std::string& strName, const int month, const int timeSum) {
   std::vector<unsigned int> monthTime(12);
   std::lock_guard<std::mutex> lk(save);

   if (mapTime.empty()) {
      monthTime.at(month) = timeSum;
      mapTime.insert(std::make_pair(strName, monthTime));
   }
   else {
      auto mapItr = mapTime.find(strName);

      if (mapItr == mapTime.end()) {
         monthTime.at(month) = timeSum;
         mapTime.insert(std::make_pair(strName, monthTime));
      }
      else {
         mapTime->second.at(month) += timeSum;
      }
   }
}
//--------------------------------------------------------------------------------------------------------------
void writeToFile(std::string name, std::string month, std::string hours) {
   std::ofstream file;
   file.open("TotalTime.csv");
   file << name << ";" << month << ";" << hours << ";" << std::endl;

}
//--------------------------------------------------------------------------------------------------------------
void asyncMainParser(std::string inStrValue) {
   /* pattern:
      0	1	      2				3	    4	     5	 6	   7
   Name;email;department;position;project;task;date;logged hours
   count of elements = 8 (-1)
   */
   const size_t countElements = 7;
   auto vectorValue = splitDelim(inStrValue, ";");

   if (vectorValue.size() >= countElements) {
      std::string strName = "", strDate = "";

      if (isFullName(vectorValue.at(0))) {
         //Name
         strName = vectorValue.at(0);
      }
      else {
         std::cout << "Error: Full Name";
      }

      /*
      if (is_Mail(vector_value.at(1))) {
         //email
      }
      else {
         std::cout << "Error: Mail";
      }
      if (is_Str(vector_value.at(2))) {
         //Department
      }
      else {
         std::cout << "Error: Department";
      }
      if (is_Str(vector_value.at(3))) {
         //position
      }
      else {
         std::cout << "Error: position";
      }
      if (is_Str(vector_value.at(4))) {
         //project
      }
      else {
         std::cout << "Error: project";
      }
      if (is_Str(vector_value.at(5))) {
         //Task
      }
      else {
         std::cout << "Error: Task";
      }
      */

      if (isDate(vectorValue.at(6))) {
         //Date
         strDate = vectorValue.at(6);//add
      }
      else {
         std::cout << "Error: Date";
      }

      if (isNumber(vectorValue.at(7))) {
         //logged hours
         if (!strName.empty() && !strDate.empty()) {
            const int hours = std::stoi(vectorValue.at(7));
            const int mon = get_mon(strDate);
            setTimeForNameOnMap(strName, month, hours);
         }
      }
      else {
         std::cout << "Error: logged hours";
      }

      //write data from
      std::lock_guard<std::mutex> lk(save);
      auto mapTimeItr = this->map_time.begin();
      while (mapTimeItr != *this->map_time.end()) {
         std::string const& name = mapTimeItr->first; // Name
         for (int indexMonth = 0; indexMonth < 12; indexMonth++) {
            const int hours = mapTimeItr->second.at(month);
            writeToFile(name, month, hours);
         }
         ++mapTimeItr;
      }
   }
   else {
      std::cout << "Error: incomplete number of elements";
   }
}
//--------------------------------------------------------------------------------------------------------------
std::string readFile(const std::string& fileName) {
   std::ifstream f(fileName);
   std::stringstream ss;
   ss << f.rdbuf();
   return ss.str();
}
//--------------------------------------------------------------------------------------------------------------
int main()
{

   std::string fileName = "file.csv";
   std::string inStrValue = readFile(fileName);
   asyncMainParser(inStrValue);

}
