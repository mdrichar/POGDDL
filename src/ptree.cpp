/************************************************************************
 * Copyright 2008, Strathclyde Planning Group,
 * Department of Computer and Information Sciences,
 * University of Strathclyde, Glasgow, UK
 * http://planning.cis.strath.ac.uk/
 *
 * Maria Fox, Richard Howey and Derek Long - VAL
 * Stephen Cresswell - PDDL Parser
 *
 * This file is part of VAL, the PDDL validator.
 *
 * VAL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * VAL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with VAL.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************/


/*-----------------------------------------------------------------------------
  Member functions for parse tree classes

  $Date: 2009-02-11 17:20:38 $
  $Revision: 1.4 $

  stephen.cresswell@cis.strath.ac.uk
  July 2001.

  Strathclyde Planning Group
 
 ----------------------------------------------------------------------------
*/


#include "ptree.h"
#include "macros.h"
#include "DebugWriteController.h"
#include "VisitController.h"
#include <memory>
#include <sstream>
#include <fstream>
#include <iterator>

/*-----------------------------------------------------------------------------
  Parse category
  ---------------------------------------------------------------------------*/

namespace VAL {

double get_clock() {
  struct timeval tv; int ok;
  struct timezone tz;
  ok = gettimeofday(&tv, &tz);
  if (ok<0) { printf("gettimeofday error"); }
  return (tv.tv_sec * 1.0 + tv.tv_usec * 1.0E-6);
}

string asIntString(const FunctionKey& fk) {
	std::ostringstream os;
	//os << name << " ";
	copy(fk.begin(),fk.end(),std::ostream_iterator<int>(os," "));
	return os.str();
}

int Counter::varListId;
void Counter::resetVarListId()
{
  varListId = 0;
}

int Counter::getVarListIdAndInc()
{
  return varListId++;
}

void gain_def::write(ostream & o) const
{
  o << "Gain def";
}

void gain_def::display(int ind) const
{

}


auto_ptr<WriteController> parse_category::wcntr = auto_ptr<WriteController>(new DebugWriteController);

WriteController * parse_category::recoverWriteController()
{
	return wcntr.release();
};

void parse_category::setWriteController(auto_ptr<WriteController> w) {wcntr = w;};

void parse_category::display(int ind) const
{
    TITLE(parse_category);
}


ostream & operator <<(ostream & o,const parse_category & p)
{
	p.write(o);
	return o;
};


/*-----------------------------------------------------------------------------
  ---------------------------------------------------------------------------*/

void symbol::display(int ind) const 
{ 
    TITLE(symbol);
    LEAF(name); 
}

void symbol::write(ostream & o) const
{
	wcntr->write_symbol(o,this);
};

void pddl_typed_symbol::display(int ind) const
{ 
    TITLE(symbol);
    LEAF(name); cout << "[" << this << "]\n";
    FIELD(type);
    FIELD(either_types);
}

void pddl_typed_symbol::write(ostream & o) const
{
	wcntr->write_pddl_typed_symbol(o,this);
};

void pddl_typed_symbol::write2(ostream& o, int ind = 0) const 
{
	indento(ind,o);
	o << this->getName();
	if (type) o << ":" << type->getName();
}

void var_symbol::write2(ostream& o, int ind = 0) const 
{
  pddl_typed_symbol::write2(o,ind);
  o << "(" << index << ") ";
}

void const_symbol::write(ostream & o) const
{
	wcntr->write_const_symbol(o,this);
};

/*-----------------------------------------------------------------------------
  ---------------------------------------------------------------------------*/

void pred_decl_list::write(ostream & o) const
{
	wcntr->write_pred_decl_list(o,this);
};

void func_decl_list::write(ostream & o) const
{
	wcntr->write_func_decl_list(o,this);
};

void var_symbol::write(ostream & o) const 
{
	wcntr->write_var_symbol(o,this);
};


/*-----------------------------------------------------------------------------
  ---------------------------------------------------------------------------*/

void plus_expression::display(int ind) const
{
    TITLE(plus_expression);
    FIELD(arg1);
    FIELD(arg2);
}

void plus_expression::write(ostream & o) const
{
	wcntr->write_plus_expression(o,this);
};

void minus_expression::display(int ind) const
{
    TITLE(minus_expression);
    FIELD(arg1);
    FIELD(arg2);
}

void minus_expression::write(ostream & o) const
{
	wcntr->write_minus_expression(o,this);
};

void mul_expression::display(int ind) const
{
    TITLE(mul_expression);
    FIELD(arg1);
    FIELD(arg2);
}

void mul_expression::write(ostream & o) const
{
	wcntr->write_mul_expression(o,this);
};

void div_expression::display(int ind) const
{
    TITLE(div_expression);
    FIELD(arg1);
    FIELD(arg2);
}

void div_expression::write(ostream & o) const
{
	wcntr->write_div_expression(o,this);
};

void uminus_expression::display(int ind) const
{
    TITLE(uminus_expression);
    FIELD(arg1);
}

void uminus_expression::write(ostream & o) const
{
	wcntr->write_uminus_expression(o,this);
};

void int_expression::display(int ind) const
{
    TITLE(int_expression);
    LEAF(val);
}

void int_expression::write(ostream & o) const
{
	wcntr->write_int_expression(o,this);
};

void float_expression::display(int ind) const
{
    TITLE(int_expression);
    LEAF(val);
}

void float_expression::write(ostream & o) const
{
	wcntr->write_float_expression(o,this);
};

void special_val_expr::display(int ind) const
{
    TITLE(special_val_expr);
    if (var==E_HASHT)
	cout << "hasht";
    else if (var==E_DURATION_VAR)
	cout << "?duration";
    else if (var==E_TOTAL_TIME)
    cout << "total-time";
    else
	cout << "?? ";
}

void special_val_expr::write(ostream & o) const
{
	wcntr->write_special_val_expr(o,this);
};

void func_term::display(int ind) const
{
    TITLE(func_term);
    FIELD(func_sym);
    FIELD(param_list);
}

void func_term::write(ostream & o) const
{
	wcntr->write_func_term(o,this);
};

void assignment::display(int ind) const
{
    TITLE(assignment);
    LEAF(op);
    FIELD(f_term);
    FIELD(expr);
}

void assignment::write(ostream & o) const
{
	wcntr->write_assignment(o,this);
};

void goal_list::display(int ind) const
{
    TITLE(goal_list);
    for(const_iterator i=begin(); i!=end(); ++i)
	ELT(*i);
}

void goal_list::write(ostream & o) const
{
	wcntr->write_goal_list(o,this);
};

void goal::display(int ind) const {};
void goal::write(ostream & o) const {};

void simple_goal::display(int ind) const
{
/*
    if (plrty == E_POS)
	cout << '+';
    else
	cout << '-';
*/
    FIELD(prop);
};

void simple_goal::write(ostream & o) const
{
	wcntr->write_simple_goal(o,this);
};

void simple_goal::write2(ostream & o, int ind) const
{
  indento(ind,o);
  o << (plrty == E_NEG ? "-" : "+");
  prop->write2(o,ind);
}

void constraint_goal::display(int ind) const
{
	TITLE(constraint_goal);
	LEAF(cons);
	FIELD(requirement);
	FIELD(trigger);
	LEAF(deadline);
	LEAF(from);
};

void constraint_goal::write(ostream & o) const 
{
	wcntr->write_constraint_goal(o,this);
};

void preference::display(int ind) const
{
	TITLE(preference);
	LEAF(name);
	FIELD(gl);
};

void preference::write(ostream & o) const
{
	wcntr->write_preference(o,this);
};

void qfied_goal::display(int ind) const
{
    TITLE(qfied_goal);
    
    LABEL(qfier);
    if (qfier == E_FORALL)
	cout << "forall";
    else if (qfier == E_EXISTS)
	cout << "exists";
    else 
	cout << "?quantifier";

    FIELD(vars);
    FIELD(gl);

}

void qfied_goal::write(ostream & o) const
{
	wcntr->write_qfied_goal(o,this);
};

void qfied_goal::write2(ostream & o, int ind) const
{
    indento(ind,o);
    if (qfier == E_FORALL)
	o << "forall";
    else if (qfier == E_EXISTS)
	o << "exists";
    else 
	o << "?quantifier";
    sym_tab->write2(o,ind);
    gl->write2(o,ind);
}

void conj_goal::display(int ind) const
{
    TITLE(conj_goal);
    FIELD(goals);
}

void conj_goal::write(ostream & o) const
{
	wcntr->write_conj_goal(o,this);
};

void conj_goal::write2(ostream & o, int ind) const
{
  indento(ind,o);
  o << "conjunction\n";
  goals->write2(o,ind+1);
}


void disj_goal::display(int ind) const
{
    TITLE(disj_goal);
    FIELD(goals);
}

void disj_goal::write(ostream & o) const
{
	wcntr->write_disj_goal(o,this);
};

void disj_goal::write2(ostream & o, int ind) const
{
  indento(ind,o);
  o << "disjunction\n";
  goals->write2(o,ind+1);
}

void timed_goal::display(int ind) const
{
    TITLE(timed_goal);
    LEAF(ts);
    FIELD(gl);
}

void timed_goal::write(ostream & o) const
{
	wcntr->write_timed_goal(o,this);
};

void imply_goal::display(int ind) const
{
    TITLE(imply_goal);
    FIELD(lhs);
    FIELD(rhs);
}

void imply_goal::write(ostream & o) const
{
	wcntr->write_imply_goal(o,this);
};

void neg_goal::display(int ind) const
{
    TITLE(neg_goal);
    FIELD(gl);
}

void neg_goal::write2(ostream & o, int ind) const
{
  indento(ind,o);
  o << "negation: ";
  gl->write2(o,ind);
}

void neg_goal::write(ostream & o) const
{
	wcntr->write_neg_goal(o,this);
};

void comparison::display(int ind) const
{
    TITLE(comparison);
    LEAF(op);
    FIELD(arg1);
    FIELD(arg2);
}

void comparison::write(ostream & o) const
{
	wcntr->write_comparison(o,this);
};

void proposition::display(int ind) const
{
    TITLE(prop);
    FIELD(head);
    FIELD(args);
};

void proposition::write2(ostream & o, int ind) const
{
  o << "\n";
  indento(ind, o);
  head->write2(o,ind);
  args->write2(o,ind);
  
}

void proposition::write(ostream & o) const
{
	wcntr->write_proposition(o,this);
};

int proposition::lastParamUsed() const 
{ 
  if (argMap.empty()) return -1; // some propositions have 0 args, treat them like those with constants only
  return *std::max_element(argMap.begin(),argMap.end()); 
}

void pred_decl::display(int ind) const
{
    TITLE(pred_decl);
    FIELD(head);
    FIELD(args);
};

void pred_decl::write2(ostream & o, int ind = 0) const
{
	indento(ind,o);
    //TITLE(pred_decl);
    head->write2(o,ind);
    o << " --  ";
    //var_tab->write2(o,ind);
    args->write2(o,ind);
    //DFIELD(head,o);
    //DFIELD(args,o);
};

void pred_decl::write(ostream & o) const
{
	wcntr->write_pred_decl(o,this);
};

void func_decl::display(int ind) const
{
    TITLE(func_decl);
    FIELD(head);
    FIELD(args);
};

void func_decl::write(ostream & o) const
{
	wcntr->write_func_decl(o,this);
};


void simple_effect::display(int ind) const
{
    TITLE(simple_effect);
    FIELD(prop);
}

void simple_effect::write(ostream & o) const
{
	wcntr->write_simple_effect(o,this);
};

void forall_effect::display(int ind) const
{
    TITLE(forall_effect);
    FIELD(operand);
//    FIELD(var_tab);
}

void forall_effect::write(ostream & o) const
{
	wcntr->write_forall_effect(o,this);
};


void cond_effect::display(int ind) const
{
    TITLE(cond_effect);
    FIELD(cond);
    FIELD(effects);
}

void cond_effect::write(ostream & o) const
{
	wcntr->write_cond_effect(o,this);
};

void timed_effect::display(int ind) const
{
    TITLE(timed_effect);
    LEAF(ts);
    FIELD(effs);
}

void timed_effect::write(ostream & o) const
{
	wcntr->write_timed_effect(o,this);
};

void timed_initial_literal::display(int ind) const
{
	TITLE(timed_initial_literal);
	LEAF(ts);
	LEAF(time_stamp);
	FIELD(effs);
};

void timed_initial_literal::write(ostream & o) const
{
	wcntr->write_timed_initial_literal(o,this);
};

/*-----------------------------------------------------------------------------
  effect_lists
  ---------------------------------------------------------------------------*/

void effect_lists::append_effects(effect_lists* from)
{
    // Splice lists in 'from' into lists in 'this'
    add_effects.splice(
	add_effects.begin(),
	from->add_effects);
    del_effects.splice(
	del_effects.begin(),
	from->del_effects);
    forall_effects.splice(
	forall_effects.begin(),
	from->forall_effects);
    cond_effects.splice(
	cond_effects.begin(),
	from->cond_effects);
    cond_assign_effects.splice(
        cond_assign_effects.begin(),
        from->cond_assign_effects);
    assign_effects.splice(
	assign_effects.begin(),
	from->assign_effects);
    timed_effects.splice(
	timed_effects.begin(),
	from->timed_effects);
    observation_effects.splice(
	observation_effects.begin(),
	from->observation_effects);
}

void effect_lists::display(int ind) const
{
    TITLE(effect_lists);

    //list<simple_effect*>::iterator i;

    LABEL(add_effects);
    add_effects.display(ind);

    LABEL(del_effects);
    del_effects.display(ind);

    LABEL(forall_effects);
    forall_effects.display(ind);

    LABEL(cond_effects);
    cond_effects.display(ind);

    LABEL(cond_assign_effects);
    cond_assign_effects.display(ind);

    LABEL(assign_effects);
    assign_effects.display(ind);

    LABEL(timed_effects);
    timed_effects.display(ind);

    LABEL(observation_effects);
    observation_effects.display(ind);
}

void effect_lists::write(ostream & o) const
{
	wcntr->write_effect_lists(o,this);
};

void effect_lists::write2(ostream & o, int ind = 0) const
{
	if (!add_effects.empty()) o << "\nADD EFFECTS\n";
	add_effects.write2(o,ind);
	if (!del_effects.empty()) o << "\nDELETE EFFECTS\n";
	del_effects.write2(o,ind);
	if (!forall_effects.empty()) o << "\nFORALL EFFECTS\n";
	forall_effects.write2(o,ind);
	if (!cond_effects.empty()) o << "\nCONDITIONAL EFFECTS\n";
	cond_effects.write2(o,ind);
	if (!cond_assign_effects.empty()) o << "\nCONDITIONAL ASSIGNMENT EFFECTS\n";
	cond_assign_effects.write2(o,ind);
	if (!assign_effects.empty()) o << "\nASSIGNMENT EFFECTS\n";
	assign_effects.write2(o,ind);
	if (!timed_effects.empty()) o << "\nTIMED EFFECTS\n";
	timed_effects.write2(o,ind);
	if (!observation_effects.empty()) o << "\nREVELATIONS\n";
	observation_effects.write2(o,ind);
}

void simple_effect::write2(ostream & o, int ind) const
{
	indento(ind, o);
	o << "se ";
	prop->write2(o,ind);

}

void assignment::write2(ostream & o, int ind) const
{
	indento(ind, o);
	o << "assgn ";
	f_term->write2(o,ind);
 	o << " " << op << " expr";	
	expr->write2(o,ind);

}

void recipient::write2(ostream & o, int ind) const
{
	indento(ind, o);
	o << "receivers";
        receivers->write2(o,ind);
	o << "prop";
        prop->write2(o,ind);
}

void observation::write2(ostream & o, int ind) const
{
	o << "rev ";
	indento(ind, o);
        assert (receivers);
	for (recipients::const_iterator itr = receivers->begin(); itr != receivers->end(); ++itr) {
	    (*itr)->write2(o,ind);
        }

}

void forall_effect::write2(ostream & o, int ind) const
{
	indento(ind, o);
	o << "fa ";

}

void cond_effect::write2(ostream & o, int ind) const
{
	indento(ind, o);
	o << "cnd|";
	cond->write2(o,ind);
        o << "|effs ";
	effects->write2(o,ind);

}

void timed_effect::write2(ostream & o, int ind) const
{
	indento(ind, o);
	o << "te ";

}

void operator_list::display(int ind) const
{
    TITLE(operator_list);
    for(const_iterator i=begin(); i!=end(); ++i)
	ELT(*i);
}

void operator_list::write(ostream & o) const
{
	wcntr->write_operator_list(o,this);
};

void operator_list::write2(ostream & o, int ind = 0) const
{
    for(const_iterator i=begin(); i!=end(); ++i)
	(*i)->write2(o,ind);
};

void operator_::display(int ind) const
{
    TITLE(operator_);
    FIELD(name);
    FIELD(parameters);
    FIELD(precondition);
    FIELD(effects);
}

void operator_::write2(ostream& o, int ind) const
{
	indento(ind,o);
    o << "op " << name->getName() << " ";
    if (parameters) parameters->write2(o,ind);
    if (precondition) precondition->write2(o,ind+1);
    if (effects) effects->write2(o,ind);
    o << "\nQUICKCHECKS: +";
    for(SetInt::const_iterator itr = prePos.begin(); itr != prePos.end(); ++itr) {
      o << *itr << " ";
    }
    o << " Negs: ";
    for(SetInt::const_iterator it = preNeg.begin(); it != preNeg.end(); ++it) {
      o << *it << " ";
    }
    for (unsigned i = 0; i < prePosList.size(); i++) {
      o << "(" << i << ")+ ";
      for (unsigned j = 0; j < prePosList[i].size(); ++j) {
        prePosList[i][j]->write2(o,ind); 
      } 
    }
    for (unsigned i = 0; i < preNegList.size(); i++) {
      o << " Neg: " ;
      for (unsigned j = 0; j < preNegList[i].size(); ++j) {
        preNegList[i][j]->write2(o,ind); 
      } 
    }
    o << "\n";
}

void derivations_list::write(ostream & o) const
{
	wcntr->write_derivations_list(o,this);
};

void derivations_list::display(int ind) const
{
	TITLE(derivations_list);
	for(const_iterator i = begin();i != end();++i)
		ELT(*i);
};

void derivation_rule::write(ostream & o) const
{
	wcntr->write_derivation_rule(o,this);
};

void derivation_rule::write2(ostream & o, unsigned ind) const
{
  o << "HEAD: ";
  head->write2(o,ind);
  o << "body: ";
  body->write2(o,ind);
  o << "Symbol Table";
  vtab->write2(o,ind);
  o << "\n";
}

void derivation_rule::display(int ind) const
{
	TITLE(derivation_rule);
	FIELD(head);
	FIELD(body);
};


void operator_::write(ostream & o) const
{
	wcntr->write_operator_(o,this);
};

void action::display(int ind) const
{
    TITLE(action);
    FIELD(name);
    FIELD(parameters);
    FIELD(precondition);
    FIELD(effects);
}

void action::write(ostream & o) const
{
	wcntr->write_action(o,this);
};

void event::display(int ind) const
{
    TITLE(event);
    FIELD(name);
    FIELD(parameters);
    FIELD(precondition);
    FIELD(effects);
}

void event::write(ostream & o) const
{
	wcntr->write_event(o,this);
};

void process::display(int ind) const
{
    TITLE(process);
    FIELD(name);
    FIELD(parameters);
    FIELD(precondition);
    FIELD(effects);
}

void process::write(ostream & o) const
{
	wcntr->write_process(o,this);
};

void durative_action::display(int ind) const
{
    TITLE(durative_action);
    FIELD(name);
    FIELD(parameters);
    FIELD(dur_constraint);
    FIELD(precondition);
    FIELD(effects);
}

void durative_action::write(ostream & o) const
{
	wcntr->write_durative_action(o,this);
};

void domain::display(int ind) const
{
    TITLE(domain);
    LEAF(name);
    LEAF(req); indent(ind); cout << pddl_req_flags_string(req);
//    FIELD(types);
//    FIELD(constants);
//    FIELD(pred_vars);
    FIELD(predicates); 
    FIELD(ops);
    FIELD(drvs);
}

void domain::write(ostream & o) const
{
	wcntr->write_domain(o,this);
};

void domain::write2(ostream & o, int ind = 0) const
{
	o << "\n";
	indento(ind,o);
	//wcntr->write_domain(o,this);
	o << "DOMAIN NAME: " << name << "\n";
	indento(ind,o);
	o << "PREDICATES DEFS:\n";
        predicates->write2(o,ind);
	o << "\n";
	indento(ind,o);
	o << "OPERATORS:";
        ops->write2(o,ind);
	//o << "\nDerivations:" << drvs->write2(o);
};

void violation_term::display(int ind) const
{
	TITLE(violation_term);
	LEAF(name);
}

void violation_term::write(ostream & o) const
{
	o << "(is-violated " << name << ")";
}

void metric_spec::display(int ind) const
{
    TITLE(metric_spec);
    LEAF(opt);
    FIELD(expr);
}

void metric_spec::write(ostream & o) const
{
	wcntr->write_metric_spec(o,this);
};

void length_spec::display(int ind) const
{
    TITLE(length_spec);
    LEAF(mode);
    LEAF(lengths);
    LEAF(lengthp);
}

void length_spec::write(ostream & o) const
{
	wcntr->write_length_spec(o,this);
};

void problem::display(int ind) const
{
    TITLE(problem);
    LEAF(req); indent(ind+1); cout << pddl_req_flags_string(req);
    FIELD(types);
    FIELD(objects);
    FIELD(initial_state);
    FIELD(the_goal);
    FIELD(constraints);
    FIELD(metric);
    FIELD(length);
};

void problem::write(ostream & o) const
{
	wcntr->write_problem(o,this);
};

void problem::write2(ostream & o, int ind) const
{

}

void plan_step::display(int ind) const
{
    LEAF(start_time);
    FIELD(op_sym);
    FIELD(params);
    LEAF(duration);
}

void plan_step::write(ostream & o) const
{
	wcntr->write_plan_step(o,this);
};

};

