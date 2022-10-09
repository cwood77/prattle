#pragma once
#include <sstream>

namespace prattle {
namespace log {

class iLog;

class tmpStrmLog {
public:
   explicit tmpStrmLog(iLog& l) : m_l(l) {}
   ~tmpStrmLog();
   std::ostream& s() { return m_s; }

private:
   iLog& m_l;
   std::stringstream m_s;
};

class iLog {
public:
   tmpStrmLog s() { return tmpStrmLog(*this); }
   virtual void write(const std::string& msg) = 0;

   virtual std::string genIndent() const = 0;
   virtual void adjustIndent(long stops) = 0;
};

inline tmpStrmLog::~tmpStrmLog()
{ m_l.write(m_s.str()); }

class logBase : public iLog {
public:
   size_t indentStopWidth;

   virtual std::string genIndent() const
   { return std::string(m_indentStop*indentStopWidth,' '); }
   virtual void adjustIndent(long stops) { m_indentStop += stops; }

protected:
   logBase() : indentStopWidth(3), m_indentStop(0) {}

private:
   size_t m_indentStop;
};

class streamLogAdapter : public logBase {
public:
   explicit streamLogAdapter(std::ostream& o) : m_o(o) {}
   virtual void write(const std::string& msg) { m_o << msg; }

private:
   std::ostream& m_o;
};

class indent {
public:
   explicit indent(const iLog& l) : l(l) {}
   const iLog& l;
};

inline std::ostream& operator<<(std::ostream& s, const indent& i)
{ s << i.l.genIndent(); return s; }

class autoIndent {
public:
   autoIndent(iLog& l, size_t nStops = 1) : m_l(l), m_nStops(nStops)
   { m_l.adjustIndent(m_nStops); }
   ~autoIndent() { m_l.adjustIndent(-m_nStops); }
private:
   iLog& m_l;
   size_t m_nStops;
};

} // namespace log
} // namespace prattle
