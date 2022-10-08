#include "../prattle/config.hpp"
#include "../prattle/pass.hpp"
#include <stdexcept>

using namespace prattle;
using namespace prattle::pass;

namespace {

// =====================================================================================

class dummyPass : public iPass {
public:
   virtual void run(config& c, void *_pIr)
   {
      auto *pIr = (size_t*)_pIr;
      *pIr += 2;
   }
};

cdwExportPass(dummyPass,"0",0);

void passManagerTest()
{
   auto& pc = passCatalog::get();
   phasePassCatalog ppc = pc.getPhase("0");

   passSchedule sched;
   passScheduler().schedule(ppc,sched);

   passRunChain rc;
   passScheduler().inflate(sched,rc);

   config cfg;
   size_t ir = 4;
   size_t *pIr = &ir;
   passManager().run(cfg,rc,pIr);
   if(ir != 6)
      throw std::runtime_error("fail");
}

// =====================================================================================

class typicalStuff : public iTarget {
public:
   virtual void configure(config& c) {}
   virtual std::string getPredecessorTarget() { return ""; }
   virtual void adjustPasses(passCatalog& c, passSchedule& s) {}
};

cdwExportTarget(typicalStuff);

class dotTarget : public iTarget {
public:
   virtual void configure(config& c) {}
   virtual std::string getPredecessorTarget() { return "typicalStuff"; }
   virtual void adjustPasses(passCatalog& c, passSchedule& s)
   {
      s.append(c.demand<dummyPass>());
      s.append(c.demand(typeid(dummyPass).name()));
   }
};

autoTargetInfo<dotTarget> gDotTarget("dot");

void targetTest()
{
   config cfg;
   auto& tf = targetCatalog::get();
   targetChain tc;
   targetChainBuilder().build(cfg,tf,"dot",tc);
   if(tc.tgts.size() != 2)
      throw std::runtime_error("fail");
}

} // anonymous namespace

// =====================================================================================

void passTest()
{
   passManagerTest();
   targetTest();
}
