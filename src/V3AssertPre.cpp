// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
// DESCRIPTION: Verilator: Collect and print statistics
//
// Code available from: http://www.veripool.org/verilator
//
//*************************************************************************
//
// Copyright 2005-2018 by Wilson Snyder.  This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
//
// Verilator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//*************************************************************************
//  Pre steps:
//	Attach clocks to each assertion
//*************************************************************************

#include "config_build.h"
#include "verilatedos.h"
#include <cstdio>
#include <cstdarg>
#include <unistd.h>
#include <map>
#include <iomanip>

#include "V3Global.h"
#include "V3AssertPre.h"
#include "V3Ast.h"

//######################################################################
// Assert class functions

class AssertPreVisitor : public AstNVisitor {
    // Removes clocks and other pre-optimizations
    // Eventually inlines calls to sequences, properties, etc.
    // We're not parsing the tree, or anything more complicated.
private:
    // NODE STATE/TYPES
    // STATE
    // Reset each module:
    AstNodeSenItem*	m_seniDefaultp;	// Default sensitivity (from AstDefClock)
    // Reset each assertion:
    AstNodeSenItem*	m_senip;	// Last sensitivity

    // METHODS
    VL_DEBUG_FUNC;  // Declare debug()

    AstSenTree* newSenTree(AstNode* nodep) {
	// Create sentree based on clocked or default clock
	// Return NULL for always
	AstSenTree* newp = NULL;
	AstNodeSenItem* senip = m_senip ? m_senip : m_seniDefaultp;
	if (!senip) {
	    nodep->v3error("Unsupported: Unclocked assertion");
	    newp = new AstSenTree(nodep->fileline(), NULL);
	} else {
	    newp = new AstSenTree(nodep->fileline(), senip->cloneTree(true));
	}
	return newp;
    }
    void clearAssertInfo() {
	m_senip = NULL;
    }

    // VISITORS  //========== Statements
    virtual void visit(AstClocking* nodep) {
	UINFO(8,"   CLOCKING"<<nodep<<endl);
	// Store the new default clock, reset on new module
	m_seniDefaultp = nodep->sensesp();
	// Trash it, keeping children
	if (nodep->bodysp()) {
	    nodep->replaceWith(nodep->bodysp()->unlinkFrBack());
	} else {
	    nodep->unlinkFrBack();
	}
	pushDeletep(nodep); VL_DANGLING(nodep);
    }

    virtual void visit(AstNodePslCoverOrAssert* nodep) {
	if (nodep->sentreep()) return;  // Already processed
	clearAssertInfo();
        iterateChildren(nodep);
	nodep->sentreep(newSenTree(nodep));
	clearAssertInfo();
    }
    virtual void visit(AstPslClocked* nodep) {
        iterateChildren(nodep);
	if (m_senip) {
	    nodep->v3error("Unsupported: Only one PSL clock allowed per assertion");
	}
	// Block is the new expression to evaluate
	AstNode* blockp = nodep->propp()->unlinkFrBack();
	if (nodep->disablep()) {
	    blockp = new AstAnd(nodep->disablep()->fileline(),
				new AstNot(nodep->disablep()->fileline(),
					   nodep->disablep()->unlinkFrBack()),
				blockp);
	}
	// Unlink and just keep a pointer to it, convert to sentree as needed
	m_senip = nodep->sensesp();
	nodep->replaceWith(blockp);
	pushDeletep(nodep); VL_DANGLING(nodep);
    }
    virtual void visit(AstNodeModule* nodep) {
        iterateChildren(nodep);
	// Reset defaults
	m_seniDefaultp = NULL;
    }
    virtual void visit(AstNode* nodep) {
        iterateChildren(nodep);
    }

public:
    // CONSTRUCTORS
    explicit AssertPreVisitor(AstNetlist* nodep) {
	m_seniDefaultp = NULL;
	clearAssertInfo();
	// Process
        iterate(nodep);
    }
    virtual ~AssertPreVisitor() {}
};

//######################################################################
// Top Assert class

void V3AssertPre::assertPreAll(AstNetlist* nodep) {
    UINFO(2,__FUNCTION__<<": "<<endl);
    {
        AssertPreVisitor visitor (nodep);
    }  // Destruct before checking
    V3Global::dumpCheckGlobalTree("assertpre", 0, v3Global.opt.dumpTreeLevel(__FILE__) >= 3);
}
