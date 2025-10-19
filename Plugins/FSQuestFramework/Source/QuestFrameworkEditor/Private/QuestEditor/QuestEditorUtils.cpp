// Copyright, Sami Kangasmaa 2022


#include "QuestEditor/QuestEditorUtils.h"

#include "QuestObjects/QuestCondition.h"
#include "QuestObjects/QuestObjective.h"
#include "QuestObjects/QuestTextArgument.h"

#include "Graph/QuestGraph.h"
#include "Nodes/QuestNode.h"
#include "Nodes/QuestGraphNodeCommonBase.h"


TArray<FName> FQuestEditorUtils::FindQuestVariableNames(UQuest* QuestAsset, EQuestVariableType Type)
{
	if (!QuestAsset)
	{
		return TArray<FName>();
	}

	TSet<FName> Names;
	TSet<const UObject*> Visited;

	// Append all variables in quest asset object
	AppendQuestVariableNames_Object(QuestAsset, Type, Names, Visited, false);

	// Append quest variables in graph nodes and recursively in their children
	for (UEdGraphNode* GraphNode : QuestAsset->GetGraph()->Nodes)
	{
		if (GraphNode)
		{
			AppendQuestVariableNames_Object(GraphNode, Type, Names, Visited, true);
		}
	}

	if (Names.Contains(NAME_None))
	{
		Names.Remove(NAME_None);
	}

	return Names.Array();
}

TArray<FQuestVariableInfo> FQuestEditorUtils::CreateQuestVariableInfoData(UQuest* QuestAsset)
{
	if (!QuestAsset)
	{
		return TArray<FQuestVariableInfo>();
	}

	TArray<FQuestVariableInfo> InfoArray;

	const TArray<FDefaultQuestVariable>& DefaultVariables = QuestAsset->GetDefaultQuestVariables();

	auto AddDefaultVarProperties = [DefaultVariables](const FName& InName, EQuestVariableType InType, FQuestVariableInfo& Info)
	{
		for (const FDefaultQuestVariable& DefaultVar : DefaultVariables)
		{
			if (DefaultVar.Initializer.Variable.Variable == InName && DefaultVar.Initializer.Variable.Type == InType)
			{
				Info.bIsDefaultVariable = true;
				Info.bReplicates = DefaultVar.bReplicates;
				Info.Description = DefaultVar.Description;
				
				switch (DefaultVar.Initializer.Variable.Type)
				{
				case EQuestVariableType::Bool:
					Info.DefaultBoolValue = DefaultVar.Initializer.BoolValue;
					break;
				case EQuestVariableType::Float:
					Info.DefaultFloatValue = DefaultVar.Initializer.FloatValue;
					break;
				case EQuestVariableType::Integer:
				default:
					Info.DefaultIntegerValue = DefaultVar.Initializer.IntegerValue;
					break;
				}
				return;
			}
		}
	};

	TArray<FName> IntegerVariableNames = FindQuestVariableNames(QuestAsset, EQuestVariableType::Integer);
	TArray<FName> FloatVariableNames = FindQuestVariableNames(QuestAsset, EQuestVariableType::Float);
	TArray<FName> BoolVariableNames = FindQuestVariableNames(QuestAsset, EQuestVariableType::Bool);

	for (const FName& VarName : IntegerVariableNames)
	{
		FQuestVariableInfo NewInfo;
		NewInfo.QuestVariable = FQuestVariable(EQuestVariableType::Integer, VarName);
		AddDefaultVarProperties(VarName, EQuestVariableType::Integer, NewInfo);
		InfoArray.Add(NewInfo);
	}

	for (const FName& VarName : FloatVariableNames)
	{
		FQuestVariableInfo NewInfo;
		NewInfo.QuestVariable = FQuestVariable(EQuestVariableType::Float, VarName);
		AddDefaultVarProperties(VarName, EQuestVariableType::Float, NewInfo);
		InfoArray.Add(NewInfo);
	}

	for (const FName& VarName : BoolVariableNames)
	{
		FQuestVariableInfo NewInfo;
		NewInfo.QuestVariable = FQuestVariable(EQuestVariableType::Bool, VarName);
		AddDefaultVarProperties(VarName, EQuestVariableType::Bool, NewInfo);
		InfoArray.Add(NewInfo);
	}

	return InfoArray;
}

