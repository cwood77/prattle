#include "node.hpp"

namespace prattle {

void iNodeVisitor::visitChildren(node& n)
{
   auto& children = n.getChildren();
   for(auto *pChild : children)
      pChild->acceptVisitor(*this);
}

node::~node()
{
   for(auto *pChild : m_children)
      delete pChild;
}

void node::appendChild(node& n)
{
   n.m_pParent = this;
   m_children.push_back(&n);
}

} // namespace prattle
