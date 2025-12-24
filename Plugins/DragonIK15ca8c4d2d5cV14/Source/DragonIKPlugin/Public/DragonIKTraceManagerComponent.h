// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"

#include "DragonIKTraceManagerComponent.generated.h"

//DECLARE_DELEGATE_TwoParams( FTraceDelegate, const FTraceHandle&, FTraceDatum &);

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDragonIKSpineData,const TArray<FDragonIKTraceParamKeyValuePair>,data);

UENUM(BlueprintType)
enum class EDragonIKTraceMType : uint8
{
	LineTrace UMETA(DisplayName = "Line Trace"),
	SphereTrace UMETA(DisplayName = "Sphere Trace"),
};

/*
USTRUCT(BlueprintType)
struct FDragonIKTraceHandlerStruct
{
	GENERATED_BODY()

	bool is_activated = false;
	bool bWantsTrace = false;
	FTraceHandle LastTraceHandle;
	FTraceDelegate TraceDelegate;
};
*/

USTRUCT(BlueprintType)
struct FDragonIKTraceKeyValuePair
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,Category = DragonIKTraceManager, BlueprintReadOnly)
	FName Key;

	UPROPERTY(EditAnywhere,Category = DragonIKTraceManager, BlueprintReadOnly)
	FName BoneName;

	UPROPERTY(EditAnywhere,Category = DragonIKTraceManager, BlueprintReadOnly)
	FHitResult hit_result;

	FDragonIKTraceKeyValuePair()
		: Key(TEXT("")),
		  hit_result(FHitResult())
	{
	}

	

	
	
};






USTRUCT(BlueprintType)
struct FDragonIKTraceMParams
{
	GENERATED_BODY()

public:

	bool allow = true;

	FName TraceIdentifier = "";
	
	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector StartLocation = FVector::ZeroVector;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector EndLocation = FVector::ZeroVector;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SphereRadius = 20;

	bool trace_complex = true;

	bool is_line_type = true;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<ETraceTypeQuery> ChannelInput = ETraceTypeQuery::TraceTypeQuery1;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EDragonIKTraceMType TraceType = EDragonIKTraceMType::LineTrace;

	FHitResult hit_result;

	FName bone_text;
	
	FDragonIKTraceMParams()
	{
		TraceIdentifier = "";
	
		//UPROPERTY(BlueprintReadWrite, EditAnywhere)
		StartLocation = FVector::ZeroVector;

		//UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EndLocation = FVector::ZeroVector;

		//UPROPERTY(BlueprintReadWrite, EditAnywhere)
		SphereRadius = 20;

		//UPROPERTY(BlueprintReadWrite, EditAnywhere)
		ChannelInput = ETraceTypeQuery::TraceTypeQuery1;

		//UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TraceType = EDragonIKTraceMType::LineTrace;

		trace_complex = true;
		
		allow = true;

		bone_text = "";
	}

	bool operator==(FDragonIKTraceMParams& B) const
	{
		// Compare all relevant fields for equality
		return TraceIdentifier == B.TraceIdentifier &&
			   StartLocation == B.StartLocation &&
			   EndLocation == B.EndLocation &&
			   SphereRadius == B.SphereRadius &&
			   ChannelInput == B.ChannelInput &&
			   TraceType == B.TraceType;
	}

	
};

USTRUCT(BlueprintType)
struct FDragonIKTraceParamKeyValuePair
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,Category = DragonIKTraceManager, BlueprintReadOnly)
	FName Key;

	UPROPERTY(EditAnywhere,Category = DragonIKTraceManager, BlueprintReadOnly)
	FDragonIKTraceMParams trace_params;

	FDragonIKTraceParamKeyValuePair()
		: Key(TEXT("")),
		  trace_params(FDragonIKTraceMParams())
	{
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDragonIKFootSolverTraceData,const TArray<FDragonIKTraceKeyValuePair>&,hit_data_pair);


UCLASS( ClassGroup=(DragonIK), meta=(DisplayName = "Dragonik Trace Manager",BlueprintSpawnableComponent) )
class DRAGONIKPLUGIN_API UDragonIKTraceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDragonIKTraceManagerComponent();



	//UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// bool use_async_trace_mode = true;



	/*
*
* This is a new optional feature which centrally manages the trace firing logic, of the spine and foot solvers of your character's animation blueprint.
* Enable "Use External Trace Management Component" boolean parameter in the dragonik foot solver and spine solver.
* Using this could potentially improve editor stability and avoid certain rare niche crashes such as during multithreading.
* This lets the spine and foot solvers to do it's calculations without the burden of firing traces from it's internal update thread.
* You also get access to the trace hit info from the "On Dragonik Foot Hit Data" callback event.
*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = DragonIKTraceManager, meta = (DisplayName = "HOVER MOUSE HERE FOR TIPS!", PinHiddenByDefault))
	FName read_this = "Hover here to learn how to use this component.";


	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = DragonIKTraceManager, meta = (DisplayName = "Is Enabled ?"))
	 bool Is_Enabled = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = DragonIKTraceManager, meta = (DisplayName = "Show Spine Solver Traces"))
	 bool bShow_Spine_Trace_Lines_InGame = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = DragonIKTraceManager, meta = (DisplayName = "Show Foot Solver Traces"))
	 bool bShow_Foot_Trace_Lines_InGame = false;

	int test_count = 0;

	bool hand_shaked = false;

	bool tick_busy = false;

	TArray<FDragonIKTraceParamKeyValuePair> PendingTraces = TArray<FDragonIKTraceParamKeyValuePair>();

	TArray<FDragonIKTraceKeyValuePair> HitResultMap = TArray<FDragonIKTraceKeyValuePair>();

	TArray<FName> Solver_Markers;
	
	// Function to issue a trace input request
	//UFUNCTION(BlueprintCallable, Category = "DragonIK Trace Input Issue")
	 void IssueTraceInput(FDragonIKTraceMParams& TraceParams);


	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FDragonIKFootSolverTraceData OnDragonikFootHitData;
	
	UFUNCTION(BlueprintImplementableEvent)
	void SpineSolverTraceData(const TArray<FDragonIKTraceKeyValuePair>& hit_data_pair);

	
	
	bool Hitmap_FindKeyExists(TArray<FDragonIKTraceKeyValuePair> array,FName key);
	FHitResult Hitmap_GetValueFromKey(TArray<FDragonIKTraceKeyValuePair> array,FName key);
	void Hitmap_SetValueOnKey(TArray<FDragonIKTraceKeyValuePair> &array,FName key,FHitResult value,FName bone_input);


	bool PendingTraces_FindKeyExists(TArray<FDragonIKTraceParamKeyValuePair> array,FName key);
	FDragonIKTraceMParams PendingTraces_GetValueFromKey(TArray<FDragonIKTraceParamKeyValuePair> array,FName key);
	void PendingTraces_SetValueOnKey(TArray<FDragonIKTraceParamKeyValuePair> &array,FName key,FDragonIKTraceMParams value);

	
	//TArray<bool> bWantsTrace;
	//TArray<FTraceHandle> LastTraceHandle;
//	TArray<FDragonIKTraceHandlerStruct> LastTraceHandle;
	//FTraceDelegate TraceDelegate;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
