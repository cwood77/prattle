#pragma once
#include <list>
#include <map>
#include <string>

namespace prattle {
namespace lex {

class kernel {
public:
   kernel() : pThumb(NULL), lineNumber(1) { unclassify(); }

   void unclassify();
   std::string toString() const;

   const char *pThumb;
   unsigned long lineNumber;
   size_t token;
   std::string lexeme;
};

class iScanStrategy {
public:
   virtual void scan(kernel& k) const = 0;
   virtual std::string getTokenName(size_t t) const = 0;
};

class compositeScanStrategy : public iScanStrategy {
public:
   void append(const iScanStrategy& next) { m_strats.push_back(&next); }
   virtual void scan(kernel& k) const;
   virtual std::string getTokenName(size_t t) const;

private:
   std::list<const iScanStrategy*> m_strats;
};

class whitespaceScanStrategy : public iScanStrategy {
public:
   virtual void scan(kernel& k) const;
   virtual std::string getTokenName(size_t t) const { return ""; }
};

class newlineScanStrategy : public iScanStrategy {
public:
   virtual void scan(kernel& k) const;
   virtual std::string getTokenName(size_t t) const { return ""; }
};

class eoiScanStrategy : public iScanStrategy {
public:
   virtual void scan(kernel& k) const;
   virtual std::string getTokenName(size_t t) const { return ""; }
};

class wordScanStrategy : public iScanStrategy {
public:
   explicit wordScanStrategy(size_t token) : m_token(token) {}
   virtual void scan(kernel& k) const;
   virtual std::string getTokenName(size_t t) const { return ""; }

private:
   const size_t m_token;
};

struct lexorSetEntry {
public:
   enum termination {
      kAlphanumeric,
      kPunctuation,
   };

   termination term;
   const char *pLexeme;
   size_t token;
   const char *pTokenName;
   size_t flags;
};

class lexorSet {
public:
   explicit lexorSet(const lexorSetEntry *pTable) { add(pTable); }
   void add(const lexorSetEntry *pTable);

   std::map<std::string,const lexorSetEntry*> alphanumerics;
   std::map<std::string,const lexorSetEntry*> punctuations;
   std::map<size_t,std::string> tokenNames;
};

class scanSetStrategy : public iScanStrategy {
public:
   explicit scanSetStrategy(const lexorSet& s) : m_s(s) {}

   virtual void scan(kernel& k) const;
   virtual std::string getTokenName(size_t t) const;

private:
   const lexorSetEntry *matchesPunc(const char *pThumb) const;
   void updateKernel(const lexorSetEntry& e, kernel& k) const;

   const lexorSet& m_s;
};

class lexorBase {
public:
   enum {
      kUnknown = 0,
      kEOI     = 1,
      kFirstDerivedToken
   };

   std::string getTokenName(size_t t) { return m_pLastStrat->getTokenName(t); }

   unsigned long getLineNumber() const { return m_k.lineNumber; }
   std::string getLexeme() const { return m_k.lexeme; }

   void advance(const iScanStrategy& s);
   void advance() { advance(*m_pDefStrat); }

   void demand(size_t token);
   void demandAndEat(size_t token, const iScanStrategy& s)
   { demand(token); advance(s); }
   void demandAndEat(size_t token) { demandAndEat(token,*m_pDefStrat); }

   void error(const std::string& msg);

protected:
   lexorBase(const iScanStrategy& defaultStrat, const char *pThumb);
   size_t _getToken() const { return m_k.token; }

private:
   kernel m_k;
   const iScanStrategy *m_pDefStrat;
   const iScanStrategy *m_pLastStrat;
};

} // namespace lex
} // namespace prattle
