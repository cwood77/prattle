#include "pass.hpp"
#include <iostream>
#include <set>
#include <stdexcept>

namespace prattle {
namespace pass {
namespace {

class prioritySorter {
public:
   typedef const iPassInfo *elt_t;

   bool operator()(const elt_t& lhs, const elt_t& rhs) const
   {
      if(lhs->getPriority() == rhs->getPriority())
         return lhs < rhs;
      else
         return lhs->getPriority() < rhs->getPriority();
   }
};

} // anonymous namespace

passCatalog& passCatalog::get()
{
   static passCatalog pc;
   return pc;
}

void passCatalog::publish(const iPassInfo& p)
{
   m_catalog[p.getName()] = &p;
}

void passCatalog::publishTo(passCatalog& other)
{
   for(auto it=m_catalog.begin();it!=m_catalog.end();++it)
      other.publish(*it->second);
}

phasePassCatalog passCatalog::getPhase(const std::string& phase)
{
   return phasePassCatalog(phase,*this);
}

const iPassInfo& passCatalog::demand(const std::string& name)
{
   auto it = m_catalog.find(name);
   if(it == m_catalog.end())
      throw std::runtime_error("pass not found: " + name);
   return *it->second;
}

void phasePassCatalog::getByPriority(std::list<const iPassInfo*>& order)
{
   std::set<const iPassInfo*,prioritySorter> s;
   for(auto it=m_pCat->m_catalog.begin();it!=m_pCat->m_catalog.end();++it)
      if(it->second->getPhase() == m_phase)
         s.insert(it->second);

   order = std::list<const iPassInfo*>(s.begin(),s.end());
}

passRunChain::~passRunChain()
{
   for(auto *pP : passes)
      delete pP;
}

void passRunChain::run(config& c, void *pIr)
{
   for(auto *pP : passes)
   {
      std::cout << "running pass " << pP->getInfo().getName() << std::endl;
      pP->run(c,pIr);
   }
}

void passScheduler::schedule(phasePassCatalog& c, passSchedule& s)
{
   std::list<const iPassInfo*> order;
   c.getByPriority(order);
   for(auto *pPi : order)
      s.append(*pPi);
}

void passScheduler::inflate(const passSchedule& s, passRunChain& c)
{
   auto& l = s.get();
   for(auto *pPi : l)
      c.passes.push_back(pPi->create());
}

void passManager::run(config& c, passRunChain& rc, void *pIr)
{
   rc.run(c,pIr);
}

targetCatalog& targetCatalog::get()
{
   static targetCatalog the;
   return the;
}

void targetCatalog::publish(const iTargetInfo& t)
{
   m_cat[t.getName()] = &t;
}

void targetCatalog::publishTo(targetCatalog& other)
{
   for(auto it=m_cat.begin();it!=m_cat.end();++it)
      other.publish(*it->second);
}

iTarget *targetCatalog::create(const std::string& name)
{
   auto *pT = tryCreate(name);
   if(!pT)
      throw std::runtime_error("target not found: " + name);
   return pT;
}

iTarget *targetCatalog::tryCreate(const std::string& name)
{
   auto *pT = m_cat[name];
   if(!pT)
      return NULL;
   return pT->create();
}

targetChain::~targetChain()
{
   for(auto *pTgt : tgts)
      delete pTgt;
}

void targetChain::adjustPasses(passCatalog& c, passSchedule& s)
{
   for(auto *pTgt : tgts)
   {
      std::cout << "gathering passes from target " << pTgt->getInfo().getName() << std::endl;
      pTgt->adjustPasses(c,s);
   }
}

void targetChainBuilder::build(config& c, iTargetFactory& f, const std::string& finalTarget, targetChain& tc)
{
   auto *pTgt = f.create(finalTarget);
   while(true)
   {
      pTgt->configure(c);
      tc.tgts.push_front(pTgt);
      auto name = pTgt->getPredecessorTarget();
      if(name.empty())
         break;
      pTgt = f.create(name);
   }
}

} // namespace pass
} // namespace prattle
