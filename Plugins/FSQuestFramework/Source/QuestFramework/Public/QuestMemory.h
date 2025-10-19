// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestObjectBase.h"
#include "QuestData.h"
#include "QuestMemory.generated.h"

/**
 * Type of a quest variable by enum
 */
UENUM(BlueprintType)
enum class EQuestVariableType : uint8
{
	Integer = 0,
	Float,
	Bool
};

/**
* Determines comparison type of quest variable
*/
UENUM(BlueprintType)
enum class EQuestVariableComparisonType : uint8
{
	// Compared against constant value
	Constant = 0,

	// Compared against other quest variable
	OtherVariable,
};

/**
* Determines clamping method of quest variable
*/
UENUM(BlueprintType)
enum class EQuestValueClamp : uint8
{
	// Value can be anything
	NotClamped = 0,

	// Value can't be lower than set value
	ClampMin,

	// Value can't be greater than set value
	ClampMax,

	// Both minimum and maximum clamp
	ClampMinAndMax,
};

/**
 * Template to clamp values in quest memory block
 */
template<typename TDataType>
struct TQuestMemoryClampOperator
{
private:
	struct MemoryBoundMinMax
	{
		TDataType BMin;
		TDataType BMax;
		EQuestValueClamp ClampOp;
		MemoryBoundMinMax() {}
		MemoryBoundMinMax(TDataType InBMin, TDataType InBMax, EQuestValueClamp InClampOp)
		{
			BMin = InBMin;
			BMax = InBMax;
			ClampOp = InClampOp;
		}
	};

	TMap<FName, MemoryBoundMinMax, TInlineSetAllocator<32>, TDefaultMapHashableKeyFuncs<FName, MemoryBoundMinMax, false>> ClampTable;

public:

	void Add(const FName& Name, TDataType InMin, TDataType InMax, EQuestValueClamp InClampOp)
	{
		ClampTable.Add(Name, MemoryBoundMinMax(InMin, InMax, InClampOp));
	}

	void Remove(const FName& Name)
	{
		ClampTable.Remove(Name);
	}

	void Reset()
	{
		ClampTable.Empty();
	}

	TDataType Operate(const FName& Name, TDataType Value) const
	{
		const MemoryBoundMinMax* MinMax = ClampTable.Find(Name);
		if (MinMax)
		{
			const TDataType MinV = MinMax->BMin;
			const TDataType MaxV = MinMax->BMax;

			const EQuestValueClamp Op = MinMax->ClampOp;
			switch (Op)
			{
			case EQuestValueClamp::NotClamped:
				return Value;
				break;
			case EQuestValueClamp::ClampMin:
				return FMath::Max<TDataType>(Value, MinV);
				break;
			case EQuestValueClamp::ClampMax:
				return FMath::Min<TDataType>(Value, MaxV);
				break;
			case EQuestValueClamp::ClampMinAndMax:
				return FMath::Clamp<TDataType>(Value, MinV, MaxV);
				break;
			default:
				return Value;
				break;
			}
		}
		return Value;
	}
};

