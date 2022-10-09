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
   nodeEditCollector*& pHead = nodeEditCollector::head();
   if(pHead)
   {
      pHead->op.Delete(*this);
      return;
   }

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
   nodeEditCollector*& pHead = nodeEditCollector::head();
   if(pHead)
   {
      pHead->op.replace(*this,n);
      return;
   }

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

void node::reparent(node& newParent, node *pAfterSibling)
{
   if(!pAfterSibling)
      throw std::runtime_error("reparent with no afterSibling unimplemented");

   nodeEditCollector*& pHead = nodeEditCollector::head();
   if(pHead)
   {
      pHead->op.reparent(*this,newParent,pAfterSibling);
      return;
   }

   // unhook myself from my current parent
   {
      auto& children = demandParent().getChildren();
      for(auto it=children.begin();it!=children.end();++it)
      {
         if(*it == this)
         {
            children.erase(it);
            break;
         }
      }
   }

   // sign up with my new parent
   {
      auto& children = newParent.getChildren();
      for(auto it=children.begin();it!=children.end();++it)
      {
         if(*it == pAfterSibling)
         {
            ++it; // C++ insert takes a 'before' iterator
            children.insert(it, this);
            this->m_pParent = &newParent;
            return;
         }
      }
      throw std::runtime_error("can't find afterSibling in reparent");
   }
}

void node::reparentChildren(node& newParent, node *pAfterSibling)
{
   std::vector<node*> copy = m_children;
   for(auto it=copy.rbegin();it!=copy.rend();++it)
      (*it)->reparent(newParent,pAfterSibling);
}

nodeEditOperation::~nodeEditOperation()
{
   for(auto it=m_replaces.begin();it!=m_replaces.end();++it)
      delete it->second;
}

void nodeEditOperation::Delete(node& old)
{
   m_deletes.push_back(&old);
}

void nodeEditOperation::replace(node& old, node& nu)
{
   m_replaces.push_back(std::make_pair<node*,node*>(&old,&nu));
}

void nodeEditOperation::reparent(node& n, node& newParent, node *pAfterSibling)
{
   m_reparents.push_back(
      std::make_pair<node*,std::pair<node*,node*> >(
         &n,
         std::make_pair<node*,node*>(&newParent,&*pAfterSibling)));
}

void nodeEditOperation::commit()
{
   // commit reparents
   for(auto it=m_reparents.begin();it!=m_reparents.end();++it)
      it->first->reparent(*it->second.first,it->second.second);
   m_reparents.clear();

   // commit replaces
   for(auto it=m_replaces.begin();it!=m_replaces.end();++it)
      it->first->replace(*it->second);
   m_replaces.clear();

   // commit deletes
   for(auto it=m_deletes.begin();it!=m_deletes.end();++it)
      (*it)->Delete();
   m_deletes.clear();
}

nodeEditCollector*& nodeEditCollector::head()
{
   static nodeEditCollector *the(NULL);
   return the;
}

nodeEditCollector::nodeEditCollector(nodeEditOperation& op)
: op(op)
, m_pPrev(NULL)
{
   nodeEditCollector*& pPtr = head();
   m_pPrev = pPtr;
   pPtr = this;
}

nodeEditCollector::~nodeEditCollector()
{
   nodeEditCollector*& pPtr = head();
   pPtr = m_pPrev;
}

} // namespace prattle
