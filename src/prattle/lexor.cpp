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

   for(;*k.pThumb=='\r'||*k.pThumb=='\n';++k.pThumb)
      if(*k.pThumb=='\n')
         k.lineNumber++;
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

void lexemeTable::add(const lexemeTableEntry *pTable)
{
   const lexemeTableEntry *pThumb = pTable;
   while(pThumb->pLexeme)
   {
      auto *pMap = &alphanumerics;
      if(pThumb->term == lexemeTableEntry::kPunctuation)
         pMap = &punctuations;

      (*pMap)[pThumb->pLexeme] = pThumb;

      pThumb++;
   }
}

void lexemeTableStrategy::scan(kernel& k) const
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

const lexemeTableEntry *lexemeTableStrategy::matchesPunc(const char *pThumb) const
{
   for(auto it=m_s.punctuations.rbegin();it!=m_s.punctuations.rend();++it)
      if(::strncmp(pThumb,it->first.c_str(),it->first.length())==0)
         return it->second;

   return NULL;
}

void lexemeTableStrategy::updateKernel(const lexemeTableEntry& e, kernel& k) const
{
   k.pThumb += ::strlen(e.pLexeme);
   k.token = e.token;
   k.lexeme = e.pLexeme;
}

standardStrategy::standardStrategy(const lexemeTable& s)
: m_tss(s)
, m_aws(0)
{
   setup(/*useAnyWord*/false);
}

standardStrategy::standardStrategy(const lexemeTable& s, size_t anyWordToken)
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

std::string lexorBase::getTokenName(size_t t)
{
   auto it = m_tokenNames.find(t);
   if(it == m_tokenNames.end())
      throw std::runtime_error("token name unknown");
   return it->second;
}

std::string lexorBase::getFileName() const
{
   return m_lin.getFileName();
}

void lexorBase::scanToEndOfLine()
{
   for(;*m_k.pThumb!=0&&*m_k.pThumb!='\r'&&*m_k.pThumb!='\n';++m_k.pThumb);
}

void lexorBase::advance(const iScanStrategy& s)
{
   m_k.unclassify();
   s.scan(m_k);
}

void lexorBase::demand(size_t token)
{
   if(getToken() != token)
      expected({token});
}

void lexorBase::expected(const std::vector<size_t>& tokens)
{
   std::stringstream msg;
   msg << "expected ";
   bool first = true;
   for(auto& s : tokens)
   {
      if(!first)
         msg << ", ";
      msg << s;
      first = false;
   }

   error(msg.str());
}

void lexorBase::error(const std::string& msg)
{
   std::stringstream stream;

   stream << msg << std::endl;
   stream << "  in " << getFileName() << std::endl;
   stream << "  at " << getLineNumber() << std::endl;
   stream << "  current token is " << getTokenName(getToken()) << std::endl;
   stream << "  current lexeme is " << getLexeme() << std::endl;

   bool truncated = (::strlen(m_k.pThumb) > 10);
   std::string hint(m_k.pThumb,truncated ? 10 : ::strlen(m_k.pThumb));
   if(truncated)
      hint += "...";
   stream << "  current text is " << hint << std::endl;

   throw std::runtime_error(stream.str());
}

lexorBase::lexorBase(const iScanStrategy& defaultStrat, iLexorInput& src)
: m_pDefStrat(&defaultStrat)
, m_lin(src)
{
   publishToken(kUnknown,"<unknown>"     );
   publishToken(kEOI,    "<end of input>");

   m_k.pThumb = src.getContents();
   advance();
}

void lexorBase::publishToken(size_t t, const char *name)
{
   m_tokenNames[t] = name;
}

} // namespace lex
} // namespace prattle
