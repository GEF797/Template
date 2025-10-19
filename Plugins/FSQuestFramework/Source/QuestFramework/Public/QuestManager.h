// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Engine/ObjectLibrary.h"
#include "QuestObjectBase.h"
#include "Tickable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest.h"
#include "QuestContext.h"
#include "QuestMessageInterface.h"
#include "QuestSaveState.h"
#include "Components/QuestComponent.h"

#include "QuestManager.generated.h"

class UQuestMessageComponent;

// ------------------------------------ //

/**
 * Represents library of loaded quest asset objects
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestAssetLibrary
{
	GENERATED_BODY()

public:

	/** Paths to search for quest assets*/
	UPROPERTY()
	TArray<FString> Paths;

	/** Actual library to load quest assets */
	UPROPERTY()
	TObjectPtr<UObjectLibrary> QuestAssetObjectLibrary;

	UPROPERTY()
	bool bInitialized;

	UPROPERTY()
	bool bLoaded;

	FQuestAssetLibrary()
	{
		QuestAssetObjectLibrary = nullptr;
		bInitialized = false;
		bLoaded = false;
	}
};

/**
 * Stores quest context and message listener to notify quest contexts from arriving messages from gameplay actors
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestContextMessageListener
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TObjectPtr<UQuestContext> Context;

	UPROPERTY()
	TObjectPtr<UObject> Listener;

	FQuestContextMessageListener()
	{
		Context = nullptr;
		Listener = nullptr;
	}

	FQuestContextMessageListener(UQuestContext* InContext, IQuestMessageInterface* InListener)
	{
		Context = InContext;
		Listener = Cast<UObject>(InListener);
		ensure(Context && Listener);
	}

	IQuestMessageInterface* GetListener() const 
	{
		return Cast<IQuestMessageInterface>(Listener);
	}

	bool operator==(const FQuestContextMessageListener& Other) const
	{
		return Context == Other.Context && Listener == Other.Listener;
	}

	bool operator!=(const FQuestContextMessageListener& Other) const
	{
		return !((*this) == Other);
	}

	friend uint32 GetTypeHash(const FQuestContextMessageListener& Data)
	{
		return GetTypeHash(Data.Context) ^ GetTypeHash(Data.Listener);
	}
};

// --------------------------------------- //

/**
 * Singleton manager for quest framework.
 * Should be referenced in game instance
 */
UCLASS(NotBlueprintable, BlueprintType, Config = Game)
class QUESTFRAMEWORK_API UQuestManager : public UGameInstanceSubsystem,
	public FTickableGameObject,
	public IQuestMessageInterface
{
	GENERATED_BODY()

public: // FTickableGameObject

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual TStatId GetStatId() const override;
	
//private:

	//static UQuestManager* GetQuestManager_Internal(const UObject* WorldContext, bool bCreated);

public:

	UQuestManager();

	UWorld* GetWorld() const override;

	/**
	 * Returns quest manager instance.
	 * NOTE! 
	 * Quest manager must be referenced in game instance and it must implement IQuestManagerAccess and override GetQuestManagerInstance()
	 */
	//UFUNCTION(BlueprintCallable, Category = "Quest Manager", meta = (WorldContext="WorldContext"))
	//static UQuestManager* GetQuestManager(const UObject* WorldContext);

	// Called when the instance is created to initialize the manager
	void InitializeQuestManager();

	/**
	 * Registers/unregisters quest component
	 */
	void RegisterQuestComponent(UQuestComponent* Component);
	void UnregisterQuestComponent(UQuestComponent* Component);

	/*
	* Message listener register/unregister.
	* Components register on BeginPlay and Unregister on EndPlay
	*/
	void RegisterMessageListener(UQuestMessageComponent* Listener);
	void UnregisterMessageListener(UQuestMessageComponent* Listener);
	const TSet<TObjectPtr<UQuestMessageComponent>>& GetMessageListeners() const { return MessageListeners; }

	/**
	 * Register/unregister of message listeners in quest contexts (eg. services of quest)
	 */
	void RegisterQuestContextMessageListener(IQuestMessageInterface* MessageInterface, UQuestContext* Context);
	void UnregisterQuestContextMessageListener(IQuestMessageInterface* MessageInterface, UQuestContext* Context);

	virtual void ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient, AActor* Sender) override;

public:

	/**
	 * Searches content paths for quests and loads then all into memory (runtime version)
	 */
	static void LoadAllQuestAssets(const UObject* WorldContext);

	/**
	 * Searches game content paths for quests and loads then all into memory
	 * Does nothing in non-editor builds
	 */
	static void LoadAllQuestAssetsInEditor();

	/**
	 * Returns list of all loaded quest assets
	 */
	static void GetAllLoadedQuestAssets(TArray<const UQuest*>& OutQuestAssets);

	/*UPROPERTY(BlueprintReadOnly)
	TMap<UQuest*, EQuestResult> ;*/

private:

	void InternalUpdate(float DeltaTime);
	void InternalLoadAllQuestAssets();
	int32 LoadQuestAssetLibrary(FQuestAssetLibrary& Library);

private:

	// Set of registered quest components
	UPROPERTY(Transient)
	TSet<TObjectPtr<UQuestComponent>> QuestComponents;

	// Set of message listeners to notify about messages sent by quest contexts
	UPROPERTY(Transient)
	TSet<TObjectPtr<UQuestMessageComponent>> MessageListeners;

	// Set of message listeners registered from quest context
	UPROPERTY(Transient)
	TSet<FQuestContextMessageListener> QuestContextMessageListeners;

	// Library of quest asset objects
	UPROPERTY(Transient)
	FQuestAssetLibrary RuntimeQuestAssetLibrary;

	// Are quest assets loaded in editor?
	static bool bLoadedEditorQuestAssets;
};
