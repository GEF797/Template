// Copyright Sami Kangasmaa, 2022


#include "QuestMemory.h"

// ---------------------------------------------- //

int32 FQuestMemoryBlock::GetInteger(const FName& Variable) const
{
	int32 Val = 0;
	if (const int32* Value = IntegerVariables.Find(Variable))
	{
		Val = *Value;
	}
	return IntegerValueOperator.Operate(Variable, Val);
}

void FQuestMemoryBlock::SetInteger(const FName& Variable, int32 Value)
{
	MakeReplicationDirty();
	int32& Ref = IntegerVariables.FindOrAdd(Variable);
	Ref = IntegerValueOperator.Operate(Variable, Value);
}

float FQuestMemoryBlock::GetFloat(const FName& Variable) const
{
	float Val = 0.0f;
	if (const float* Value = FloatVariables.Find(Variable))
	{
		Val = *Value;
	}
	return FloatValueOperator.Operate(Variable, Val);
}

void FQuestMemoryBlock::SetFloat(const FName& Variable, float Value)
{
	MakeReplicationDirty();
	float& Ref = FloatVariables.FindOrAdd(Variable);
	Ref = FloatValueOperator.Operate(Variable, Value);
}

bool FQuestMemoryBlock::GetBool(const FName& Variable) const
{
	if (const bool* Value = BoolVariables.Find(Variable))
	{
		return *Value;
	}
	return false;
}

void FQuestMemoryBlock::SetBool(const FName& Variable, bool Value)
{
	MakeReplicationDirty();
	bool& Ref = BoolVariables.FindOrAdd(Variable);
	Ref = Value;
}

// ---------------------------------------------- //

void FQuestMemoryBlockReplicator::ReplicateMemoryBlock(const FQuestMemoryBlock& MemoryBlock)
{
	ReplicateFromValueMap<FQuestRepVar_Int, int32>(MemoryBlock.IntegerVariables, Integers, DoNotRep_Integer);
	ReplicateFromValueMap<FQuestRepVar_Float, float>(MemoryBlock.FloatVariables, Floats, DoNotRep_Float);
	ReplicateFromValueMap<FQuestRepVar_Bool, bool>(MemoryBlock.BoolVariables, Bools, DoNotRep_Bool);
}

void FQuestMemoryBlockReplicator::ReadToMemoryBlock(FQuestMemoryBlock& MemoryBlock) const
{
	ReplicateToValueMap<FQuestRepVar_Int, int32>(MemoryBlock.IntegerVariables, Integers, DoNotRep_Integer);
	ReplicateToValueMap<FQuestRepVar_Float, float>(MemoryBlock.FloatVariables, Floats, DoNotRep_Float);
	ReplicateToValueMap<FQuestRepVar_Bool, bool>(MemoryBlock.BoolVariables, Bools, DoNotRep_Bool);
}

void FQuestMemoryBlockReplicator::SetVariableReplicationFlags(const TArray<FDefaultQuestVariable>& DefaultVariables)
{
	DoNotRep_Integer.Empty();
	DoNotRep_Float.Empty();
	DoNotRep_Bool.Empty();

	for (const FDefaultQuestVariable& Variable : DefaultVariables)
	{
		if (!Variable.bReplicates)
		{
			if (Variable.Initializer.Variable.Type == EQuestVariableType::Integer)
			{
				DoNotRep_Integer.Add(Variable.Initializer.Variable.Variable);
			}
			else if (Variable.Initializer.Variable.Type == EQuestVariableType::Float)
			{
				DoNotRep_Float.Add(Variable.Initializer.Variable.Variable);
			}
			else if (Variable.Initializer.Variable.Type == EQuestVariableType::Bool)
			{
				DoNotRep_Bool.Add(Variable.Initializer.Variable.Variable);
			}
		}
	}
}

// ---------------------------------------------- //

bool FQuestVariableComparison::Compare(const FQuestMemoryBlock& MemoryBlock) const
{
	if (ComparisonType == EQuestVariableComparisonType::Constant)
	{
		if (Variable.Type == EQuestVariableType::Integer)
		{
			return CompareValues<int32, int32>(MemoryBlock.GetInteger(Variable.Variable), OtherInteger, Operator);
		}
		else if (Variable.Type == EQuestVariableType::Float)
		{
			return CompareValues<float, float>(MemoryBlock.GetFloat(Variable.Variable), OtherFloat, Operator);
		}
		else // bool
		{
			return CompareValues<bool, bool>(MemoryBlock.GetBool(Variable.Variable), bOtherBool, Operator);
		}
	}
	else if (ComparisonType == EQuestVariableComparisonType::OtherVariable)
	{
		if (Variable.Type == EQuestVariableType::Integer)
		{
			if (OtherVariable.Type == EQuestVariableType::Integer)
			{
				return CompareValues<int32, int32>(MemoryBlock.GetInteger(Variable.Variable), MemoryBlock.GetInteger(OtherVariable.Variable), Operator);
			}
			else if (OtherVariable.Type == EQuestVariableType::Float)
			{
				return CompareValues<int32, float>(MemoryBlock.GetInteger(Variable.Variable), MemoryBlock.GetFloat(OtherVariable.Variable), Operator);
			}
			else if (OtherVariable.Type == EQuestVariableType::Bool)
			{
				return CompareValues<int32, int32>(MemoryBlock.GetInteger(Variable.Variable), MemoryBlock.GetBoolAsInt(OtherVariable.Variable), Operator);
			}
		}
		else if (Variable.Type == EQuestVariableType::Float)
		{
			if (OtherVariable.Type == EQuestVariableType::Integer)
			{
				return CompareValues<float, int32>(MemoryBlock.GetFloat(Variable.Variable), MemoryBlock.GetInteger(OtherVariable.Variable), Operator);
			}
			else if (OtherVariable.Type == EQuestVariableType::Float)
			{
				return CompareValues<float, float>(MemoryBlock.GetFloat(Variable.Variable), MemoryBlock.GetFloat(OtherVariable.Variable), Operator);
			}
			else if (OtherVariable.Type == EQuestVariableType::Bool)
			{
				return CompareValues<float, int32>(MemoryBlock.GetFloat(Variable.Variable), MemoryBlock.GetBoolAsInt(OtherVariable.Variable), Operator);
			}
		}
		else // bool
		{
			if (OtherVariable.Type == EQuestVariableType::Integer)
			{
				return CompareValues<int32, int32>(MemoryBlock.GetBoolAsInt(Variable.Variable), MemoryBlock.GetInteger(OtherVariable.Variable), Operator);
			}
			else if (OtherVariable.Type == EQuestVariableType::Float)
			{
				return CompareValues<int32, float>(MemoryBlock.GetBoolAsInt(Variable.Variable), MemoryBlock.GetFloat(OtherVariable.Variable), Operator);
			}
			else if (OtherVariable.Type == EQuestVariableType::Bool)
			{
				return CompareValues<bool, bool>(MemoryBlock.GetBool(Variable.Variable), MemoryBlock.GetBool(OtherVariable.Variable), Operator);
			}
		}
	}

	return false;
}

