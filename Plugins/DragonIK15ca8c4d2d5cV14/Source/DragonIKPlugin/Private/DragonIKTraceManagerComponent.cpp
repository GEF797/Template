// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonIKTraceManagerComponent.h"

#include "DragonIK_Library.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UDragonIKTraceManagerComponent::UDragonIKTraceManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UDragonIKTraceManagerComponent::IssueTraceInput(FDragonIKTraceMParams& TraceParams)
{
	test_count++;
	
	if(hand_shaked)
	{
		
		if(!Solver_Markers.Contains(TraceParams.TraceIdentifier))
			Solver_Markers.AddUnique(TraceParams.TraceIdentifier);
	
		TraceParams.allow = true;
		
		PendingTraces_SetValueOnKey(PendingTraces,TraceParams.TraceIdentifier,TraceParams);

		if(HitResultMap.Num() > 0)
		{
			if(Solver_Markers.Contains(TraceParams.TraceIdentifier) && HitResultMap.IsEmpty() == false && HitResultMap.Num() > 0 && !TraceParams.TraceIdentifier.IsEqual(""))
			{
				if(Hitmap_FindKeyExists(HitResultMap,TraceParams.TraceIdentifier) )
				{
					//TraceParams.hit_result = HitResultMap[TraceParams.TraceIdentifier];

					TraceParams.hit_result = Hitmap_GetValueFromKey(HitResultMap,TraceParams.TraceIdentifier);

					//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, " HitResultMap " + FString::SanitizeFloat(HitResultMap.Num()));
				}
			}
		}

		



		

	}
}



bool UDragonIKTraceManagerComponent::Hitmap_FindKeyExists(TArray<FDragonIKTraceKeyValuePair> array, FName key)
{


	if(array.Num()>0)
	{
		for (int i=0;i<array.Num();i++)
		{
			if(array[i].Key == key)
			{
				return true;
			}
		}
	}

	return false;
}

FHitResult UDragonIKTraceManagerComponent::Hitmap_GetValueFromKey(TArray<FDragonIKTraceKeyValuePair> array, FName key)
{

	if(array.Num()>0)
	{
		for (int i=0;i<array.Num();i++)
		{
			if(array[i].Key == key)
			{
				return array[i].hit_result;
			}
		}
	}

	return FHitResult();
}

void UDragonIKTraceManagerComponent::Hitmap_SetValueOnKey(TArray<FDragonIKTraceKeyValuePair> &array, FName key,
	FHitResult value,FName bone_input)
{

	{
		bool key_found = false;
		for (int i=0;i<array.Num();i++)
		{
			if(array[i].Key == key)
			{
				array[i].hit_result = value;
				key_found = true;
				return;
			}
		}

		if(key_found == false)
		{
			FDragonIKTraceKeyValuePair trace_value_pair = FDragonIKTraceKeyValuePair();
			trace_value_pair.Key = key;
			trace_value_pair.hit_result = value;
			trace_value_pair.BoneName = bone_input;
			
			array.Add(trace_value_pair);
		}
	}
/*	else
	{
		array = TArray<FDragonIKTraceKeyValuePair>();
	}*/
	
}

bool UDragonIKTraceManagerComponent::PendingTraces_FindKeyExists(TArray<FDragonIKTraceParamKeyValuePair> array, FName key)
{
	if(array.Num()>0)
	{
		for (int i=0;i<array.Num();i++)
		{
			if(array[i].Key == key)
			{
				return true;
			}
		}
	}

	return false;
}

FDragonIKTraceMParams UDragonIKTraceManagerComponent::PendingTraces_GetValueFromKey(TArray<FDragonIKTraceParamKeyValuePair> array, FName key)
{
	if(array.Num()>0)
	{
		for (int i=0;i<array.Num();i++)
		{
			if(array[i].Key == key)
			{
				return array[i].trace_params;
			}
		}
	}

	return FDragonIKTraceMParams();
}

void UDragonIKTraceManagerComponent::PendingTraces_SetValueOnKey(TArray<FDragonIKTraceParamKeyValuePair>& array, FName key,FDragonIKTraceMParams value)
{
	{
		bool key_found = false;
		for (int i=0;i<array.Num();i++)
		{
			if(array[i].Key == key)
			{
				array[i].trace_params = value;
				key_found = true;
				return;
			}
		}

		if(key_found == false)
		{
			FDragonIKTraceParamKeyValuePair trace_value_pair = FDragonIKTraceParamKeyValuePair();
			trace_value_pair.Key = key;
			trace_value_pair.trace_params = value;
			
			array.Add(trace_value_pair);
		}
	}
}

