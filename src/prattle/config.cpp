#include "config.hpp"

namespace prattle {

config::~config()
{
   for(auto it=m_settings.begin();it!=m_settings.end();++it)
      delete it->second;
}

} // namespace prattle
