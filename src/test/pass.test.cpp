#include "../prattle/config.hpp"
#include "../prattle/pass.hpp"
#include <stdexcept>

using namespace prattle;
using namespace prattle::pass;

namespace {

class dummyPass : public iPass {
public:
   virtual void run(config& c, void*& _pIr)
   {
      auto *pIr = (size_t*&)_pIr;
      *pIr += 2;
   }
};

autoPassInfo<dummyPass> gDummyPass("0",0);

} // anonymous namespace

void passTest()
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
   passManager().run(cfg,rc,(void*&)pIr);
   if(ir != 6)
      throw std::runtime_error("fail");
}
