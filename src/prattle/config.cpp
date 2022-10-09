#include "config.hpp"

namespace prattle {

void stringSetting::dump(std::ostream& s)
{
   s << value;
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

} // namespace prattle
