#include "../prattle/config.hpp"
#include "../prattle/module.hpp"
#include "../prattle/pass.hpp"
#include <stdexcept>

using namespace prattle;
using namespace prattle::pass;

namespace {

// =====================================================================================

class dummyPass : public iPass {
public:
   virtual void run(config& c, passLinks& l, void *_pIr)
   {
      auto *pIr = (size_t*)_pIr;
      *pIr += 2;

      auto *pPrev = l.findLink<dummyPass>();
      if(pPrev)
         *pIr += 1;
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
   virtual void adjustPasses(module::moduleLoader&, passCatalog& c, passSchedule& s) {}
};

cdwExportTarget(typicalStuff);

class dotTarget : public iTarget {
public:
   virtual void configure(config& c) {}
   virtual std::string getPredecessorTarget() { return "typicalStuff"; }
   virtual void adjustPasses(module::moduleLoader&, passCatalog& c, passSchedule& s)
   {
      s.append(c.demand("dummyPass"));
      s.append(c.demand("dummyPass"));
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

   module::moduleLoader mLdr;
   auto& pc = passCatalog::get();
   passSchedule sched;
   tc.adjustPasses(mLdr,pc,sched);
   if(sched.get().size() != 2)
      throw std::runtime_error("fail");

   passRunChain rc;
   passScheduler().inflate(sched,rc);

   size_t ir = 4;
   size_t *pIr = &ir;
   passManager().run(cfg,rc,pIr);
   if(ir != 9)
      throw std::runtime_error("fail");
}

} // anonymous namespace

// =====================================================================================

void passTest()
{
   passManagerTest();
   targetTest();
}
