#pragma once
#include <functional>
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
   virtual iSetting *clone() = 0;
};

class stringSetting : public iSetting {
public:
   std::string value;
   virtual void dump(std::ostream& s);
   virtual iSetting *clone();
};

class stringArraySetting : public iSetting {
public:
   std::vector<std::string> value;
   virtual void dump(std::ostream& s);
   virtual iSetting *clone();
};

class boolSetting : public iSetting {
public:
   bool value;
   virtual void dump(std::ostream& s);
   virtual iSetting *clone();
};

class config {
public:
   config() {}
   ~config();

   template<class T> T *fetch(const std::string& name)
   {
      auto k = makeKey<T>(name);
      auto it = m_settings.find(k);
      if(it == m_settings.end())
         return NULL;
      return dynamic_cast<T*>(it->second);
   }

   template<class T> T& demand(const std::string& name)
   {
      T *pAns = fetch<T>(name);
      if(!pAns)
         throw std::runtime_error("cannot find " + name);
      return *pAns;
   }

   template<class T>
   T& createOrFetch(const std::string& name, std::function<T&>& setter = [](auto&){})
   {
      iSetting*& pVal = m_settings[makeKey<T>(name)];
      if(!pVal)
      {
         pVal = new T();
         setter(*pVal);
      }
      return *dynamic_cast<T*>(pVal);
   }

   void dump(std::ostream& s);

   void cloneInto(config& other);

private:
   template<class T>
   static std::string makeKey(const std::string& name)
   { return name + "/" + typeid(T).name(); }

   std::map<std::string,iSetting*> m_settings;

   config(const config&);
   config& operator=(const config&);
};

} // namespace prattle
