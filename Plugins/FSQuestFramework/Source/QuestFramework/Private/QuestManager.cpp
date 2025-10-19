// Copyright Sami Kangasmaa, 2022


#include "QuestManager.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#include "QuestFrameworkModule.h"
#include "QuestManagerAccess.h"
#include "QuestFrameworkSettings.h"

#include "Quest.h"
#include "QuestContext.h"
#include "Components/QuestMessageComponent.h"
#include "QuestUtilityLibrary.h"

bool UQuestManager::bLoadedEditorQuestAssets = false;

void UQuestManager::Tick(float DeltaTime)
{
	InternalUpdate(DeltaTime);
}

TStatId UQuestManager::GetStatId() const
{
	return TStatId();
}

UQuestManager::UQuestManager() {}

UWorld* UQuestManager::GetWorld() const
{
	if (HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
	{
		return nullptr;
	}

	// Get from outer
	if (UObject* Outer = GetOuter())
	{
		if (UWorld* World = Outer->GetWorld())
		{
			return World;
		}
	}

	return nullptr;
}

//UQuestManager* UQuestManager::GetQuestManager(const UObject* WorldContext)
//{
//	return GetQuestManager_Internal(WorldContext, false);
//}

//UQuestManager* UQuestManager::GetQuestManager_Internal(const UObject* WorldContext, bool bCreated)
//{
//	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
//	if (!World)
//	{
//		return nullptr;
//	}
//
//	UGameInstance* GameInstance = World->GetGameInstance();
//	if (!GameInstance)
//	{
//		return nullptr;
//	}
//
//	if (GameInstance->GetClass()->ImplementsInterface(UQuestManagerAccess::StaticClass()))
//	{
//		UQuestManager* Manager = IQuestManagerAccess::Execute_GetQuestManagerInstance(GameInstance);
//		if (Manager)
//		{
//			return Manager;
//		}
//		else if(!bCreated) // Not created yet. We call this again with bCreated=true when creating a new one to prevent stackoverflow
//		{
//			Manager = NewObject<UQuestManager>(GameInstance); // Create new manager with outer set to GameInstance
//			check(Manager);
//
//			IQuestManagerAccess::Execute_RetainQuestManager(GameInstance, Manager);
//
//			Manager->InitializeQuestManager();
//			return GetQuestManager_Internal(WorldContext, true); // Call again to ensure that Execute_GetQuestManagerInstance returns valid manager
//		}
//	}
//
//	ensure(false && "Can't get QuestManager! Have you implemented IQuestManagerAccess to the game instance?");
//	return nullptr;
//}

void UQuestManager::InitializeQuestManager()
{
	InternalLoadAllQuestAssets();

	QuestComponents.Empty();
	MessageListeners.Empty();
	QuestContextMessageListeners.Empty();
}

void UQuestManager::RegisterQuestComponent(UQuestComponent* Component)
{
	if (ensure(Component))
	{
		QuestComponents.Add(Component);
	}
}

void UQuestManager::UnregisterQuestComponent(UQuestComponent* Component)
{
	if (ensure(Component))
	{
		QuestComponents.Remove(Component);
	}
}

void UQuestManager::RegisterMessageListener(UQuestMessageComponent* Listener)
{
	if (ensure(Listener))
	{
		MessageListeners.Add(Listener);
	}
}

void UQuestManager::UnregisterMessageListener(UQuestMessageComponent* Listener)
{
	if (ensure(Listener))
	{
		MessageListeners.Remove(Listener);
	}
}

void UQuestManager::RegisterQuestContextMessageListener(IQuestMessageInterface* MessageInterface, UQuestContext* Context)
{
	if (!ensure(Context && MessageInterface))
	{
		return;
	}

	if (MessageInterface == this)
	{
		return;
	}

	QuestContextMessageListeners.Add(FQuestContextMessageListener(Context, MessageInterface));
}

void UQuestManager::UnregisterQuestContextMessageListener(IQuestMessageInterface* MessageInterface, UQuestContext* Context)
{
	if (!ensure(Context && MessageInterface))
	{
		return;
	}

	if (MessageInterface == this)
	{
		return;
	}

	QuestContextMessageListeners.Remove(FQuestContextMessageListener(Context, MessageInterface));
}

void UQuestManager::ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient, AActor* Sender)
{
	if (Recipient)
	{
		// Single receiver
		for (FQuestContextMessageListener& Listener : QuestContextMessageListeners)
		{
			if (!ensure(Listener.Context && Listener.GetListener()))
			{
				continue;
			}

			const UQuestComponent* ContextQuestComponent = Listener.Context->GetOwnerQuestComponent();
			const UQuestComponent* ReceipentQuestComponent = UQuestUtilityLibrary::FindRelevantQuestComponentFromActor(Recipient);

			if (ContextQuestComponent && ReceipentQuestComponent)
			{
				if (ContextQuestComponent == ReceipentQuestComponent)
				{
					Listener.GetListener()->ReceiveMessageFromListener(MessageTag, Payload, Recipient, Sender);
				}
			}
		}
	}
	else // Broadcast
	{
		
		for (FQuestContextMessageListener& Listener : QuestContextMessageListeners)
		{
			if (IQuestMessageInterface* MessageInterface = Listener.GetListener())
			{
				MessageInterface->ReceiveMessageFromListener(MessageTag, Payload, Recipient, Sender);
			}
		}
	}
}

void UQuestManager::LoadAllQuestAssets(const UObject* WorldContext)
{
	if (UQuestManager* QuestManager = WorldContext->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		QuestManager->InternalLoadAllQuestAssets();
		return;
	}
	// Runtime
	//if (UQuestManager* Manager = GetQuestManager(WorldContext))
	//{
	//	Manager->InternalLoadAllQuestAssets();
	//	return;
	//}

#if WITH_EDITOR
	// Fallback to load in editor
	LoadAllQuestAssetsInEditor();
#endif
}

