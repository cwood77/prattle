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
   bool tryLoad(const std::string& name);
   void collect(pass::passCatalog& p, pass::targetCatalog& t);

private:
   std::set<std::string> m_loaded;
   std::list<HINSTANCE> m_libs;
   std::list<iModule*> m_mods;
};

class incrementalModuleLoader {
public:
   incrementalModuleLoader(pass::passCatalog& p, pass::targetCatalog& tCat, moduleLoader& mLdr)
   : m_pCat(p), m_tCat(tCat), m_mLdr(mLdr) {}

   void tryLoad(const std::string& name);

private:
   pass::passCatalog& m_pCat;
   pass::targetCatalog& m_tCat;
   moduleLoader& m_mLdr;
};

class loadingTargetFactory : public pass::iTargetFactory {
public:
   explicit loadingTargetFactory(pass::targetCatalog& tCat, incrementalModuleLoader& iml)
   : m_tCat(tCat), m_imLdr(iml) {}

   virtual pass::iTarget *create(const std::string& name);

private:
   pass::targetCatalog& m_tCat;
   incrementalModuleLoader& m_imLdr;
};

} // namespace module
} // namespace prattle

#define cdwImplModule(__mod__) \
__declspec(dllexport) prattle::module::iModule *createModule() { return &__mod__; }
