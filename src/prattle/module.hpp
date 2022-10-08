#pragma once
#define WIN32_LEAN_AND_MEAN
#include "pass.hpp"
#include "windows.h"
#include <list>
#include <set>
#include <string>

namespace prattle {
namespace module {

class iModule {
public:
   virtual void solicit(std::list<std::string>& depMods) = 0;
   virtual void collect(pass::passCatalog& p, pass::targetCatalog& t) = 0;
};

class moduleLoader {
public:
   ~moduleLoader();
   void tryLoad(const std::string& name);
   void collect(pass::passCatalog& p, pass::targetCatalog& t);

private:
   std::set<std::string> m_loaded;
   std::list<HINSTANCE> m_libs;
   std::list<iModule*> m_mods;
};

class loadingTargetFactory : public pass::iTargetFactory {
public:
   loadingTargetFactory(pass::targetCatalog& tCat, moduleLoader& mLdr)
   : m_tCat(tCat), m_mLdr(mLdr) {}

   virtual pass::iTarget *create(const std::string& name);

private:
   pass::targetCatalog& m_tCat;
   moduleLoader& m_mLdr;
};

} // namespace module
} // namespace prattle

#define cdwImplModule(__mod__) \
__declspec(dllexport) prattle::module::iModule *createModule() { return &__mod__; }
