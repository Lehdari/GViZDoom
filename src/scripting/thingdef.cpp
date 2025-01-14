/*
** thingdef.cpp
**
** Actor definitions
**
**---------------------------------------------------------------------------
** Copyright 2002-2008 Christoph Oelckers
** Copyright 2004-2008 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 4. When not used as part of ZDoom or a ZDoom derivative, this code will be
**    covered by the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or (at
**    your option) any later version.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "gi.h"
#include "actor.h"
#include "info.h"
#include "sc_man.h"
#include "tarray.h"
#include "w_wad.h"
#include "templates.h"
#include "r_defs.h"
#include "a_pickups.h"
#include "s_sound.h"
#include "cmdlib.h"
#include "p_lnspec.h"
#include "decallib.h"
#include "m_random.h"
#include "i_system.h"
#include "m_argv.h"
#include "p_local.h"
#include "doomerrors.h"
#include "a_weapons.h"
#include "p_conversation.h"
#include "v_text.h"
//#include "thingdef.h"
#include "backend/codegen.h"
#include "a_sharedglobal.h"
#include "backend/vmbuilder.h"
#include "stats.h"
#include "logging.h"

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------
void InitThingdef();

// STATIC FUNCTION PROTOTYPES --------------------------------------------

static TMap<FState *, FScriptPosition> StateSourceLines;
static FScriptPosition unknownstatesource("unknown file", 0);

//==========================================================================
//
// Saves the state's source lines for error messages during postprocessing
//
//==========================================================================

void SaveStateSourceLines(FState *firststate, TArray<FScriptPosition> &positions)
{
	for (unsigned i = 0; i < positions.Size(); i++)
	{
		StateSourceLines[firststate + i] = positions[i];
	}
}

FScriptPosition & GetStateSource(FState *state)
{
	auto check = StateSourceLines.CheckKey(state);
	return check ? *check : unknownstatesource;
}

//==========================================================================
//
// SetImplicitArgs
//
// Adds the parameters implied by the function flags.
//
//==========================================================================

void SetImplicitArgs(TArray<PType *> *args, TArray<uint32_t> *argflags, TArray<FName> *argnames, PContainerType *cls, uint32_t funcflags, int useflags)
{
	// Must be called before adding any other arguments.
	assert(args == nullptr || args->Size() == 0);
	assert(argflags == nullptr || argflags->Size() == 0);

	if (funcflags & VARF_Method)
	{
		// implied self pointer
		if (args != nullptr)		args->Push(NewPointer(cls, !!(funcflags & VARF_ReadOnly))); 
		if (argflags != nullptr)	argflags->Push(VARF_Implicit | VARF_ReadOnly);
		if (argnames != nullptr)	argnames->Push(NAME_self);
	}
	if (funcflags & VARF_Action)
	{
		assert(!(funcflags & VARF_ReadOnly));
		// implied caller and callingstate pointers
		if (args != nullptr)
		{
			// Special treatment for weapons and CustomInventory flagged functions: 'self' is not the defining class but the actual user of the item, so this pointer must be of type 'Actor'
			if (useflags & (SUF_WEAPON|SUF_ITEM))
			{
				args->Insert(0, NewPointer(RUNTIME_CLASS(AActor)));	// this must go in before the real pointer to the containing class.
			}
			else
			{
				args->Push(NewPointer(cls));
			}
			args->Push(NewPointer(NewStruct("FStateParamInfo", nullptr)));
		}
		if (argflags != nullptr)
		{
			argflags->Push(VARF_Implicit | VARF_ReadOnly);
			argflags->Push(VARF_Implicit | VARF_ReadOnly);
		}
		if (argnames != nullptr)
		{
			argnames->Push(NAME_invoker);
			argnames->Push(NAME_stateinfo);
		}
	}
}

//==========================================================================
//
// CreateAnonymousFunction
//
// Creates a function symbol for an anonymous function
// This contains actual info about the implied variables which is needed
// during code generation.
//
//==========================================================================

PFunction *CreateAnonymousFunction(PContainerType *containingclass, PType *returntype, int flags)
{
	TArray<PType *> rets(1);
	TArray<PType *> args;
	TArray<uint32_t> argflags;
	TArray<FName> argnames;

	// Functions that only get flagged for actors do not need the additional two context parameters.
	int fflags = (flags& (SUF_OVERLAY | SUF_WEAPON | SUF_ITEM)) ? VARF_Action | VARF_Method : VARF_Method;

	// [ZZ] give anonymous functions the scope of their class 
	//      (just give them VARF_Play, whatever)
	fflags |= VARF_Play;

	rets[0] = returntype != nullptr? returntype : TypeError;	// Use TypeError as placeholder if we do not know the return type yet.
	SetImplicitArgs(&args, &argflags, &argnames, containingclass, fflags, flags);

	PFunction *sym = Create<PFunction>(containingclass, NAME_None);	// anonymous functions do not have names.
	sym->AddVariant(NewPrototype(rets, args), argflags, argnames, nullptr, fflags, flags);
	return sym;
}

//==========================================================================
//
// FindClassMemberFunction
//
// Looks for a name in a class's symbol table and outputs appropriate messages
//
//==========================================================================

PFunction *FindClassMemberFunction(PContainerType *selfcls, PContainerType *funccls, FName name, FScriptPosition &sc, bool *error)
{
	// Skip ACS_NamedExecuteWithResult. Anything calling this should use the builtin instead.
	if (name == NAME_ACS_NamedExecuteWithResult) return nullptr;

	PSymbolTable *symtable;
	auto symbol = selfcls->Symbols.FindSymbolInTable(name, symtable);
	auto funcsym = dyn_cast<PFunction>(symbol);

	if (symbol != nullptr)
	{
		auto cls_ctx = PType::toClass(funccls);
		auto cls_target = funcsym ? PType::toClass(funcsym->OwningClass) : nullptr;
		if (funcsym == nullptr)
		{
			sc.Message(MSG_ERROR, "%s is not a member function of %s", name.GetChars(), selfcls->TypeName.GetChars());
		}
		else if ((funcsym->Variants[0].Flags & VARF_Private) && symtable != &funccls->Symbols)
		{
			// private access is only allowed if the symbol table belongs to the class in which the current function is being defined.
			sc.Message(MSG_ERROR, "%s is declared private and not accessible", symbol->SymbolName.GetChars());
		}
		else if ((funcsym->Variants[0].Flags & VARF_Protected) && symtable != &funccls->Symbols && (!cls_ctx || !cls_target || !cls_ctx->Descriptor->IsDescendantOf(cls_target->Descriptor)))
		{
			sc.Message(MSG_ERROR, "%s is declared protected and not accessible", symbol->SymbolName.GetChars());
		}
		else if (funcsym->Variants[0].Flags & VARF_Deprecated)
		{
			sc.Message(MSG_WARNING, "Call to deprecated function %s", symbol->SymbolName.GetChars());
		}
	}
	// return nullptr if the name cannot be found in the symbol table so that the calling code can do other checks.
	return funcsym;
}

//==========================================================================
//
// CreateDamageFunction
//
// creates a damage function from the given expression
//
//==========================================================================

void CreateDamageFunction(PNamespace *OutNamespace, const VersionInfo &ver, PClassActor *info, AActor *defaults, FxExpression *id, bool fromDecorate, int lumpnum)
{
	if (id == nullptr)
	{
		defaults->DamageFunc = nullptr;
	}
	else
	{
		auto dmg = new FxReturnStatement(new FxIntCast(id, true), id->ScriptPosition);
		auto funcsym = CreateAnonymousFunction(info->VMType, TypeSInt32, 0);
		defaults->DamageFunc = FunctionBuildList.AddFunction(OutNamespace, ver, funcsym, dmg, FStringf("%s.DamageFunction", info->TypeName.GetChars()), fromDecorate, -1, 0, lumpnum);
	}
}

//==========================================================================
//
// CheckForUnsafeStates
//
// Performs a quick analysis to find potentially bad states.
// This is not perfect because it cannot track jumps by function.
// For such cases a runtime check in the relevant places is also present.
//
//==========================================================================
static void CheckForUnsafeStates(PClassActor *obj)
{
	static ENamedName weaponstates[] = { NAME_Ready, NAME_Deselect, NAME_Select, NAME_Fire, NAME_AltFire, NAME_Hold, NAME_AltHold, NAME_Flash, NAME_AltFlash, NAME_None };
	static ENamedName pickupstates[] = { NAME_Pickup, NAME_Drop, NAME_Use, NAME_None };
	TMap<FState *, bool> checked;
	ENamedName *test;

	if (obj->IsDescendantOf(NAME_Weapon))
	{
		if (obj->Size == RUNTIME_CLASS(AWeapon)->Size) return;	// This class cannot have user variables.
		test = weaponstates;
	}
	else
	{
		auto citype = PClass::FindActor(NAME_CustomInventory);
		if (obj->IsDescendantOf(citype))
		{
			if (obj->Size == citype->Size) return;	// This class cannot have user variables.
			test = pickupstates;
		}
		else return;	// something else derived from AStateProvider. We do not know what this may be.
	}

	for (; *test != NAME_None; test++)
	{
		FState *state = obj->FindState(*test);
		while (state != nullptr && checked.CheckKey(state) == nullptr)	// have we checked this state already. If yes, we can stop checking the current chain.
		{
			checked[state] = true;
			if (state->ActionFunc && state->ActionFunc->Unsafe)
			{
				// If an unsafe function (i.e. one that accesses user variables) is being detected, print a warning once and remove the bogus function. We may not call it because that would inevitably crash.
				GetStateSource(state).Message(MSG_ERROR, TEXTCOLOR_RED "Unsafe state call in state %s which accesses user variables, reached by %s.%s.\n",
					FState::StaticGetStateName(state).GetChars(), obj->TypeName.GetChars(), FName(*test).GetChars());
			}
			state = state->NextState;
		}
	}
}

//==========================================================================
//
// CheckStates
//
// Checks if states link to ones with proper restrictions
// Checks that all base labels refer a string with proper restrictions.
// For these cases a runtime check in the relevant places is also present.
//
//==========================================================================

static void CheckLabel(PClassActor *obj, FStateLabel *slb, int useflag, FName statename, const char *descript)
{
	auto state = slb->State;
	if (state != nullptr)
	{
		if (!(state->UseFlags & useflag))
		{
			GetStateSource(state).Message(MSG_ERROR, TEXTCOLOR_RED "%s references state %s as %s state, but this state is not flagged for use as %s.\n",
				obj->TypeName.GetChars(), FState::StaticGetStateName(state).GetChars(), statename.GetChars(), descript);
		}
	}
	if (slb->Children != nullptr)
	{
		for (int i = 0; i < slb->Children->NumLabels; i++)
		{
			auto state = slb->Children->Labels[i].State;
			CheckLabel(obj, &slb->Children->Labels[i], useflag, statename, descript);
		}
	}
}

static void CheckStateLabels(PClassActor *obj, ENamedName *test, int useflag,  const char *descript)
{
	FStateLabels *labels = obj->GetStateLabels();

	for (; *test != NAME_None; test++)
	{
		auto label = labels->FindLabel(*test);
		if (label != nullptr)
		{
			CheckLabel(obj, label, useflag, *test, descript);
		}
	}
}


static void CheckStates(PClassActor *obj)
{
	static ENamedName actorstates[] = { NAME_Spawn, NAME_See, NAME_Melee, NAME_Missile, NAME_Pain, NAME_Death, NAME_Wound, NAME_Raise, NAME_Yes, NAME_No, NAME_Greetings, NAME_None };
	static ENamedName weaponstates[] = { NAME_Ready, NAME_Deselect, NAME_Select, NAME_Fire, NAME_AltFire, NAME_Hold, NAME_AltHold, NAME_Flash, NAME_AltFlash, NAME_None };
	static ENamedName pickupstates[] = { NAME_Pickup, NAME_Drop, NAME_Use, NAME_None };
	TMap<FState *, bool> checked;

	CheckStateLabels(obj, actorstates, SUF_ACTOR, "actor sprites");

	if (obj->IsDescendantOf(NAME_Weapon))
	{
		CheckStateLabels(obj, weaponstates, SUF_WEAPON, "weapon sprites");
	}
	else if (obj->IsDescendantOf(NAME_CustomInventory))
	{
		CheckStateLabels(obj, pickupstates, SUF_ITEM, "CustomInventory state chain");
	}
	for (unsigned i = 0; i < obj->GetStateCount(); i++)
	{
		auto state = obj->GetStates() + i;
		if (state->NextState && (state->UseFlags & state->NextState->UseFlags) != state->UseFlags)
		{
			GetStateSource(state).Message(MSG_ERROR, TEXTCOLOR_RED "State %s links to a state with incompatible restrictions.\n",
				FState::StaticGetStateName(state).GetChars());
		}
	}
}

//==========================================================================
//
// LoadActors
//
// Called from FActor::StaticInit()
//
//==========================================================================
void ParseScripts();
void ParseAllDecorate();
void SynthesizeFlagFields();

void LoadActors()
{
	cycle_t timer;

	timer.Reset(); timer.Clock();
	FScriptPosition::ResetErrorCounter();

	InitThingdef();
	FScriptPosition::StrictErrors = true;
	ParseScripts();

	FScriptPosition::StrictErrors = false;
	ParseAllDecorate();
	SynthesizeFlagFields();

	FunctionBuildList.Build();

	if (FScriptPosition::ErrorCounter > 0)
	{
		I_Error("%d errors while parsing DECORATE scripts", FScriptPosition::ErrorCounter);
	}
	FScriptPosition::ResetErrorCounter();

	for (int i = PClassActor::AllActorClasses.Size() - 1; i >= 0; i--)
	{
		auto ti = PClassActor::AllActorClasses[i];
		if (ti->Size == TentativeClass)
		{
			if (ti->bOptional)
			{
				Printf(TEXTCOLOR_ORANGE "Class %s referenced but not defined\n", ti->TypeName.GetChars());
				FScriptPosition::WarnCounter++;
				// the class must be rendered harmless so that it won't cause problems.
				ti->ParentClass = RUNTIME_CLASS(AActor);
				ti->Size = sizeof(AActor);
			}
			else
			{
				Printf(TEXTCOLOR_RED "Class %s referenced but not defined\n", ti->TypeName.GetChars());
				FScriptPosition::ErrorCounter++;
			}
			continue;
		}

		if (GetDefaultByType(ti) == nullptr)
		{
			Printf(TEXTCOLOR_RED "No ActorInfo defined for class '%s'\n", ti->TypeName.GetChars());
			FScriptPosition::ErrorCounter++;
			continue;
		}


		CheckStates(ti);

		if (ti->bDecorateClass && ti->IsDescendantOf(RUNTIME_CLASS(AStateProvider)))
		{
			// either a DECORATE based weapon or CustomInventory. 
			// These are subject to relaxed rules for user variables in states.
			// Although there is a runtime check for bogus states, let's do a quick analysis if any of the known entry points
			// hits an unsafe state. If we can find something here it can be handled wuth a compile error rather than a runtime error.
			CheckForUnsafeStates(ti);
		}

		// ensure that all actor bouncers have PASSMOBJ set.
		auto defaults = GetDefaultByType(ti);
		if (defaults->BounceFlags & (BOUNCE_Actors | BOUNCE_AllActors))
		{
			// PASSMOBJ is irrelevant for normal missiles, but not for bouncers.
			defaults->flags2 |= MF2_PASSMOBJ;
		}
	}
	if (FScriptPosition::ErrorCounter > 0)
	{
		I_Error("%d errors during actor postprocessing", FScriptPosition::ErrorCounter);
	}

	timer.Unclock();
    if (!batchrun) doom_logging::print("script parsing took %.2f ms\n", timer.TimeMS());
	
    // Now we may call the scripted OnDestroy method.
	PClass::bVMOperational = true;
	StateSourceLines.Clear();
}