/**
 * Data structure to store quest variables by type
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestMemoryBlock
{
	GENERATED_BODY()

private:

	friend struct FQuestMemoryBlockReplicator;

	UPROPERTY()
	TMap<FName, int32> IntegerVariables;

	UPROPERTY()
	TMap<FName, float> FloatVariables;

	UPROPERTY()
	TMap<FName, bool> BoolVariables;

	// Bound rules for values (non-serialized)
	TQuestMemoryClampOperator<int32> IntegerValueOperator;
	TQuestMemoryClampOperator<float> FloatValueOperator;

	// Set to true when the block is altered any how by calling setters to know if the memory should be replicated
	bool bIsNetworkDirty = true;

public:

	FQuestMemoryBlock() {}
	FQuestMemoryBlock(const FQuestMemoryBlock& Other)
	{
		IntegerVariables = Other.IntegerVariables;
		FloatVariables = Other.FloatVariables;
		BoolVariables = Other.BoolVariables;
	}

	void Reset()
	{
		IntegerVariables.Empty();
		FloatVariables.Empty();
		BoolVariables.Empty();
		IntegerValueOperator.Reset();
		FloatValueOperator.Reset();
	}

	void SetMemoryClampOperators(const TQuestMemoryClampOperator<int32>& IntOp, const TQuestMemoryClampOperator<float>& FloatOp)
	{
		IntegerValueOperator = IntOp;
		FloatValueOperator = FloatOp;
	}

	void GetMemoryClampOperators(TQuestMemoryClampOperator<int32>*& IntOp, TQuestMemoryClampOperator<float>*& FloatOp)
	{
		IntOp = &IntegerValueOperator;
		FloatOp = &FloatValueOperator;
	}

	void MakeReplicationDirty() { bIsNetworkDirty = true; }

	bool PollNetworkReplication() 
	{
		const bool bRep = bIsNetworkDirty;
		bIsNetworkDirty = false;
		return bRep;
	}

	int32 GetInteger(const FName& Variable) const;
	void SetInteger(const FName& Variable, int32 Value);
	float GetFloat(const FName& Variable) const;
	void SetFloat(const FName& Variable, float Value);
	bool GetBool(const FName& Variable) const;
	int32 GetBoolAsInt(const FName& Variable) const { return GetBool(Variable) ? 1 : 0; }
	void SetBool(const FName& Variable, bool Value);
};

USTRUCT()
struct FQuestRepVar_Int
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FName Name;

	UPROPERTY()
	int32 Value;

	FQuestRepVar_Int() 
	{
		Name = NAME_None;
		Value = 0;
	}
	FQuestRepVar_Int(const FName& InName, int32 InValue)
	{
		Name = InName;
		Value = InValue;
	}

	void Reset()
	{
		Value = 0;
	}
};

USTRUCT()
struct FQuestRepVar_Float
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FName Name;

	UPROPERTY()
	float Value;

	FQuestRepVar_Float() 
	{
		Name = NAME_None;
		Value = 0;
	}
	FQuestRepVar_Float(const FName& InName, float InValue)
	{
		Name = InName;
		Value = InValue;
	}

	void Reset()
	{
		Value = 0.0f;
	}
};

USTRUCT()
struct FQuestRepVar_Bool
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FName Name;

	UPROPERTY()
	bool Value;

	FQuestRepVar_Bool() 
	{
		Name = NAME_None;
		Value = false;
	}
	FQuestRepVar_Bool(const FName& InName, bool InValue)
	{
		Name = InName;
		Value = InValue;
	}

	void Reset()
	{
		Value = false;
	}
};

/**
 * Data structure to replicate FQuestMemoryBlock from server to remotes
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestMemoryBlockReplicator
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TArray<FQuestRepVar_Int> Integers;

	UPROPERTY()
	TArray<FQuestRepVar_Float> Floats;

	UPROPERTY()
	TArray<FQuestRepVar_Bool> Bools;

	TSet<FName> DoNotRep_Integer;
	TSet<FName> DoNotRep_Float;
	TSet<FName> DoNotRep_Bool;

	template<typename TVarStructType>
	bool ContainsKeyInArray(const FName& Key, const TArray<TVarStructType>& InArray) const
	{
		for (const TVarStructType& Element : InArray)
		{
			if (Element.Name == Key)
			{
				return true;
			}
		}
		return false;
	}

	template<typename TVarStructType, typename TVarMapValueType>
	void NullifyNonPresentArrayValues(const TMap<FName, TVarMapValueType>& InValueMap, TArray<TVarStructType>& RepValueArray, const TSet<FName>& DoNotRepSet)
	{
		for (TVarStructType& RepValue : RepValueArray)
		{
			if (!InValueMap.Contains(RepValue.Name) && !DoNotRepSet.Contains(RepValue.Name))
			{
				RepValue.Reset();
			}
		}
	}

	template<typename TVarStructType, typename TVarMapValueType>
	void NullifyNonPresentMapValues(TMap<FName, TVarMapValueType>& InValueMap, const TArray<TVarStructType>& RepValueArray, const TSet<FName>& DoNotRepSet) const
	{
		TArray<FName, TInlineAllocator<32>> RemoveArray;
		for (const TPair<FName, TVarMapValueType>& MapValue : InValueMap)
		{
			if (DoNotRepSet.Contains(MapValue.Key))
			{
				continue;
			}

			if (!ContainsKeyInArray<TVarStructType>(MapValue.Key, RepValueArray))
			{
				RemoveArray.Add(MapValue.Key);
			}
		}
		for (const FName& RemoveKey : RemoveArray)
		{
			InValueMap.Remove(RemoveKey);
		}
	}

	template<typename TVarStructType, typename TVarMapValueType>
	void ReplicateFromValueMap(const TMap<FName, TVarMapValueType>& InValueMap, TArray<TVarStructType>& RepValueArray, const TSet<FName>& DoNotRepSet)
	{
		NullifyNonPresentArrayValues<TVarStructType, TVarMapValueType>(InValueMap, RepValueArray, DoNotRepSet);
		for (const TPair<FName, TVarMapValueType>& VarValue : InValueMap)
		{
			// Skip replication if it is not suppose to be replicated
			if (DoNotRepSet.Contains(VarValue.Key))
			{
				continue;
			}

			// Find or add
			bool bAdd = true;
			for (TVarStructType& RepValue : RepValueArray)
			{
				if (RepValue.Name == VarValue.Key)
				{
					RepValue.Value = VarValue.Value;
					bAdd = false;
					break;
				}
			}
			if (bAdd)
			{
				TVarStructType NewEntry;
				NewEntry.Name = VarValue.Key;
				NewEntry.Value = VarValue.Value;
				RepValueArray.Add(NewEntry);
			}
		}
	}

	template<typename TVarStructType, typename TVarMapValueType>
	void ReplicateToValueMap(TMap<FName, TVarMapValueType>& InValueMap, const TArray<TVarStructType>& RepValueArray, const TSet<FName>& DoNotRepSet) const
	{
		NullifyNonPresentMapValues<TVarStructType, TVarMapValueType>(InValueMap, RepValueArray, DoNotRepSet);
		for (const TVarStructType& RepValue : RepValueArray)
		{
			// Don't change the value if this was not suppose to replicate. Should never happen but checking this to be sure.
			if (DoNotRepSet.Contains(RepValue.Name))
			{
				continue;
			}

			TVarMapValueType& ValueRef = InValueMap.FindOrAdd(RepValue.Name);
			ValueRef = RepValue.Value;
		}
	}

public:

	/**
	 * Writes content of memory block to replicator, so server will send the data to the remotes
	 */
	void ReplicateMemoryBlock(const FQuestMemoryBlock& MemoryBlock);

	/**
	 * Reads replicated values to memory block in remote machine
	 */
	void ReadToMemoryBlock(FQuestMemoryBlock& MemoryBlock) const;

	/**
	* Sets special replication flags for variables passed in.
	* Variables without any specific flags are replicated by default.
	*/
	void SetVariableReplicationFlags(const TArray<struct FDefaultQuestVariable>& DefaultVariables);
};

