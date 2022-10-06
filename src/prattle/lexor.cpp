#include "lexor.hpp"
#include "loader.hpp"
#include <cstring>
#include <sstream>

namespace prattle {
namespace lex {

void kernel::unclassify()
{
   token = lexorBase::kUnknown;
   lexeme = "";
}

std::string kernel::toString() const
{
   std::stringstream stream;

   stream << "k:" << pThumb << "|" << token << "|" << std::endl;

   return stream.str();
}

void compositeScanStrategy::scan(kernel& k) const
{
   bool restart;
   do
   {
      restart = false;
      for(auto *pStrat : m_strats)
      {
         auto before = k.pThumb;

         pStrat->scan(k);
         if(k.token != lexorBase::kUnknown)
            break;

         if(before != k.pThumb)
         {
            restart = true;
            break;
         }
      }
   }
   while(restart);
}

std::string compositeScanStrategy::getTokenName(size_t t) const
{
   for(auto *pStrat : m_strats)
   {
      auto rVal = pStrat->getTokenName(t);
      if(!rVal.empty())
         return rVal;
   }

   throw std::runtime_error("unknown token");
}

void whitespaceScanStrategy::scan(kernel& k) const
{
   if(k.token != lexorBase::kUnknown)
      return;

   for(;*k.pThumb==' '||*k.pThumb=='\t';++k.pThumb);
}

void newlineScanStrategy::scan(kernel& k) const
{
   if(k.token != lexorBase::kUnknown)
      return;

   for(;*k.pThumb=='\r'||*k.pThumb=='\n';++k.pThumb);
}

void eoiScanStrategy::scan(kernel& k) const
{
   if(k.token != lexorBase::kUnknown)
      return;

   if(*k.pThumb == 0)
      k.token = lexorBase::kEOI;
}

void anyWordStrategy::scan(kernel& k) const
{
   if(k.token != lexorBase::kUnknown)
      return;

   const char *pStart = k.pThumb;
   for(;
      *k.pThumb!=0&&*k.pThumb!=' '&&*k.pThumb!='\t'&&*k.pThumb!='\r'&&*k.pThumb!='\n';
      ++k.pThumb);
   k.token = m_token;
   k.lexeme = std::string(pStart,k.pThumb-pStart);
}

void tokenTable::add(const tokenTableEntry *pTable)
{
   const tokenTableEntry *pThumb = pTable;
   while(pThumb->pLexeme)
   {
      auto *pMap = &alphanumerics;
      if(pThumb->term == tokenTableEntry::kPunctuation)
         pMap = &punctuations;

      (*pMap)[pThumb->pLexeme] = pThumb;
      tokenNames[pThumb->token] = pThumb->pTokenName;

      pThumb++;
   }
}

void tokenTableStrategy::scan(kernel& k) const
{
   if(k.token != lexorBase::kUnknown)
      return;

   // check for puncs with just strlen
   if(auto *pE = matchesPunc(k.pThumb))
   {
      updateKernel(*pE,k);
      return;
   }

   // check for alphas with strlen, provided terminators are punc, newlines, or whitespace
   for(auto it=m_s.alphanumerics.rbegin();it!=m_s.alphanumerics.rend();++it)
   {
      if(::strncmp(k.pThumb,it->first.c_str(),it->first.length())==0)
      {
         const char *pTerm = k.pThumb + it->first.length();
         if(
            *pTerm == 0 ||
            *pTerm == '\r' ||
            *pTerm == '\n' ||
            *pTerm == '\t' ||
            *pTerm == ' ' ||
            matchesPunc(pTerm))
         {
            updateKernel(*it->second,k);
            return;
         }
      }
   }
}

std::string tokenTableStrategy::getTokenName(size_t t) const
{
   auto it = m_s.tokenNames.find(t);
   if(it == m_s.tokenNames.end())
      throw std::runtime_error("ise - getTokenName");
   return it->second;
}

const tokenTableEntry *tokenTableStrategy::matchesPunc(const char *pThumb) const
{
   for(auto it=m_s.punctuations.rbegin();it!=m_s.punctuations.rend();++it)
      if(::strncmp(pThumb,it->first.c_str(),it->first.length())==0)
         return it->second;

   return NULL;
}

void tokenTableStrategy::updateKernel(const tokenTableEntry& e, kernel& k) const
{
   k.pThumb += ::strlen(e.pLexeme);
   k.token = e.token;
   k.lexeme = e.pLexeme;
}

standardStrategy::standardStrategy(const tokenTable& s)
: m_tss(s)
, m_aws(0)
{
   setup(/*useAnyWord*/false);
}

standardStrategy::standardStrategy(const tokenTable& s, size_t anyWordToken)
: m_tss(s)
, m_aws(anyWordToken)
{
   setup(/*useAnyWord*/true);
}

void standardStrategy::setup(bool useAnyWord)
{
   append(m_wss);
   append(m_nlss);
   append(m_eoiss);
   append(m_tss);
   if(useAnyWord)
      append(m_aws);
}

void lexorBase::advance(const iScanStrategy& s)
{
   m_k.unclassify();
   m_pLastStrat = &s;
   m_pLastStrat->scan(m_k);
}

void lexorBase::demand(size_t token)
{
   if(_getToken() != token)
   {
      std::stringstream msg;
      msg << "expected " << getTokenName(token);
      msg << ", but got " << getTokenName(m_k.token);
      error(msg.str());
   }
}

void lexorBase::error(const std::string& msg)
{
   throw std::runtime_error(msg.c_str());
}

lexorBase::lexorBase(const iScanStrategy& defaultStrat, iLexorInput& src)
: m_pDefStrat(&defaultStrat)
, m_pLastStrat(&defaultStrat)
, m_lin(src)
{
   m_k.pThumb = src.getContents();
   advance();
}

} // namespace lex
} // namespace prattle
