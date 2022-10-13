#include "config.hpp"

namespace prattle {

void stringSetting::dump(std::ostream& s)
{
   s << value;
}

iSetting *stringSetting::clone()
{
   auto *pCpy = new stringSetting();
   pCpy->value = value;
   return pCpy;
}

void stringArraySetting::dump(std::ostream& s)
{
   s << "[";
   for(auto v : value)
      s << " " << v;
   s << " ]";
}

iSetting *stringArraySetting::clone()
{
   auto *pCpy = new stringArraySetting();
   pCpy->value = value;
   return pCpy;
}

void boolSetting::dump(std::ostream& s)
{
   s << (value ? "T" : "F");
}

iSetting *boolSetting::clone()
{
   auto *pCpy = new boolSetting();
   pCpy->value = value;
   return pCpy;
}

config::~config()
{
   for(auto it=m_settings.begin();it!=m_settings.end();++it)
      delete it->second;
}

void config::dump(std::ostream& s)
{
   s << "{" << std::endl;

   for(auto it=m_settings.begin();it!=m_settings.end();++it)
   {
      s << "  " << it->first << " = ";
      it->second->dump(s);
      s << std::endl;
   }

   s << "}" << std::endl;
}

void config::cloneInto(config& other)
{
   for(auto it=m_settings.begin();it!=m_settings.end();++it)
      other.m_settings[it->first] = it->second->clone();
}

} // namespace prattle
