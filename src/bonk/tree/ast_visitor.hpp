#pragma once

#include "nodes/var_definition.hpp"
#include "nodes/identifier.hpp"
#include "nodes/block_definition.hpp"
#include "nodes/operator.hpp"
#include "nodes/cycle.hpp"
#include "nodes/call_parameter.hpp"
#include "nodes/call.hpp"
#include "nodes/check.hpp"
#include "nodes/number.hpp"

namespace bonk {

class ASTVisitor {
  public:
    virtual void visit(TreeNodeVariableDefinition* node) = 0;
    virtual void visit(TreeNodeIdentifier* node) = 0;
    virtual void visit(TreeNodeBlockDefinition* node) = 0;
    virtual void visit(TreeNodeOperator* node) = 0;
    virtual void visit(TreeNodeCycle* node) = 0;
    virtual void visit(TreeNodeCallParameter* node) = 0;
    virtual void visit(TreeNodeCall* node) = 0;
    virtual void visit(TreeNodeCheck* node) = 0;
    virtual void visit(TreeNodeNumber* node) = 0;
    virtual void visit(TreeNodeList* node) = 0;
};

}