/**
 * Represents quest variable (name and type)
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestVariable
{
	GENERATED_BODY()

public:

	/**
	 * Type of the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	EQuestVariableType Type = EQuestVariableType::Integer;

	/**
	 * Name of the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	FName Variable = NAME_None;

	FQuestVariable() {}
	FQuestVariable(EQuestVariableType InType, const FName& InName)
	{
		Type = InType;
		Variable = InName;
	}

public:

	template<typename TValueType>
	TValueType GetFromMemoryBlock(const FQuestMemoryBlock& Block) const
	{
		if (Type == EQuestVariableType::Integer)
		{
			return (TValueType)(Block.GetInteger(Variable));
		}
		else if (Type == EQuestVariableType::Float)
		{
			return (TValueType)(Block.GetFloat(Variable));
		}
		else if (Type == EQuestVariableType::Bool)
		{
			return (TValueType)(Block.GetBool(Variable));
		}
		checkNoEntry();
		return (TValueType)0;
	}

	template<typename TValueType>
	void SetInMemoryBlock(FQuestMemoryBlock& Block, TValueType InValue) const
	{
		if (Type == EQuestVariableType::Integer)
		{
			Block.SetInteger(Variable, (int32)InValue);
		}
		else if (Type == EQuestVariableType::Float)
		{
			Block.SetFloat(Variable, (float)InValue);
		}
		else if (Type == EQuestVariableType::Bool)
		{
			Block.SetBool(Variable, (bool)InValue);
		}
	}
};

/**
 * Defines bounds of quest variable value
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestVariableBounds
{
	GENERATED_BODY()

public:

	/**
	 * Variable which bounds are set
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	FQuestVariable Variable;

	/**
	 * Clamping method to use with the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	EQuestValueClamp ClampOperation = EQuestValueClamp::ClampMinAndMax;

	/**
	 * Minimum value of the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	int32 MinInteger = 0;

	/**
	 * Maximum value of the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	int32 MaxInteger = QUEST_MAX_INT;

	/**
	 * Minimum value of the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	float MinFloat = 0.0f;

	/**
	 * Maximum value of the variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	float MaxFloat = QUEST_MAX_FLT;
};


/**
 * Struct to display in details panel to set variable to given value
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestVariableInitializer
{
	GENERATED_BODY()

public:

	/**
	 * Variable to set
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	FQuestVariable Variable;

	/**
	 * Sets variable to this value
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	int32 IntegerValue = 0;

	/**
	 * Sets variable to this value
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	float FloatValue = 0;

	/**
	 * Sets variable to this value
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	bool BoolValue = false;

	/**
	 * When this is set true, value is added to existing value instead of overriding it
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	bool bAdditive = false;

public:

	/**
	* Sets variable to value
	*/
	void SetValue(FQuestMemoryBlock& MemoryBlock) const
	{
		if (bAdditive)
		{
			switch (Variable.Type)
			{
			case EQuestVariableType::Integer:
				Variable.SetInMemoryBlock<int32>(MemoryBlock, IntegerValue + Variable.GetFromMemoryBlock<int32>(MemoryBlock));
				break;
			case EQuestVariableType::Float:
				Variable.SetInMemoryBlock<float>(MemoryBlock, FloatValue + Variable.GetFromMemoryBlock<float>(MemoryBlock));
				break;
			case EQuestVariableType::Bool:
				Variable.SetInMemoryBlock<bool>(MemoryBlock, !Variable.GetFromMemoryBlock<bool>(MemoryBlock)); // flip
				break;
			default:
				break;
			}
		}
		else
		{
			switch (Variable.Type)
			{
			case EQuestVariableType::Integer:
				Variable.SetInMemoryBlock<int32>(MemoryBlock, IntegerValue);
				break;
			case EQuestVariableType::Float:
				Variable.SetInMemoryBlock<float>(MemoryBlock, FloatValue);
				break;
			case EQuestVariableType::Bool:
				Variable.SetInMemoryBlock<bool>(MemoryBlock, BoolValue);
				break;
			default:
				break;
			}
		}
	}
};

