#pragma once
#include <list>
#include <map>
#include <string>
#include <vector>

// the lexor is composed of iScanStrategy instances, which can do free-form scanning or
// table-based scanning

namespace prattle {

class iLexorInput;

namespace lex {

class kernel {
public:
   kernel() : pThumb(NULL), lineNumber(1) { unclassify(); }

   void unclassify();

   const char *pThumb;
   unsigned long lineNumber;
   size_t token;
   std::string lexeme;
};

class iScanStrategy {
public:
   virtual void scan(kernel& k) const = 0;
};

class compositeScanStrategy : public iScanStrategy {
public:
   void append(const iScanStrategy& next) { m_strats.push_back(&next); }

   virtual void scan(kernel& k) const;

private:
   std::list<const iScanStrategy*> m_strats;
};

class whitespaceScanStrategy : public iScanStrategy {
public:
   virtual void scan(kernel& k) const;
};

class newlineScanStrategy : public iScanStrategy {
public:
   virtual void scan(kernel& k) const;
};

class eoiScanStrategy : public iScanStrategy {
public:
   virtual void scan(kernel& k) const;
};

// classify any lexeme as this token
class anyWordStrategy : public iScanStrategy {
public:
   explicit anyWordStrategy(size_t token) : m_token(token) {}
   virtual void scan(kernel& k) const;

private:
   const size_t m_token;
};

struct lexemeTableEntry {
public:
   enum termination {
      kAlphanumeric,
      kPunctuation,
   };

   termination term;
   const char *pLexeme;
   size_t token;
};

class lexemeTable {
public:
   explicit lexemeTable(const lexemeTableEntry *pTable) { add(pTable); }
   void add(const lexemeTableEntry *pTable);

   std::map<std::string,const lexemeTableEntry*> alphanumerics;
   std::map<std::string,const lexemeTableEntry*> punctuations;
};

class lexemeTableStrategy : public iScanStrategy {
public:
   explicit lexemeTableStrategy(const lexemeTable& s) : m_s(s) {}

   virtual void scan(kernel& k) const;

private:
   const lexemeTableEntry *matchesPunc(const char *pThumb) const;
   void updateKernel(const lexemeTableEntry& e, kernel& k) const;

   const lexemeTable& m_s;
};

class standardStrategy : public compositeScanStrategy {
public:
   explicit standardStrategy(const lexemeTable& s);
   standardStrategy(const lexemeTable& s, size_t anyWordToken);

private:
   void setup(bool useAnyWord);

   whitespaceScanStrategy m_wss;
   newlineScanStrategy m_nlss;
   eoiScanStrategy m_eoiss;
   lexemeTableStrategy m_tss;
   anyWordStrategy m_aws;
};

class lexorBase {
public:
   enum {
      kUnknown = 0,
      kEOI     = 1,
      kFirstDerivedToken
   };

   std::string getTokenName(size_t t);

   std::string getFileName() const;
   unsigned long getLineNumber() const { return m_k.lineNumber; }
   size_t getToken() const { return m_k.token; }
   std::string getLexeme() const { return m_k.lexeme; }

   void scanToEndOfLine();
   void advance(const iScanStrategy& s);
   void advance() { advance(*m_pDefStrat); }

   void demand(size_t token);
   void demandAndEat(size_t token, const iScanStrategy& s) { demand(token); advance(s); }
   void demandAndEat(size_t token) { demandAndEat(token,*m_pDefStrat); }

   void expected(const std::vector<std::string>& tokens);
   void error(const std::string& msg);

protected:
   lexorBase(const iScanStrategy& defaultStrat, iLexorInput& src);

   void publishToken(size_t t, const char *name);

private:
   kernel m_k;
   const iScanStrategy *m_pDefStrat;
   iLexorInput& m_lin;
   std::map<size_t,std::string> m_tokenNames;
};

} // namespace lex
} // namespace prattle
