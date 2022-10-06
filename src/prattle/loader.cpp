#include "loader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace prattle {
namespace {

class fileSource : public iLexorInput {
public:
   fileSource(const std::string& fileName, const std::string& contents)
   : m_fileName(fileName), m_contents(contents) {}

   virtual std::string getFileName() const { return m_fileName; }
   virtual const char *getContents() const { return m_contents.c_str(); }

private:
   std::string m_fileName;
   std::string m_contents;
};

} // anonymous namespace

iLexorInput *fileLoader::load(const std::string& path)
{
   std::ifstream file(path.c_str());
   if(!file.good())
      throw std::runtime_error("can't open " + path);

   std::stringstream buffer;
   while(true)
   {
      std::string line;
      std::getline(file,line);
      if(!file.good())
      {
         if(!line.empty())
            buffer << line;
         break;
      }
      buffer << line << std::endl;
   }

   return new fileSource(path,buffer.str());
}

} // namespace prattle