/**
 * Describes definition of default quest variable that can have additional settings for it.
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FDefaultQuestVariable
{
	GENERATED_BODY()

public:

	/*
	* Inner property that has the actual initialization data
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Memory")
	FQuestVariableInitializer Initializer;

	/*
	* Should the variable replicate?
	* This should be toggled off to save network performance if the variable won't change.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Memory")
	bool bReplicates = true;

	/*
	* Description of the variable (editor-only)
	* This is shown as a tooltip in the variable listing tab.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Memory")
	FText Description;

	FDefaultQuestVariable()
	{
		Description = FText::GetEmpty();
	}
};

/**
 * Represents quest variable and additional information of it.
 * Used in quest editor.
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestVariableInfo
{
	GENERATED_BODY()

public:

	// Actual variable name and type.
	FQuestVariable QuestVariable;

	// should the variable replicate?
	bool bReplicates = true;

	// Description of the variable (editor-only)
	FText Description;

	// Default value of the variable (per type)
	int32 DefaultIntegerValue = 0;
	float DefaultFloatValue = 0;
	bool DefaultBoolValue = false;

	// Is this variable default variable or added to quest memory later when the quest progress?
	bool bIsDefaultVariable = false;
};


/**
 * Comparison method against quest variable
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestVariableComparison
{
	GENERATED_BODY()

public:

	/**
	 * Operator to use to do comparison
	 * A [operator] B, where A is quest variable and B is value compared against to
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	EQuestComparisonOperator Operator = EQuestComparisonOperator::Equal;

	/**
	 * Variable to compare (A)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	FQuestVariable Variable;

	/**
	 * Type of compared value (B)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	EQuestVariableComparisonType ComparisonType = EQuestVariableComparisonType::Constant;

public:

	/**
	 * Integer (B) to compare Variable (A)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	int32 OtherInteger = 0;

	/**
	 * Float (B) to compare Variable (A)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	float OtherFloat = 0.0f;

	/**
	 * Boolean (B) to compare Variable (A)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	bool bOtherBool = true;

	/**
	 * When ComparisonType is set to OtherVariable this is B variable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	FQuestVariable OtherVariable;

public:

	/**
	 * Reads value of the variable from given memory block
	 * and runs compare operation for variable and other value set in here
	 */
	bool Compare(const FQuestMemoryBlock& MemoryBlock) const;

	// Helper
	template<typename TA, typename TB>
	static bool CompareValues(TA A, TB B, EQuestComparisonOperator Op)
	{
		if (Op == EQuestComparisonOperator::Equal)
		{
			return A == B;
		}
		else if (Op == EQuestComparisonOperator::NotEqual)
		{
			return A != B;
		}
		else if (Op == EQuestComparisonOperator::Less)
		{
			return A < B;
		}
		else if (Op == EQuestComparisonOperator::Greater)
		{
			return A > B;
		}
		else if (Op == EQuestComparisonOperator::LessEqual)
		{
			return A <= B;
		}
		else if (Op == EQuestComparisonOperator::GreaterEqual)
		{
			return A >= B;
		}
		return false;
	}
};