void FQuestEditorUtils::AppendQuestVariableNames_Object(const UObject* Container, EQuestVariableType Type, TSet<FName>& Names, TSet<const UObject*>& VisitedObjects, bool bRecursiveObjects)
{
	if (!Container)
	{
		return;
	}

	if (VisitedObjects.Contains(Container))
	{
		return;
	}

	VisitedObjects.Add(Container);

	UClass* ObjectClass = Container->GetClass();
	if (!ObjectClass)
	{
		return;
	}

	for (TFieldIterator<FProperty> Prop(ObjectClass); Prop; ++Prop)
	{
		if (!Prop)
		{
			continue;
		}

		const FProperty* Property = *Prop;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			const void* StructContainer = StructProperty->ContainerPtrToValuePtr<void>(Container);
			if (Prop.GetStruct() == FQuestVariable::StaticStruct())
			{
				FProperty* TypeProperty = Prop.GetStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FQuestVariable, Type));
				FProperty* VariableProperty = Prop.GetStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FQuestVariable, Variable));

				if (TypeProperty && VariableProperty)
				{
					FEnumProperty* EnumTypeProperty = CastField<FEnumProperty>(TypeProperty);
					if (EnumTypeProperty)
					{
						const EQuestVariableType VarType = (EQuestVariableType)(*EnumTypeProperty->ContainerPtrToValuePtr<uint8>(StructContainer));
						if (VarType == Type)
						{
							if (FNameProperty* VariableNameProperty = CastField<FNameProperty>(VariableProperty))
							{
								const FName& VariableName = VariableNameProperty->GetPropertyValue_InContainer(StructContainer);
								Names.Add(VariableName);
							}
						}
					}
				}

				continue;
			}

			AppendQuestVariableNames_Struct(StructContainer, StructProperty, Type, Names, VisitedObjects, bRecursiveObjects);
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			if (bRecursiveObjects)
			{
				const UObject* SubObject = ObjectProperty->GetPropertyValue_InContainer(Container);
				AppendQuestVariableNames_Object(SubObject, Type, Names, VisitedObjects, bRecursiveObjects);
			}
		}
		else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			if (ArrayProperty->Inner && ArrayProperty->Inner->IsA<FObjectProperty>())
			{
				if (bRecursiveObjects)
				{
					TArray<const UObject*> SubObjects = *ArrayProperty->ContainerPtrToValuePtr<TArray<const UObject*>>(Container);
					for (const UObject* SubObject : SubObjects)
					{
						if (SubObject)
						{
							AppendQuestVariableNames_Object(SubObject, Type, Names, VisitedObjects, bRecursiveObjects);
						}
					}
				}
			}
			else if (ArrayProperty->Inner && ArrayProperty->Inner->IsA<FStructProperty>())
			{
				FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner);
				const void* StructArrayContainer = ArrayProperty->ContainerPtrToValuePtr<void>(Container);

				FScriptArrayHelper Helper(ArrayProperty, StructArrayContainer);
				for (int32 Idx = 0; Idx < Helper.Num(); Idx++)
				{
					uint8* StructContainer = Helper.GetRawPtr(Idx);
					AppendQuestVariableNames_Struct(StructContainer, InnerStructProperty, Type, Names, VisitedObjects, bRecursiveObjects);
				}
				
			}
		}
	}
}

void FQuestEditorUtils::AppendQuestVariableNames_Struct(const void* Container, const FStructProperty* InStructProperty, EQuestVariableType Type, TSet<FName>& Names, TSet<const UObject*>& VisitedObjects, bool bRecursiveObjects)
{
	if (!Container || !InStructProperty)
	{
		return;
	}
	
	UStruct* Struct = InStructProperty->Struct;
	if (!Struct)
	{
		return;
	}

	// Quest variable struct
	if (Struct == FQuestVariable::StaticStruct())
	{
		FProperty* TypeProperty = Struct->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FQuestVariable, Type));
		FProperty* VariableProperty = Struct->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FQuestVariable, Variable));

		if (TypeProperty && VariableProperty)
		{
			FEnumProperty* EnumTypeProperty = CastField<FEnumProperty>(TypeProperty);
			if (EnumTypeProperty)
			{
				uint8 EnumValue = *EnumTypeProperty->ContainerPtrToValuePtr<uint8>(Container);
				if ((EQuestVariableType)EnumValue == Type)
				{
					if (FNameProperty* VariableNameProperty = CastField<FNameProperty>(VariableProperty))
					{
						const FName& VariableName = VariableNameProperty->GetPropertyValue_InContainer(Container);
						Names.Add(VariableName);
					}
				}
			}
		}

		return;
	}

	for (TFieldIterator<FProperty> Prop(Struct); Prop; ++Prop)
	{
		if (!Prop)
		{
			continue;
		}

		const FProperty* Property = *Prop;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			const void* StructContainer = StructProperty->ContainerPtrToValuePtr<void>(Container);
			AppendQuestVariableNames_Struct(StructContainer, StructProperty, Type, Names, VisitedObjects, bRecursiveObjects);
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			if (bRecursiveObjects)
			{
				const UObject* SubObject = ObjectProperty->GetPropertyValue_InContainer(Container);
				AppendQuestVariableNames_Object(SubObject, Type, Names, VisitedObjects, bRecursiveObjects);
			}
		}
		else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			if (ArrayProperty->Inner && ArrayProperty->Inner->IsA<FObjectProperty>())
			{
				if (bRecursiveObjects)
				{
					TArray<const UObject*> SubObjects = *ArrayProperty->ContainerPtrToValuePtr<TArray<const UObject*>>(Container);
					for (const UObject* SubObject : SubObjects)
					{
						if (SubObject)
						{
							AppendQuestVariableNames_Object(SubObject, Type, Names, VisitedObjects, bRecursiveObjects);
						}
					}
				}
			}
			else if (ArrayProperty->Inner && ArrayProperty->Inner->IsA<FStructProperty>())
			{
				FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner);
				const void* StructArrayContainer = ArrayProperty->ContainerPtrToValuePtr<void>(Container);

				FScriptArrayHelper Helper(ArrayProperty, StructArrayContainer);
				for (int32 Idx = 0; Idx < Helper.Num(); Idx++)
				{
					uint8* StructContainer = Helper.GetRawPtr(Idx);
					AppendQuestVariableNames_Struct(StructContainer, InnerStructProperty, Type, Names, VisitedObjects, bRecursiveObjects);
				}

			}
		}
	}
}