bool FQuestMemoryQuery::RunQuery(const FQuestMemoryBlock& MemoryBlock) const
{
	if (Operator == EQuestLogicalOperator::Or)
	{
		for (const FQuestVariableComparison& Comparison : Comparisons)
		{
			if (Comparison.Compare(MemoryBlock)) // One comparison true -> or should return true
			{
				return bInvertResult ? false : true;
			}
		}
		return bInvertResult ? false : true;
	}
	else // and
	{
		bool bResult = true;
		for (const FQuestVariableComparison& Comparison : Comparisons)
		{
			if (!Comparison.Compare(MemoryBlock)) // Single returned false -> result is false
			{
				bResult = false;
				break;
			}
		}
		return bInvertResult ? !bResult : bResult;
	}

	//checkNoEntry();
	//return false;
}

// ---------------------------------------------- //

UQuestMemory::UQuestMemory()
{
	MemoryBlock.Reset();
}

void UQuestMemory::InitializeDefaultVariables(const TArray<FDefaultQuestVariable>& Initializers)
{
	for (const FDefaultQuestVariable& DefaultVariableRef : Initializers)
	{
		DefaultVariableRef.Initializer.SetValue(MemoryBlock);
	}
}

bool UQuestMemory::InitializeValueBounds(const TArray<FQuestVariableBounds>& ValueBounds)
{
	bool bOk = true;

	TSet<FName> AddedIntVars;
	TSet<FName> AddedFloatVars;
	AddedIntVars.Reserve(ValueBounds.Num());
	AddedFloatVars.Reserve(ValueBounds.Num());

	TQuestMemoryClampOperator<int32>* IntegerValueOperator = nullptr;
	TQuestMemoryClampOperator<float>* FloatValueOperator = nullptr;
	MemoryBlock.GetMemoryClampOperators(IntegerValueOperator, FloatValueOperator);

	if (!ensure(IntegerValueOperator && FloatValueOperator))
	{
		return false;
	}

	IntegerValueOperator->Reset();
	FloatValueOperator->Reset();

	for (const FQuestVariableBounds& Bound : ValueBounds)
	{
		if (Bound.Variable.Type == EQuestVariableType::Integer)
		{
			if (AddedIntVars.Contains(Bound.Variable.Variable))
			{
				bOk = false;
				continue;
			}
			AddedIntVars.Add(Bound.Variable.Variable);
			IntegerValueOperator->Add(Bound.Variable.Variable, Bound.MinInteger, Bound.MaxInteger, Bound.ClampOperation);
		}
		else if (Bound.Variable.Type == EQuestVariableType::Float)
		{
			if (AddedFloatVars.Contains(Bound.Variable.Variable))
			{
				bOk = false;
				continue;
			}
			AddedFloatVars.Add(Bound.Variable.Variable);
			FloatValueOperator->Add(Bound.Variable.Variable, Bound.MinFloat, Bound.MaxFloat, Bound.ClampOperation);
		}
	}

	return bOk;
}

int32 UQuestMemory::GetInteger(FName Variable) const
{
	return MemoryBlock.GetInteger(Variable);
}

void UQuestMemory::SetInteger(FName Variable, int32 Value)
{
	MemoryBlock.SetInteger(Variable, Value);
}

float UQuestMemory::GetFloat(FName Variable) const
{
	return MemoryBlock.GetFloat(Variable);
}

void UQuestMemory::SetFloat(FName Variable, float Value)
{
	MemoryBlock.SetFloat(Variable, Value);
}

bool UQuestMemory::GetBool(FName Variable) const
{
	return MemoryBlock.GetBool(Variable);
}

void UQuestMemory::SetBool(FName Variable, bool Value)
{
	MemoryBlock.SetBool(Variable, Value);
}

bool UQuestMemory::RunQuery(const FQuestMemoryQuery& Query) const
{
	return Query.RunQuery(MemoryBlock);
}