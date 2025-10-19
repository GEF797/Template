// Copyright, Sami Kangasmaa 2022


#include "QuestUtilityLibrary.h"
#include "QuestFrameworkModule.h"
#include "QuestManager.h"
#include "Components/QuestComponent.h"
#include "Components/QuestTrackerComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include "ViewModels/QuestObjectiveViewModel.h"
#include "ViewModels/QuestContextViewModel.h"
#include "ViewModels/QuestTrackerViewModel.h"
#include "ViewModels/QuestListViewModel.h"

UQuestComponent* UQuestUtilityLibrary::GetQuestComponentFromActor(AActor* Actor, bool bLookForComponent)
{
	UQuestComponent* FoundComponent = nullptr;
	if (Actor)
	{
		// Try first via interface
		if (IQuestComponentInterface* ComponentInterface = Cast<IQuestComponentInterface>(Actor))
		{
			FoundComponent = ComponentInterface->GetQuestComponent();
		}

		// Look for the component
		if (!FoundComponent && bLookForComponent)
		{
			FoundComponent = Actor->FindComponentByClass<UQuestComponent>();
		}
	}
	return FoundComponent;
}

UQuestComponent* UQuestUtilityLibrary::FindRelevantQuestComponentFromActor(AActor* Actor)
{
	UQuestComponent* FoundComponent = nullptr;
	if (Actor)
	{
		FoundComponent = GetQuestComponentFromActor(Actor);

		// Search from pawn
		if (!FoundComponent)
		{
			if (AController* Controller = Cast<AController>(Actor))
			{
				FoundComponent = GetQuestComponentFromActor(Controller->GetPawn());
			}
		}

		// Search from controller
		if (!FoundComponent)
		{
			if (APawn* Pawn = Cast<APawn>(Actor))
			{
				FoundComponent = GetQuestComponentFromActor(Pawn->GetController());
			}
		}
	}
	return FoundComponent;
}

UQuestTrackerComponent* UQuestUtilityLibrary::GetQuestTrackerComponentFromActor(AActor* Actor, bool bLookForComponent)
{
	UQuestTrackerComponent* FoundComponent = nullptr;
	if (Actor)
	{
		// Try first via interface
		if (IQuestComponentInterface* ComponentInterface = Cast<IQuestComponentInterface>(Actor))
		{
			FoundComponent = ComponentInterface->GetQuestTrackerComponent();
		}

		// Look for the component
		if (!FoundComponent && bLookForComponent)
		{
			FoundComponent = Actor->FindComponentByClass<UQuestTrackerComponent>();
		}
	}
	return FoundComponent;
}

UQuestComponent* UQuestUtilityLibrary::GetGameModeQuestComponent(const UObject* WorldContext)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		return GetQuestComponentFromActor(World->GetAuthGameMode(), true);
	}
	return nullptr;
}

UQuestComponent* UQuestUtilityLibrary::GetGameStateQuestComponent(const UObject* WorldContext)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		return GetQuestComponentFromActor(World->GetGameState(), true);
	}
	return nullptr;
}

UQuestComponent* UQuestUtilityLibrary::GetPlayerQuestComponent(APlayerController* PlayerController)
{
	UQuestComponent* FoundComponent = nullptr;
	if (PlayerController)
	{
		// Get from controller
		FoundComponent = GetQuestComponentFromActor(PlayerController, true);

		// Look for player state
		if (!FoundComponent)
		{
			FoundComponent = GetQuestComponentFromActor(PlayerController->GetPlayerState<APlayerState>());
		}

		// Look for pawn
		if (!FoundComponent)
		{
			FoundComponent = GetQuestComponentFromActor(PlayerController->GetPawn());
		}
	}
	return FoundComponent;
}

UQuestTrackerComponent* UQuestUtilityLibrary::GetPlayerQuestTrackerComponent(APlayerController* PlayerController)
{
	UQuestTrackerComponent* FoundComponent = nullptr;
	if (PlayerController)
	{
		// Get from controller
		FoundComponent = GetQuestTrackerComponentFromActor(PlayerController, true);

		// Look for player state
		if (!FoundComponent)
		{
			FoundComponent = GetQuestTrackerComponentFromActor(PlayerController->GetPlayerState<APlayerState>());
		}

		// Look for pawn
		if (!FoundComponent)
		{
			FoundComponent = GetQuestTrackerComponentFromActor(PlayerController->GetPawn());
		}
	}
	return FoundComponent;
}

UQuestTrackerViewModel* UQuestUtilityLibrary::GetPlayerQuestTrackerViewModel(APlayerController* PlayerController)
{
	if (UQuestTrackerComponent* Tracker = GetPlayerQuestTrackerComponent(PlayerController))
	{
		return Tracker->GetViewModel();
	}
	return nullptr;
}

UQuestListViewModel* UQuestUtilityLibrary::GetPlayerQuestListViewModel(APlayerController* PlayerController)
{
	if (UQuestTrackerComponent* Tracker = GetPlayerQuestTrackerComponent(PlayerController))
	{
		return Tracker->GetListViewModel();
	}
	return nullptr;
}

