#pragma once
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace prattle {

class iSetting {
public:
   virtual ~iSetting() {}
   virtual void dump(std::ostream& s) = 0;
};

class stringSetting : public iSetting {
public:
   std::string value;
   virtual void dump(std::ostream& s);
};

class config {
public:
   config() {}
   ~config();

   //template<class T> T *fetch(const std::string& name = "");

   template<class T> T& demand(const std::string& name = "")
   {
      auto k = makeKey<T>(name);
      auto it = m_settings.find(k);
      if(it == m_settings.end())
         throw std::runtime_error("cannot find " + k);
      return *dynamic_cast<T*>(it->second);
   }

   template<class T>
   T& createOrFetch(const std::string& name = "")
   {
      iSetting*& pVal = m_settings[makeKey<T>(name)];
      if(!pVal)
         pVal = new T();
      return *dynamic_cast<T*>(pVal);
   }

   void dump(std::ostream& s);

private:
   template<class T>
   static std::string makeKey(const std::string& name)
   { return name + "/" + typeid(T).name(); }

   std::map<std::string,iSetting*> m_settings;

   config(const config&);
   config& operator=(const config&);
};

} // namespace prattle
