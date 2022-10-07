#include "../prattle/node.hpp"
#include <sstream>

using namespace prattle;

namespace {

class rootNode;
class leftChild;
class rightChild;

class iMyNodeVisitor : public iNodeVisitor {
public:
   virtual void visit(rootNode& n) = 0;
   virtual void visit(leftChild& n) = 0;
   virtual void visit(rightChild& n) = 0;
};

class rootNode : public node { cdwImplNode(rootNode,iMyNodeVisitor); };
class leftChild : public node { cdwImplNode(leftChild,iMyNodeVisitor); };
class rightChild : public node { cdwImplNode(rightChild,iMyNodeVisitor); };

class streamingNodeVisitor : public iMyNodeVisitor {
public:
   std::stringstream s;

   virtual void visit(node& n)
   {
      s << n.getName();
      visitChildren(n);
   }

   virtual void visit(rootNode& n)
   {
      s << n.getName();
      visitChildren(n);
   }

   virtual void visit(leftChild& n)
   {
      s << n.getName();
      visitChildren(n);
   }

   virtual void visit(rightChild& n)
   {
      s << n.getName();
      visitChildren(n);
   }
};

} // namespace anonymous

void nodeTest()
{
   rootNode h;
   auto& l = h.appendChild<leftChild>();
   auto& r = h.appendChild<rightChild>();

   // root/parents
   auto &x = r.getRoot();
   if(&x != &h)
      throw std::runtime_error("fail");

   // ancestors
   auto &y = r.demandAncestor<rootNode>();
   if(&y != &h)
      throw std::runtime_error("fail");

   // search/find/demand down
   auto& z = h.demandDown<leftChild>();
   if(&z != &l)
      throw std::runtime_error("fail");

   // attributes
   attribute<size_t> a;
   if(r[a] != 0)
      throw std::runtime_error("fail");
   r[a] = 12;
   if(r[a] != 12)
      throw std::runtime_error("fail");

   // visitors
   streamingNodeVisitor v;
   h.acceptVisitor(v);
   if(v.s.str() != "rootNodeleftChildrightChild")
      throw std::runtime_error(v.s.str());
}