UQuestComponent* UQuestUtilityLibrary::GetSharedQuestComponent(const UObject* WorldContext)
{
	UQuestComponent* FoundComponent = GetGameStateQuestComponent(WorldContext);
	if (!FoundComponent)
	{
		FoundComponent = GetGameModeQuestComponent(WorldContext);
	}
	return FoundComponent;
}

void UQuestUtilityLibrary::BroadcastQuestMessage(const UObject* WorldContext, const FGameplayTag MessageTag, FQuestEventPayload Payload, AActor* Sender)
{
	if (!MessageTag.IsValid())
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Tried to broadcast quest message without a tag!"));
		return;
	}

	if (UQuestManager* Manager = WorldContext->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		return Manager->ReceiveMessageFromListener(MessageTag, Payload, nullptr, Sender);
	}
}

void UQuestUtilityLibrary::SendQuestMessage(const UObject* WorldContext, const FGameplayTag MessageTag, FQuestEventPayload Payload, AActor* Recipient, AActor* Sender)
{
	if (!MessageTag.IsValid())
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Tried to send quest message without a tag!"));
		return;
	}

	if (IsValid(Recipient))
	{
		/*if (UQuestManager* Manager = UQuestManager::GetQuestManager(WorldContext))*/
		if (UQuestManager* Manager = WorldContext->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
		{
			return Manager->ReceiveMessageFromListener(MessageTag, Payload, Recipient, Sender);
		}
	}
}

int32 UQuestUtilityLibrary::GetQuestVariableValueAsInteger(UQuestContext* Context, const FQuestVariable& Variable)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);
		return Variable.GetFromMemoryBlock<int32>(Memory->GetMemoryBlock());
	}
	return 0;
}

float UQuestUtilityLibrary::GetQuestVariableValueAsFloat(UQuestContext* Context, const FQuestVariable& Variable)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);
		return Variable.GetFromMemoryBlock<float>(Memory->GetMemoryBlock());
	}
	return 0.0f;
}

bool UQuestUtilityLibrary::GetQuestVariableValueAsBool(UQuestContext* Context, const FQuestVariable& Variable)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);
		return Variable.GetFromMemoryBlock<bool>(Memory->GetMemoryBlock());
	}
	return false;
}

void UQuestUtilityLibrary::SetQuestVariableAsInteger(UQuestContext* Context, const FQuestVariable& Variable, int32 NewValue)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);
		return Variable.SetInMemoryBlock<int32>(Memory->GetMemoryBlock(), NewValue);
	}
}

void UQuestUtilityLibrary::SetQuestVariableAsFloat(UQuestContext* Context, const FQuestVariable& Variable, float NewValue)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);
		return Variable.SetInMemoryBlock<float>(Memory->GetMemoryBlock(), NewValue);
	}
}

void UQuestUtilityLibrary::SetQuestVariableAsBool(UQuestContext* Context, const FQuestVariable& Variable, bool NewValue)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);
		return Variable.SetInMemoryBlock<bool>(Memory->GetMemoryBlock(), NewValue);
	}
}

FText UQuestUtilityLibrary::FormatQuestText(const FQuestText& Text, UQuestContext* Context)
{
	return Text.GetFormattedText(Context);
}

FLinearColor UQuestUtilityLibrary::ConvertQuestResultToColor(EQuestResult InResult, FLinearColor NotStarted, FLinearColor InProgress, FLinearColor Completed, FLinearColor Failed)
{
	FLinearColor Result = FLinearColor();
	ConvertQuestResultToType<FLinearColor>(InResult, NotStarted, InProgress, Completed, Failed, Result);
	return Result;
}

bool UQuestUtilityLibrary::ConvertQuestResultToBool(EQuestResult InResult, bool NotStarted, bool InProgress, bool Completed, bool Failed)
{
	bool Result = false;
	ConvertQuestResultToType<bool>(InResult, NotStarted, InProgress, Completed, Failed, Result);
	return Result;
}

FText UQuestUtilityLibrary::ConvertQuestResultToText(EQuestResult InResult, FText NotStarted, FText InProgress, FText Completed, FText Failed)
{
	FText Result;
	ConvertQuestResultToType<FText>(InResult, NotStarted, InProgress, Completed, Failed, Result);
	return Result;
}

void UQuestUtilityLibrary::FilterChangedObjectiveViewModels(const TArray<UQuestObjectiveViewModel*>& OldList, const TArray<UQuestObjectiveViewModel*>& NewList, TArray<UQuestObjectiveViewModel*>& OutAdded, TArray<UQuestObjectiveViewModel*>& OutRemoved)
{
	FilterAddedRemoved<UQuestObjectiveViewModel*>(OldList, NewList, OutAdded, OutRemoved);
}

void UQuestUtilityLibrary::FilterChangedQuestContextViewModels(const TArray<UQuestContextViewModel*>& OldList, const TArray<UQuestContextViewModel*>& NewList, TArray<UQuestContextViewModel*>& OutAdded, TArray<UQuestContextViewModel*>& OutRemoved)
{
	FilterAddedRemoved<UQuestContextViewModel*>(OldList, NewList, OutAdded, OutRemoved);
}