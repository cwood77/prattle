#pragma once
#include <string>
#include <map>
#include <typeinfo>

namespace prattle {

class iSetting {
public:
   virtual ~iSetting() {}
};

class stringSetting : public iSetting {
public:
   std::string value;
};

class config {
public:
   config() {}
   ~config();

   template<class T> T *fetch(const std::string& name = "");
   template<class T> T& demand(const std::string& name = "");

   template<class T>
   T& createOrFetch(const std::string& name = "")
   {
      iSetting*& pVal = m_settings[makeKey<T>(name)];
      if(!pVal)
         pVal = new T();
      return *dynamic_cast<T*>(pVal);
   }

private:
   template<class T>
   static std::string makeKey(const std::string& name)
   { return name + "/" + typeid(T).name(); }

   std::map<std::string,iSetting*> m_settings;

   config(const config&);
   config& operator=(const config&);
};

} // namespace prattle
