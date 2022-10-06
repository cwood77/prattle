#include "../prattle/lexor.hpp"
#include <sstream>
#include <stdexcept>

using namespace prattle::lex;

namespace {

// =====================================================================================

void testStrategies()
{
   const tokenTableEntry commonLexorSet[] = {
      { tokenTableEntry::kPunctuation,  "_#",      1, "" },
      { tokenTableEntry::kAlphanumeric, "_entity", 2, "" },
      { tokenTableEntry::kPunctuation,  NULL,      0, "" }
   };
   tokenTable cls(commonLexorSet);
   tokenTableStrategy ss_strat(cls);
   compositeScanStrategy strat;
   strat.append(ss_strat);

   // tokenTableStrategy
   {
      kernel k;
      k.pThumb = "_#_#_entity_entity";

      // punctuation scan (i.e. punc can be terminated by punc)
      strat.scan(k);
      if(std::string("_#_entity_entity") != k.pThumb)
         throw std::runtime_error("fail");
      if(1 != k.token)
         throw std::runtime_error("fail");

      k.unclassify();

      // punctuation scan (i.e. punc can be terminated by alphanum)
      strat.scan(k);
      if(std::string("_entity_entity") != k.pThumb)
         throw std::runtime_error("fail");
      if(1 != k.token)
         throw std::runtime_error("fail");

      k.unclassify();

      // alphanumeric scan (i.e. alphanum cannot be terminated by alphanum)
      strat.scan(k);
      if(std::string("_entity_entity") != k.pThumb)
         throw std::runtime_error("fail");
      if(0 != k.token)
         throw std::runtime_error("fail");
   }

   // tokenTableStrategy + whitespaceScanStrategy collaboration
   {
      kernel k;
      k.pThumb = "_entity _entity";
      compositeScanStrategy localStrat;
      localStrat.append(strat);

      // alphanumeric scan (i.e. alphanum can be terminated by whitespace)
      localStrat.scan(k);
      if(std::string(" _entity") != k.pThumb)
         throw std::runtime_error("fail");
      if(2 != k.token)
         throw std::runtime_error("fail");

      k.unclassify();

      // alphanumeric scan can terminate with whitespace but won't make progress
      localStrat.scan(k);
      if(std::string(" _entity") != k.pThumb)
         throw std::runtime_error("fail");
      if(0 != k.token)
         throw std::runtime_error("fail");

      whitespaceScanStrategy wss;
      localStrat.append(wss);
      k.unclassify();

      // whitespace scanner collaborates with alphanum
      localStrat.scan(k);
      if(std::string("") != k.pThumb)
         throw std::runtime_error("fail");
      if(2 != k.token)
         throw std::runtime_error("fail");
   }

   // multi collaboration
   {
      kernel k;
      k.pThumb = " \t _entity  \n  \r  word";
      compositeScanStrategy localStrat;
      localStrat.append(strat);
      whitespaceScanStrategy wss;
      newlineScanStrategy nlss;
      eoiScanStrategy eoiss;
      anyWordStrategy wdss(17);
      localStrat.append(wss);
      localStrat.append(nlss);
      localStrat.append(eoiss);
      localStrat.append(wdss);

      localStrat.scan(k);
      if(2 != k.token)
         throw std::runtime_error("fail");
      k.unclassify();
      localStrat.scan(k);
      if(17 != k.token)
         throw std::runtime_error("fail");
      if(k.lexeme != "word")
         throw std::runtime_error("fail");
      k.unclassify();
      localStrat.scan(k);
      if(lexorBase::kEOI != k.token)
         throw std::runtime_error("fail");
   }
}

// =====================================================================================

class testLexor : public lexorBase {
public:
   enum tokens {
      kLBrace = kFirstDerivedToken,
      kRBrace,
      kEntity,
      kActions,
      kComment,
      kWord,
   };

   testLexor(const iScanStrategy& defaultStrat, const char *pThumb)
   : lexorBase(defaultStrat,pThumb) {}

   tokens getToken() const { return (tokens)_getToken(); }
};

static const tokenTableEntry commonLs[] = {
   { tokenTableEntry::kPunctuation,  "_#", testLexor::kComment, "comment" },
   { tokenTableEntry::kPunctuation,  NULL },
};

static const tokenTableEntry topLevelLs[] = {
   { tokenTableEntry::kAlphanumeric, "_entity", testLexor::kEntity, "entity" },
   { tokenTableEntry::kPunctuation,  NULL },
};

static const tokenTableEntry entityLs[] = {
   { tokenTableEntry::kPunctuation,  "{",       testLexor::kLBrace,  "left brace"  },
   { tokenTableEntry::kPunctuation,  "}",       testLexor::kRBrace,  "right brace" },
   { tokenTableEntry::kAlphanumeric, "actions", testLexor::kActions, "actions"     },
   { tokenTableEntry::kPunctuation,  NULL },
};

void testLexor_()
{
   std::stringstream in;
   in
      << "_entity {"    << std::endl
      << "   actions {" << std::endl
      << "      word"   << std::endl
      << "   }"         << std::endl
      << "}"            << std::endl
   ;
   std::string copy = in.str();

   whitespaceScanStrategy wss;
   newlineScanStrategy nlss;
   eoiScanStrategy eoiss;
   anyWordStrategy wdss(testLexor::kWord);

   tokenTable _tlls(commonLs);
   _tlls.add(topLevelLs);
   tokenTableStrategy _tlsss(_tlls);
   compositeScanStrategy tlss;
   tlss.append(_tlsss);
   tlss.append(wss);
   tlss.append(nlss);
   tlss.append(eoiss);
   tlss.append(wdss);

   tokenTable _enls(commonLs);
   _enls.add(entityLs);
   tokenTableStrategy _ensss(_enls);
   compositeScanStrategy enss;
   enss.append(_ensss);
   enss.append(wss);
   enss.append(nlss);
   enss.append(eoiss);
   enss.append(wdss);

   testLexor l(tlss,copy.c_str());
   l.demandAndEat(testLexor::kEntity, enss);
   l.demandAndEat(testLexor::kLBrace, enss);
   l.demandAndEat(testLexor::kActions, enss);
   l.demandAndEat(testLexor::kLBrace, enss);
   l.demand(testLexor::kWord);
   if(l.getLexeme() != "word")
      throw std::runtime_error("fail");
   l.advance(enss);
   l.demandAndEat(testLexor::kRBrace, enss);
   l.demandAndEat(testLexor::kRBrace);
}

} // anonymous namespace

// =====================================================================================

void lexorTest()
{
   testStrategies();
   testLexor_();
}
