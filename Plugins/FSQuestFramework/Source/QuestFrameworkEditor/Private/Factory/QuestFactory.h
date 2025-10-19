// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "QuestFactory.generated.h"

class UQuestEvent;
class UQuestCondition;
class UQuestObjectiveObserver;
class UQuestService_BlueprintBase;
class UQuestTextArgument;

/**
 * Factory class to create quest assets in content browser
 */
UCLASS()
class UQuestFactory : public UFactory
{
	GENERATED_BODY()

public:

	UQuestFactory(const FObjectInitializer& ObjectInitializer);
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class UQuestObjectFactoryBase : public UFactory
{
	GENERATED_BODY()

protected:

	UPROPERTY()
	TSubclassOf<UObject> ParentClass;
	
	UPROPERTY()
	FText Title;

public:

	UQuestObjectFactoryBase(const FObjectInitializer& ObjectInitializer);

	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class UQuestEventFactory : public UQuestObjectFactoryBase
{
	GENERATED_BODY()
public:
	UQuestEventFactory(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class UQuestConditionFactory : public UQuestObjectFactoryBase
{
	GENERATED_BODY()
public:
	UQuestConditionFactory(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class UQuestObjectiveObserverFactory : public UQuestObjectFactoryBase
{
	GENERATED_BODY()
public:
	UQuestObjectiveObserverFactory(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class UQuestServiceFactory : public UQuestObjectFactoryBase
{
	GENERATED_BODY()
public:
	UQuestServiceFactory(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class UQuestTextArgumentFactory : public UQuestObjectFactoryBase
{
	GENERATED_BODY()
public:
	UQuestTextArgumentFactory(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class UQuestCustomDataFactory : public UQuestObjectFactoryBase
{
	GENERATED_BODY()
public:
	UQuestCustomDataFactory(const FObjectInitializer& ObjectInitializer);
};