void UQuestManager::LoadAllQuestAssetsInEditor()
{
#if WITH_EDITOR
	if (!bLoadedEditorQuestAssets)
	{
		if (GIsEditor && !IsRunningCommandlet())
		{
			bool bInPlayMode = GWorld ? GWorld->HasBegunPlay() : false;
			if (!bInPlayMode)
			{
				UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UQuest::StaticClass(), false, GIsEditor && !IsRunningCommandlet());
				ObjectLibrary->bIncludeOnlyOnDiskAssets = false;

				TArray<FString> Paths;
				Paths.Add(TEXT("/Game"));

				ObjectLibrary->AddToRoot();
				ObjectLibrary->LoadAssetDataFromPaths(Paths, true);
				ObjectLibrary->LoadAssetsFromAssetData();
				ObjectLibrary->RemoveFromRoot();

				bLoadedEditorQuestAssets = true;
			}
		}
	}
#endif
}

void UQuestManager::GetAllLoadedQuestAssets(TArray<const UQuest*>& OutQuestAssets)
{
	OutQuestAssets.Empty();
	for (TObjectIterator<UQuest> Itr; Itr; ++Itr)
	{
		UQuest* QuestAsset = *Itr;
		if (IsValid(QuestAsset))
		{
			OutQuestAssets.Add(QuestAsset);
		}
	}
}

void UQuestManager::InternalUpdate(float DeltaTime)
{
	for (UQuestComponent* Component : QuestComponents)
	{
		if (IsValid(Component))
		{
			Component->InternalEvaluate();
			Component->InternalTickServices(DeltaTime);
			Component->InternalReplicateContexts();
		}
	}
}

void UQuestManager::InternalLoadAllQuestAssets()
{
	if (!RuntimeQuestAssetLibrary.bInitialized || !RuntimeQuestAssetLibrary.bLoaded)
	{
		// Initialize
		RuntimeQuestAssetLibrary = FQuestAssetLibrary();

		const TArray<FString>* QuestAssetPaths = nullptr;
		const UQuestFrameworkSettings* Settings = GetDefault<UQuestFrameworkSettings>();
		if (Settings)
		{
			QuestAssetPaths = &Settings->QuestAssetPaths;
		}

		TArray<FString> ValidPaths;
		if (QuestAssetPaths)
		{
			for (const FString& Path : (*QuestAssetPaths))
			{
				if (Path.IsEmpty())
				{
					continue;
				}
				ValidPaths.Add(Path);
			}
		}

		if (ValidPaths.Num() <= 0)
		{
			ValidPaths.Add(TEXT("/Game"));
			UE_LOG(LogQuestFramework, Warning, TEXT("Paths for quest assets were not set. Scanning all assets. This operation may be slow! Prefer adding QuestAssetPaths to config file."));
		}

		RuntimeQuestAssetLibrary.Paths = ValidPaths;
		RuntimeQuestAssetLibrary.bLoaded = false;

		if (!RuntimeQuestAssetLibrary.QuestAssetObjectLibrary)
		{
			RuntimeQuestAssetLibrary.QuestAssetObjectLibrary = UObjectLibrary::CreateLibrary(UQuest::StaticClass(), false, GIsEditor && !IsRunningCommandlet());
			if (GIsEditor)
			{
				RuntimeQuestAssetLibrary.QuestAssetObjectLibrary->bIncludeOnlyOnDiskAssets = false;
			}
		}

		RuntimeQuestAssetLibrary.bInitialized = RuntimeQuestAssetLibrary.QuestAssetObjectLibrary != nullptr;
		const int32 LoadedAssetNum = LoadQuestAssetLibrary(RuntimeQuestAssetLibrary);

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
		TArray<const UQuest*> QuestAssetEnsureArray;
		GetAllLoadedQuestAssets(QuestAssetEnsureArray);
		if (QuestAssetEnsureArray.Num() != LoadedAssetNum)
		{
			UE_LOG(LogQuestFramework, Warning, TEXT("Quest assets count missmatch! Requested load count: %d, actual count in memory: %d. Make sure that search paths are set correctly, so all quest assets would be loaded correctly in packaged builds."), LoadedAssetNum, QuestAssetEnsureArray.Num());
		}
#endif

	}
}

int32 UQuestManager::LoadQuestAssetLibrary(FQuestAssetLibrary& Library)
{
	UE_LOG(LogQuestFramework, Log, TEXT("Loading quest assets to runtime library..."));

	if (!Library.bInitialized)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't load quest asset library because it is not initialized!"));
		return -1;
	}

	if (Library.bLoaded)
	{
		UE_LOG(LogQuestFramework, Log, TEXT("Quest assets were already loaded."));
		return -1;
	}

	if (Library.Paths.Num() <= 0)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't load quest asset library because paths are empty!"));
		return -1;
	}

	check(Library.QuestAssetObjectLibrary != nullptr);

	// Load
	const int32 Count = Library.QuestAssetObjectLibrary->LoadAssetDataFromPaths(Library.Paths, true);
	Library.QuestAssetObjectLibrary->LoadAssetsFromAssetData();
	Library.bLoaded = Count > 0;

	if (Library.bLoaded)
	{
		UE_LOG(LogQuestFramework, Log, TEXT("%d quest assets loaded!"), Count);
	}
	else
	{
		UE_LOG(LogQuestFramework, Log, TEXT("No quest assets were found to load!"));
	}

	return Count;
}