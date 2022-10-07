#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>

class node;

class iNodeVisitor {
public:
   virtual void visit(node& n) = 0;
};

class nodeRef;

class node {
public:
   virtual ~node();

   void appendChild(node& n);

   template<class T> T& demandAncestor();

   virtual void acceptVisitor(iNodeVisitor& v) { v.visit(*this); }

protected:
   node();

private:
   std::vector<node*> m_children;
   node *m_pParent;
   std::set<nodeRef*> m_refs;

   node(const node&);
   node& operator=(const node&);
};

class nodeRef {
public:
   nodeRef();
   ~nodeRef();
   nodeRef(const nodeRef& source);
   nodeRef& operator=(const nodeRef& source);

   void bind(node& target);

private:
   node *m_pTarget;
};

class sectionToC {
};

class rootNode : public node {
public:
   std::unique_ptr<sectionToC> *pToc;
};

class sectionNode : public node {
public:
   std::string name;
};

class paragraphNode : public node {
public:
   std::string text;
};

class conditionNode : public node {
};

class choiceNode : public conditionNode {
};

class testNode : public conditionNode {
public:
   std::string attribute;
};

class gotoNode : public node {
public:
   std::string sectionName;
   sectionNode *pTargetLink;
};

class keepReadingNode : public node {
};

// transforms:
// - remove keep readings
// - generate keep readings
// - randomize/rename section names (repeatable)

// other:
// - generate a dot file
// - write the sections out in order
//
