#pragma once
#include <string>

namespace prattle {

class iLexorInput {
public:
   virtual ~iLexorInput() {}
   virtual std::string getFileName() const = 0;
   virtual const char *getContents() const = 0;
};

class memorySource : public iLexorInput {
public:
   explicit memorySource(const char *pThumb) : m_pThumb(pThumb) {}
   virtual std::string getFileName() const { return ""; }
   virtual const char *getContents() const { return m_pThumb; }

private:
   const char *m_pThumb;
};

class fileLoader {
public:
   static iLexorInput *load(const std::string& path);

private:
   fileLoader();
   fileLoader(const fileLoader&);
   fileLoader& operator=(const fileLoader&);
};

} // namespace prattle
