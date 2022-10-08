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

node& node::demandParent()
{
   if(m_pParent == NULL)
      throw std::runtime_error("parent node required");
   return *m_pParent;
}

void node::Delete()
{
   auto& children = demandParent().getChildren();
   for(auto it=children.begin();it!=children.end();++it)
   {
      if(*it == this)
      {
         children.erase(it);
         delete this;
         return;
      }
   }
   throw std::runtime_error("ise");
}

void node::replace(node& n)
{
   auto& p = demandParent();
   auto& children = p.getChildren();
   for(auto it=children.begin();it!=children.end();++it)
   {
      if(*it == this)
      {
         *it = &n;
         n.m_pParent = &p;
         delete this;
         return;
      }
   }
   throw std::runtime_error("ise");
}

} // namespace prattle
