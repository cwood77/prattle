#pragma once
#include <functional>
#include <list>
#include <map>
#include <stdexcept>
#include <vector>

namespace prattle {

class attributable {
public:
   template<class A> typename A::ValueType& operator[](A& attr)
   { return attr.createOrFetch(*this); }
};

template<class T>
class attribute {
public:
   typedef T ValueType;

   T& createOrFetch(attributable& n) { return m_values[&n]; }

private:
   std::map<attributable*,T> m_values;
};

class node;

class iNodeVisitor {
public:
   virtual void visit(node& n) = 0;

protected:
   void visitChildren(node& n);
};

template<class V>
class visitorAcceptor {
public:
   explicit visitorAcceptor(iNodeVisitor& v) : m_b(v), m_pD(dynamic_cast<V*>(&v)) {}
   template<class N> void tryVisit(N *pNode)
   { if(m_pD) m_pD->visit(*pNode); else m_b.visit(*pNode); }

private:
   iNodeVisitor& m_b;
   V *m_pD;
};

class node : public attributable {
public:
   node() : lineNumber(0), m_pParent(NULL) {}
   virtual ~node();

   void appendChild(node& n);
   template<class T> T& appendChild() { auto *p = new T(); appendChild(*p); return *p; }

   std::vector<node*>& getChildren() { return m_children; }
   size_t getIndexOfChild(node& child);
   node *getParent() { return m_pParent; }
   node& demandParent();
   node& getRoot() { return m_pParent ? m_pParent->getRoot() : *this; }

   template<class T> T& demandAncestor()
   {
      if(m_pParent)
      {
         T *pD = dynamic_cast<T*>(m_pParent);
         if(pD)
            return *pD;
         return m_pParent->demandAncestor<T>();
      }
      throw std::runtime_error("ancestor not found");
   }

   template<class T> void searchDown(
      std::vector<T*>& ans,
      std::function<bool(T&)> pred = [](auto&){ return true; })
   {
      T *pD = dynamic_cast<T*>(this);
      if(pD && pred(*pD))
         ans.push_back(pD);
      for(auto *pChild : m_children)
         pChild->searchDown<T>(ans,pred);
   }

   template<class T> T *findDown(std::function<bool(T&)> pred = [](auto&){ return true; })
   {
      std::vector<T*> ans;
      searchDown<T>(ans,pred);
      if(ans.size() == 0)
         return NULL;
      if(ans.size() > 1)
         throw std::runtime_error("too many hits");
      return *ans.begin();
   }

   template<class T> T& demandDown(std::function<bool(T&)> pred = [](auto&){ return true; })
   {
      T *pAns = findDown<T>(pred);
      if(!pAns)
         throw std::runtime_error("no match found");
      return *pAns;
   }

   virtual const char *getName() const { return "node"; }
   virtual void acceptVisitor(iNodeVisitor& v) { v.visit(*this); }

   // lexical breadcrumbs
   std::string filePath;
   unsigned long lineNumber;

   void Delete();
   void replace(node& n);
   template<class T> T& replace() { auto *p = new T(); replace(*p); return *p; }
   void reparent(node& newParent, node *pAfterSibling = NULL);
   void reparentChildren(node& newParent, node *pAfterSibling = NULL);

private:
   std::vector<node*> m_children;
   node *m_pParent;

   node(const node&);
   node& operator=(const node&);
};

#define cdwImplNode(__name__,__visitor__) \
public: \
   virtual const char *getName() const { return #__name__; } \
   virtual void acceptVisitor(iNodeVisitor& v) \
   { visitorAcceptor<__visitor__>(v).tryVisit(this); } \

class nodeEditOperation {
public:
   ~nodeEditOperation();

   void Delete(node& old);
   void replace(node& old, node& nu);
   void reparent(node& n, node& newParent, node *pAfterSibling = NULL);
   void reparentChildren(node& n, node& newParent, node *pAfterSibling = NULL);
   void defer(std::function<void(void)> f);
   void commit();

private:
   std::list<node*> m_deletes;
   std::list<std::pair<node*,node*> > m_replaces;
   std::list<std::pair<node*,std::pair<node*,node*> > > m_reparents;
   std::list<std::pair<node*,std::pair<node*,node*> > > m_reparentChildren;
   std::list<std::function<void(void)> > m_defers;
};

class nodeEditCollector {
public:
   static nodeEditCollector*& head();

   explicit nodeEditCollector(nodeEditOperation& op);
   ~nodeEditCollector();

   nodeEditOperation& op;

private:
   nodeEditCollector *m_pPrev;
};

} // namespace prattle