/**
* Represents rules for query to quest variable memory
*/
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestMemoryQuery
{
	GENERATED_BODY()

public:

	/**
	 * List of comparison operations to do against variables in memory
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	TArray<FQuestVariableComparison> Comparisons;

	/**
	 * Operator to use if there are multiple variable checks
	 * And = All comparisons must match
	 * Or = Any of comparisons must match
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	EQuestLogicalOperator Operator = EQuestLogicalOperator::And;

	/**
	 * Should the result inverted (not boolean for final result)?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Memory")
	bool bInvertResult = false;

public:

	/*
	* Runs this memory query for MemoryBlock
	* Returns true if the query passes, false otherwise
	*/
	bool RunQuery(const FQuestMemoryBlock& MemoryBlock) const;

	/**
	 * Checks if the query has any comparison operators set
	 */
	bool HasComparisonOperators() const { return Comparisons.Num() > 0; }
};

/**
 * Instanced object to store quest related variables
 */
UCLASS(NotBlueprintable, BlueprintType)
class QUESTFRAMEWORK_API UQuestMemory : public UQuestObjectBase
{
	GENERATED_BODY()
	
public:

	UQuestMemory();

	// Initializes the memory with a list of default variables (variables and their initial values)
	void InitializeDefaultVariables(const TArray<FDefaultQuestVariable>& Initializers);

	// Initializes memory block clamp operators from list of variable value bounds
	bool InitializeValueBounds(const TArray<FQuestVariableBounds>& ValueBounds);

	// Returns integer by variable name. Zero if not found.
	UFUNCTION(BlueprintPure, Category = "Quest Memory")
	int32 GetInteger(FName Variable) const;

	// Sets integer variable by name.
	UFUNCTION(BlueprintCallable, Category = "Quest Memory")
	void SetInteger(FName Variable, int32 Value);

	// Returns float by variable name. Zero if not found.
	UFUNCTION(BlueprintPure, Category = "Quest Memory")
	float GetFloat(FName Variable) const;

	// Sets float variable by name.
	UFUNCTION(BlueprintCallable, Category = "Quest Memory")
	void SetFloat(FName Variable, float Value);

	// Returns boolean by variable name. false if not found.
	UFUNCTION(BlueprintPure, Category = "Quest Memory")
	bool GetBool(FName Variable) const;

	// Sets boolean variable by name.
	UFUNCTION(BlueprintCallable, Category = "Quest Memory")
	void SetBool(FName Variable, bool Value);

	/*
	* Runs memory query for this memory object.
	* Returns true if the query passes, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Memory")
	bool RunQuery(const FQuestMemoryQuery& Query) const;

protected:

	UPROPERTY()
	FQuestMemoryBlock MemoryBlock;

public:

	// Getter for whole memory block. Use with caution
	FQuestMemoryBlock& GetMemoryBlock() { return MemoryBlock; }
	const FQuestMemoryBlock& GetMemoryBlock() const { return MemoryBlock; }
};


