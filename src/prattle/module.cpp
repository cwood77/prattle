#include "module.hpp"

namespace prattle {
namespace module {

moduleLoader::~moduleLoader()
{
   for(auto h : m_libs)
      ::FreeLibrary(h);
}

void moduleLoader::tryLoad(const std::string& name)
{
   if(m_loaded.find(name)!=m_loaded.end())
      return; // already loaded
   m_loaded.insert(name);

   HINSTANCE h = ::LoadLibraryA(name.c_str());
   if(!h)
      return; // not a DLL?
   m_libs.push_back(h);

   typedef iModule *(*createFunc_t)();
   auto pFunc = (createFunc_t)::GetProcAddress(h,"asdasdad");
   if(!pFunc)
      return; // not a module

   iModule *pMod = pFunc();
   m_mods.push_back(pMod);

   std::list<std::string> depMods;
   pMod->solicit(depMods);
   for(auto n : depMods)
      tryLoad(n);
}

void moduleLoader::collect(pass::passCatalog& p, pass::targetCatalog& t)
{
   for(auto *pM : m_mods)
      pM->collect(p,t);
}

} // namespace module
} // namespace prattle
