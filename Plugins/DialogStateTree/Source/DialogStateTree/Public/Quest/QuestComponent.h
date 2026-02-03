// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "QuestComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged,FName,QuestName,int32, State);


/** A single entry in an inventory */
USTRUCT(Blueprintable,BlueprintType)
struct FQuestEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FQuestEntry()
	{}

	//FString GetDebugString() const;

public:
	friend FQuestList;
	friend UQuestComponent;

	UPROPERTY(BlueprintReadWrite)
	FName QuestName;

	UPROPERTY(BlueprintReadWrite)
	int32 State = 0;

};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FQuestList : public FFastArraySerializer
{
	GENERATED_BODY()

	FQuestList()
		: OwnerComponent(nullptr)
	{
	}

	FQuestList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

	TArray<FName> GetAllItems() const;

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FQuestEntry, FQuestList>(Entries, DeltaParms, *this);
	}

	
	void AddEntry(FName QuestName, int32 State=0);

	void ChangeEntryState(FName QuestName,int32 State);

	int32 GetQuestState(FName QuestName);

	void RemoveEntry(FName QuestName);



private:
	friend UQuestComponent;

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FQuestEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FQuestList> : public TStructOpsTypeTraitsBase2<FQuestList>
{
	enum { WithNetDeltaSerializer = true };
};






UCLASS(Blueprintable,BlueprintType, meta=(BlueprintSpawnableComponent) )
class DIALOGSTATETREE_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Quest)
	void AddQuest(FName QuestName,int32 State=0);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Quest)
	void RemoveQuest(FName QuestName);


	UFUNCTION(BlueprintCallable, Category = Quest, BlueprintPure = false)
	void ChangeQuestState(FName QuestName, int32 State);


	UFUNCTION(BlueprintCallable, Category = Quest, BlueprintPure = false)
	int32 GetQuestState(FName QuestName);

	UFUNCTION(BlueprintCallable, Category = Quest, BlueprintPure = false)
	TArray<FName> GetAllQuests() const;

	UPROPERTY(BlueprintAssignable)
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnQuestStateChanged OnQuestAdd;

	UPROPERTY(BlueprintAssignable)
	FOnQuestStateChanged OnQuestRemove;
private:
	UPROPERTY(Replicated)
	FQuestList QuestList;
		
};
