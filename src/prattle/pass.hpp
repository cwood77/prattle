#pragma once
#include <list>
#include <map>
#include <string>
#include <typeinfo>

namespace prattle {

class config;

namespace pass {

class iPassInfo;

class iPass {
public:
   virtual ~iPass() {}
   const iPassInfo& getInfo() const { return *m_pInfo; }
   virtual void run(config& c, void *pIr) = 0;

   const iPassInfo *m_pInfo;
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
   passInfo(const std::string& phase, unsigned long priority, const std::string& name)
   : m_phase(phase), m_priority(priority), m_name(name) {}

   virtual const std::string& getPhase() const { return m_phase; }
   virtual unsigned long getPriority() const { return m_priority; }
   virtual std::string getName() const { return m_name; }
   virtual iPass *create() const { T *p =  new T(); p->m_pInfo = this; return p; }

private:
   std::string m_phase;
   unsigned long m_priority;
   std::string m_name;
};

class phasePassCatalog;

class passCatalog {
public:
   static passCatalog& get();

   void publish(const iPassInfo& p);
   void publishTo(passCatalog& other);
   phasePassCatalog getPhase(const std::string& phase);

   template<class T> const iPassInfo& demand() { return demand(typeid(T).name()); }
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
   autoPassInfo(const std::string& phase, unsigned long priority, const std::string& name)
   : passInfo<T>(phase,priority,name)
   { passCatalog::get().publish(*this); }
};

#define cdwExportPass(__name__,__phase__,__priority__) \
   static autoPassInfo<__name__> g##__name__(__phase__,__priority__,#__name__);

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

   void run(config& c, void *pIr);

   std::list<iPass*> passes;
};

class passScheduler {
public:
   void schedule(phasePassCatalog& c, passSchedule& s);
   void inflate(const passSchedule& s, passRunChain& c);
};

class passManager {
public:
   void run(config& c, passRunChain& rc, void *pIr);
   void run(config& c, passRunChain& rc)
   { void *pUnused = NULL; run(c,rc,pUnused); }
};

class iTargetInfo;

class iTarget {
public:
   virtual ~iTarget() {}
   const iTargetInfo& getInfo() const { return *m_pInfo; }
   virtual void configure(config& c) = 0;
   virtual std::string getPredecessorTarget() = 0;
   virtual void adjustPasses(passCatalog& c, passSchedule& s) = 0;

   const iTargetInfo *m_pInfo;
};

class iTargetInfo {
public:
   virtual std::string getName() const = 0;
   virtual iTarget *create() const = 0;
};

template<class T>
class targetInfo : public iTargetInfo {
public:
   explicit targetInfo(const std::string& name)
   : m_name(name) {}

   virtual std::string getName() const { return m_name; }
   virtual iTarget *create() const { T *p = new T(); p->m_pInfo = this; return p; }

private:
   std::string m_name;
};

class iTargetFactory {
public:
   virtual iTarget *create(const std::string& name) = 0;
};

class targetCatalog : public iTargetFactory {
public:
   static targetCatalog& get();

   void publish(const iTargetInfo& t);
   void publishTo(targetCatalog& other);
   virtual iTarget *create(const std::string& name);
   iTarget *tryCreate(const std::string& name);

private:
   std::map<std::string,const iTargetInfo*> m_cat;
};

template<class T>
class autoTargetInfo : public targetInfo<T> {
public:
   explicit autoTargetInfo(const std::string& name)
   : targetInfo<T>(name)
   { targetCatalog::get().publish(*this); }
};

#define cdwExportTarget(__name__) \
   static autoTargetInfo<__name__> g##__name__(#__name__);

class targetChain {
public:
   ~targetChain();

   void adjustPasses(passCatalog& c, passSchedule& s);

   std::list<iTarget*> tgts;
};

class targetChainBuilder {
public:
   void build(config& c, iTargetFactory& f, const std::string& finalTarget, targetChain& tc);
};

} // namespace pass
} // namespace prattle