// Called when the game starts
void UDragonIKTraceManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Solver_Markers = TArray<FName>();

	PendingTraces = TArray<FDragonIKTraceParamKeyValuePair>();
	//PendingTraces.AddDefaulted(20);
	HitResultMap = TArray<FDragonIKTraceKeyValuePair>();
	HitResultMap.AddDefaulted(100);
	
	OnDragonikFootHitData.AddDynamic(this, &UDragonIKTraceManagerComponent::SpineSolverTraceData);
	
	// ...
	
}


// Called every frame
void UDragonIKTraceManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const TArray<FName> Solver_Const_Markers = Solver_Markers;

	if(Solver_Const_Markers.Num() > 0 && hand_shaked && Is_Enabled)
	{
		{
			//for (FDragonIKTraceMParams TraceParams : PendingTraces)
			for (int i=0;i<Solver_Const_Markers.Num();i++)
			{
				if(Solver_Const_Markers[i].IsValid())
				{

					FString solver_type = Solver_Const_Markers[i].ToString();
					solver_type = solver_type.Mid(0,1);

					
					FDragonIKTraceMParams current_param = PendingTraces_GetValueFromKey(PendingTraces,Solver_Const_Markers[i]);
				
					if (current_param.TraceType == EDragonIKTraceMType::LineTrace)
					{
						// Perform a line trace
						FHitResult HitResult;

						TArray<AActor*> actor_to_ignore;

						if((solver_type=="F" && bShow_Foot_Trace_Lines_InGame) || (solver_type=="S" && bShow_Spine_Trace_Lines_InGame))
						{
							UKismetSystemLibrary::LineTraceSingle(GetWorld(),current_param.StartLocation,current_param.EndLocation,current_param.ChannelInput,current_param.trace_complex,actor_to_ignore,EDrawDebugTrace::ForOneFrame,HitResult,true);
						}
						else
						{
							UKismetSystemLibrary::LineTraceSingle(GetWorld(),current_param.StartLocation,current_param.EndLocation,current_param.ChannelInput,current_param.trace_complex,actor_to_ignore,EDrawDebugTrace::None,HitResult,true);
						}

						//PendingTraces[Solver_Const_Markers[i]].hit_result = HitResult;
						current_param.hit_result = HitResult;
						PendingTraces_SetValueOnKey(PendingTraces,current_param.TraceIdentifier,current_param);

						Hitmap_SetValueOnKey(HitResultMap,current_param.TraceIdentifier,HitResult,current_param.bone_text);



					
					}
					else if (current_param.TraceType == EDragonIKTraceMType::SphereTrace)
					{
						// Perform a sphere trace
						FHitResult HitResult;
						TArray<AActor*> actor_to_ignore;

						if((solver_type=="F" && bShow_Foot_Trace_Lines_InGame) || (solver_type=="S" && bShow_Spine_Trace_Lines_InGame))
						{
							UKismetSystemLibrary::SphereTraceSingle(GetWorld(),current_param.StartLocation,current_param.EndLocation,current_param.SphereRadius,current_param.ChannelInput,current_param.trace_complex,actor_to_ignore,EDrawDebugTrace::ForOneFrame,HitResult,true);

						}
						else
						{
							UKismetSystemLibrary::SphereTraceSingle(GetWorld(),current_param.StartLocation,current_param.EndLocation,current_param.SphereRadius,current_param.ChannelInput,current_param.trace_complex,actor_to_ignore,EDrawDebugTrace::None,HitResult,true);

						}


						

						//FDragonIKTraceMParams current_param = PendingTraces_GetValueFromKey(PendingTraces,Solver_Const_Markers[i]);
						//current_param.hit_result = Data.OutHits[0];
						current_param.hit_result = HitResult;
					
						//PendingTraces[Solver_Const_Markers[i]].hit_result = HitResult;
						PendingTraces_SetValueOnKey(PendingTraces,current_param.TraceIdentifier,current_param);

						Hitmap_SetValueOnKey(HitResultMap,current_param.TraceIdentifier,HitResult,current_param.bone_text);
						
					
					}

					
					
				}
			}

			TArray<FDragonIKTraceKeyValuePair> footsolver_hitdata = TArray<FDragonIKTraceKeyValuePair>();

			for(int j=0;j<Solver_Const_Markers.Num();j++)
			{
				if(Solver_Const_Markers[j].IsValid())
				{
					FString solver_type = Solver_Const_Markers[j].ToString();
					solver_type = solver_type.Mid(0,1);

					if(solver_type.Equals("F") && HitResultMap.Num() > j)
					{
						footsolver_hitdata.Add(HitResultMap[j]);
					}
				}
			}

			//if(bShow_Spine_Trace_Lines_InGame)
			OnDragonikFootHitData.Broadcast(footsolver_hitdata);
			
		}
	}

	
	
}