void indent(int ind)
{
    cout << '\n';
    for (int i=0; i<ind; i++)
	cout << "   ";
}

void indento(int ind, std::ostream& o)
{
    //o << '\n';
    for (int i=0; i<ind; i++)
	o << "   ";
}

namespace VAL {
/*---------------------------------------------------------------------------
  Functions called from parser that perform checks, and possibly generate
  errors.
  --------------------------------------------------------------------------*/

bool types_defined = false;
bool types_used = false;

void requires(pddl_req_flag flags)
{
    if (!(flags & current_analysis->req))
	current_analysis->error_list.add(
	    E_WARNING,
	    "Undeclared requirement " + 
	    pddl_req_flags_string(flags));
}

void log_error(error_severity sev, const string& description)
{
    current_analysis->error_list.add(
	sev,
	description);
}

/*---------------------------------------------------------------------------
  Descriptions of requirements flags.
  Surely some way to avoid saying this again.
-----------------------------------------------------------------------------*/

string pddl_req_flags_string(pddl_req_flag flags)
{
    string result;

    if (flags & E_EQUALITY) result += ":equality ";
    if (flags & E_STRIPS) result += ":strips ";
    if (flags & E_TYPING) result += ":typing ";
    if (flags & E_DISJUNCTIVE_PRECONDS) 
	result += ":disjunctive-preconditions ";
    if (flags & E_EXT_PRECS) result += ":existential-preconditions ";
    if (flags & E_UNIV_PRECS) result += ":universal-preconditions ";
    if (flags & E_COND_EFFS) result += ":conditional-effects ";
    if (flags & E_FLUENTS) result += ":fluents ";
    if (flags & E_DURATIVE_ACTIONS) result += ":durative-actions ";
    if (flags & E_DURATION_INEQUALITIES) result += ":duration-inequalities ";
    if (flags & E_CONTINUOUS_EFFECTS) result += ":continuous-effects ";
    if (flags & E_NEGATIVE_PRECONDITIONS) result += ":negative-preconditions ";
    if (flags & E_DERIVED_PREDICATES) result += ":derived-predicates ";
    if (flags & E_TIMED_INITIAL_LITERALS) result += ":timed-initial-literals ";
    if (flags & E_PREFERENCES) result += ":preferences ";
    if (flags & E_CONSTRAINTS) result += ":constraints ";
    if (flags & E_TIME) result += ":time ";
    return result;
}


// Search tables from top of stack to find symbol matching name.
// If none found, add to top table.
// Return pointer to symbol.
var_symbol* var_symbol_table_stack::symbol_get(const string& name)
{
    var_symbol* sym= NULL;

    // Iterate through stack from top to bottom
    // (may need to change direction if changing underlying 
    // impl. of stack)
    for (iterator i=begin(); i!=end() && sym==NULL; ++i)
	sym= (*i)->symbol_probe(name);

    if (sym !=NULL)
	// return found symbol
	return sym;
    else
    {
	// Log a warning 
	// add new symbol to current table.
	log_error(E_WARNING,"Undeclared variable symbol: ?" + name);
	return top()->symbol_put(name);
    }
};

var_symbol* var_symbol_table_stack::symbol_put(const string& name)
{
    return top()->symbol_put(name);
};

void gain_def::add_to(operator_list* ops,derivations_list* drvs,gain_list* gains)
{
	gains->push_back(this);
};

/*--------------------------------------------------------------------
 * Visitor methods.
 *--------------------------------------------------------------------*/

void symbol::visit(VisitController *v) const {v->visit_symbol(this);};
void pred_symbol::visit(VisitController *v) const {v->visit_pred_symbol(this);};
void func_symbol::visit(VisitController *v) const {v->visit_func_symbol(this);};
void const_symbol::visit(VisitController *v) const {v->visit_const_symbol(this);};
void var_symbol::visit(VisitController *v) const {v->visit_var_symbol(this);};
void pddl_typed_symbol::visit(VisitController *v) const {v->visit_pddl_typed_symbol(this);};
void plus_expression::visit(VisitController *v) const {v->visit_plus_expression(this);};
void minus_expression::visit(VisitController *v) const {v->visit_minus_expression(this);};
void mul_expression::visit(VisitController *v) const {v->visit_mul_expression(this);};
void div_expression::visit(VisitController *v) const {v->visit_div_expression(this);};
void uminus_expression::visit(VisitController *v) const {v->visit_uminus_expression(this);};
void int_expression::visit(VisitController *v) const {v->visit_int_expression(this);};
void float_expression::visit(VisitController *v) const {v->visit_float_expression(this);};
void special_val_expr::visit(VisitController *v) const {v->visit_special_val_expr(this);};
void func_term::visit(VisitController *v) const {v->visit_func_term(this);};
void assignment::visit(VisitController *v) const {v->visit_assignment(this);};
void goal::visit(VisitController * v) const {};
void constraint_goal::visit(VisitController *v) const {v->visit_constraint_goal(this);};
void preference::visit(VisitController *v) const {v->visit_preference(this);};
void simple_goal::visit(VisitController *v) const {v->visit_simple_goal(this);};
void qfied_goal::visit(VisitController *v) const {v->visit_qfied_goal(this);};
void conj_goal::visit(VisitController *v) const {v->visit_conj_goal(this);};
void disj_goal::visit(VisitController *v) const {v->visit_disj_goal(this);};
void timed_goal::visit(VisitController *v) const {v->visit_timed_goal(this);};
void imply_goal::visit(VisitController *v) const {v->visit_imply_goal(this);};
void neg_goal::visit(VisitController *v) const {v->visit_neg_goal(this);};
void comparison::visit(VisitController *v) const {v->visit_comparison(this);};
void proposition::visit(VisitController *v) const {v->visit_proposition(this);};
void pred_decl::visit(VisitController *v) const {v->visit_pred_decl(this);};
void func_decl::visit(VisitController *v) const {v->visit_func_decl(this);};
void simple_effect::visit(VisitController *v) const {v->visit_simple_effect(this);};
void forall_effect::visit(VisitController *v) const {v->visit_forall_effect(this);};
void cond_effect::visit(VisitController *v) const {v->visit_cond_effect(this);};
void timed_effect::visit(VisitController *v) const {v->visit_timed_effect(this);};
void timed_initial_literal::visit(VisitController *v) const {v->visit_timed_initial_literal(this);};
void effect_lists::visit(VisitController *v) const {v->visit_effect_lists(this);};
void derivation_rule::visit(VisitController * v) const {v->visit_derivation_rule(this);};
void operator_::visit(VisitController *v) const {v->visit_operator_(this);};
void action::visit(VisitController *v) const {v->visit_action(this);};
void event::visit(VisitController *v) const {v->visit_event(this);};
void process::visit(VisitController *v) const {v->visit_process(this);};
void durative_action::visit(VisitController *v) const {v->visit_durative_action(this);};
void domain::visit(VisitController *v) const {v->visit_domain(this);};
void metric_spec::visit(VisitController *v) const {v->visit_metric_spec(this);};
void length_spec::visit(VisitController *v) const {v->visit_length_spec(this);};
void problem::visit(VisitController *v) const {v->visit_problem(this);};
void plan_step::visit(VisitController *v) const {v->visit_plan_step(this);};
void violation_term::visit(VisitController * v) const {v->visit_violation_term(this);};
void observation::visit(VisitController * v) const {v->visit_observation(this);};
void recipient::visit(VisitController * v) const {v->visit_recipient(this);};
void gain_def::visit(VisitController * v) const {v->visit_gain(this);};
};

