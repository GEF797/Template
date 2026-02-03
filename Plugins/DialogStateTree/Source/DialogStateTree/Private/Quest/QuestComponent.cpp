// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UQuestComponent::UQuestComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, QuestList(this)
{
	SetIsReplicatedByDefault(true);
	

}

void UQuestComponent::AddQuest(FName QuestName,int32 State)
{
	QuestList.AddEntry(QuestName,State);
	OnQuestAdd.Broadcast(QuestName, State);
}

void UQuestComponent::RemoveQuest(FName QuestName)
{
	QuestList.RemoveEntry(QuestName);
	OnQuestRemove.Broadcast(QuestName, 0);
}

void UQuestComponent::ChangeQuestState(FName QuestName, int32 State)
{
	QuestList.ChangeEntryState(QuestName,State);
	OnQuestStateChanged.Broadcast(QuestName, State);
}

int32 UQuestComponent::GetQuestState(FName QuestName)
{
	return QuestList.GetQuestState(QuestName);
}

TArray<FName> UQuestComponent::GetAllQuests() const
{
	return QuestList.GetAllItems();
}


void UQuestComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, QuestList);
}

TArray<FName> FQuestList::GetAllItems() const
{
	TArray<FName> Results;
	Results.Reserve(Entries.Num());
	for (const FQuestEntry& Entry : Entries)
	{
		
			Results.Add(Entry.QuestName);
		
	}
	return Results;
}

void FQuestList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FQuestEntry& Stack = Entries[Index];
	
		if (UQuestComponent* QuestComp = Cast<UQuestComponent>(OwnerComponent))
		{

			QuestComp->OnQuestRemove.Broadcast(Stack.QuestName, Stack.State);
		}
	}
}

void FQuestList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FQuestEntry& Stack = Entries[Index];
	
		if (UQuestComponent* QuestComp = Cast<UQuestComponent>(OwnerComponent))
		{

			QuestComp->OnQuestAdd.Broadcast(Stack.QuestName, Stack.State);
		}
	}
}

void FQuestList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FQuestEntry& Stack = Entries[Index];
		
		if (UQuestComponent* QuestComp = Cast<UQuestComponent>(OwnerComponent))
		{

			QuestComp->OnQuestStateChanged.Broadcast(Stack.QuestName, Stack.State);
		}
		
	}
}

void FQuestList::AddEntry(FName QuestName, int32 State)
{
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	FQuestEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.QuestName = QuestName;  //@TODO: Using the actor instead of component as the outer due to UE-127172


	NewEntry.State = 0;

	MarkItemDirty(NewEntry);
}

void FQuestList::ChangeEntryState(FName QuestName, int32 State)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FQuestEntry& Entry = *EntryIt;
		if (Entry.QuestName == QuestName)
		{
			Entry.State= State;
			MarkItemDirty(Entry);

			break;
		}

		
	}
}

int32 FQuestList::GetQuestState(FName QuestName)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FQuestEntry& Entry = *EntryIt;
		if (Entry.QuestName == QuestName)
		{
			return Entry.State;
		}


	}

	return -1;
}

void FQuestList::RemoveEntry(FName QuestName)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FQuestEntry& Entry = *EntryIt;
		if (Entry.QuestName == QuestName)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();

			break;
		}

		
	}
}

//FString FQuestEntry::GetDebugString() const
//{
//	return FString::Printf(TEXT("%s (%d)"), QuestName, State);
//}
