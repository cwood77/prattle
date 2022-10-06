#pragma once
#include <list>
#include <map>
#include <string>
#include <typeinfo>

namespace prattle {

class config;

namespace pass {

class iPass {
public:
   virtual ~iPass() {}
   virtual void run(config& c, void*& pIr) = 0;
};

class iPassInfo {
public:
   virtual const std::string& getPhase() const = 0;
   virtual unsigned long getPriority() const = 0;
   virtual std::string getName() const = 0;
   virtual iPass *create() const = 0;
};

template<class T>
class passInfo : public iPassInfo {
public:
   passInfo(const std::string& phase, unsigned long priority)
   : m_phase(phase), m_priority(priority) {}

   virtual const std::string& getPhase() const { return m_phase; }
   virtual unsigned long getPriority() const { return m_priority; }
   virtual std::string getName() const { return typeid(T).name(); }
   virtual iPass *create() const { return new T(); }

private:
   std::string m_phase;
   unsigned long m_priority;
};

class phasePassCatalog;

class passCatalog {
public:
   static passCatalog& get();

   void publish(const iPassInfo& p);
   phasePassCatalog getPhase(const std::string& phase);
   const iPassInfo& demand(const std::string& name);

private:
   std::map<std::string,const iPassInfo*> m_catalog;

friend class phasePassCatalog;
};

class phasePassCatalog {
public:
   phasePassCatalog(const std::string& phase, passCatalog& cat)
   : m_phase(phase), m_pCat(&cat) {}

   void getByPriority(std::list<const iPassInfo*>& order);

private:
   std::string m_phase;
   passCatalog *m_pCat;
};

template<class T>
class autoPassInfo : public passInfo<T> {
public:
   autoPassInfo(const std::string& phase, unsigned long priority)
   : passInfo<T>(phase,priority)
   { passCatalog::get().publish(*this); }
};

class passSchedule {
public:
   void append(const iPassInfo& i) { m_sched.push_back(&i); }

   const std::list<const iPassInfo*>& get() const { return m_sched; }

private:
   std::list<const iPassInfo*> m_sched;
};

class passRunChain {
public:
   ~passRunChain();

   void run(config& c, void*& pIr);

   std::list<iPass*> passes;
};

class passScheduler {
public:
   void schedule(phasePassCatalog& c, passSchedule& s);
   void inflate(const passSchedule& s, passRunChain& c);
};

class passManager {
public:
   void run(config& c, passRunChain& rc, void*& pIr);
};

#if 0
class iTarget {
public:
   virtual void configure(config& c) = 0;
   virtual void adjustPasses(passCatalog& c, passSchedule& s) = 0;
};

class targetFactory {
public:
   iTarget *create(const std::string& name);
};

// simple "steps" outside the passManager??
// - default target selection pass
// - file loading pass
// passes:
// - table generation pass
// - obfuscation pass
//   ....
#endif

} // namespace pass
} // namespace prattle
