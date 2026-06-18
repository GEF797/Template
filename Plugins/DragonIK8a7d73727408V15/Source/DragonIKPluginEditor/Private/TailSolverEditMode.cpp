/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/

#include "TailSolverEditMode.h"
#include "AnimGraphNode_TailSolver.h"
#include "IPersonaPreviewScene.h"
#include "Animation/DebugSkelMeshComponent.h"

#include "SceneManagement.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
//#include "Materials/MaterialExpressionConstant3Vector.h"



const FEditorModeID TailSolverEditModes::TailSolver("AnimGraph.DragonControl.TailSolver");


void FTailSolverEditMode::EnterMode(class UAnimGraphNode_Base* InEditorNode, struct FAnimNode_Base* InRuntimeNode)
{
	RuntimeNode = static_cast<FAnimNode_TailSolver*>(InRuntimeNode);
	GraphNode = CastChecked<UAnimGraphNode_TailSolver>(InEditorNode);

	FDragonControlBaseEditMode::EnterMode(InEditorNode, InRuntimeNode);
}

void FTailSolverEditMode::ExitMode()
{
	RuntimeNode = nullptr;
	GraphNode = nullptr;

	FDragonControlBaseEditMode::ExitMode();
}

FVector FTailSolverEditMode::GetWidgetLocation() const
{


	if (GraphNode)
	{
		//if (GraphNode->ik_node.is_focus_debugtarget)
		{
			return GraphNode->ik_node.CachedEffectorCSTransform.GetLocation();
		}
	}


	return FVector(0, 0, 0);

}

UE::Widget::EWidgetMode FTailSolverEditMode::GetWidgetMode() const
{
	return UE::Widget::WM_Translate;
}





struct HTailSolverHandleHitProxy : public HHitProxy
{
	DECLARE_HIT_PROXY()

	FTransform Debug_Transform;


	HTailSolverHandleHitProxy(FTransform Debug_Transform_in): HHitProxy(HPP_World), Debug_Transform(Debug_Transform_in)
	{
	}

	// HHitProxy interface
	virtual EMouseCursor::Type GetMouseCursor() override { return EMouseCursor::CardinalCross; }
	// End of HHitProxy interface
};

IMPLEMENT_HIT_PROXY(HTailSolverHandleHitProxy, HHitProxy)





void FTailSolverEditMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	UDebugSkelMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();



	if (RuntimeNode)
	{

		for (int32 i = 0; i < RuntimeNode->Chain.Num(); i++)
		{



			UMaterialInstanceDynamic* Trace_Mat = UMaterialInstanceDynamic::Create(GEngine->ArrowMaterial, GEngine->ArrowMaterial);
			Trace_Mat->SetVectorParameterValue("GizmoColor", FLinearColor(FColor::Red));

			const FMaterialRenderProxy* TraceMaterialProxy = Trace_Mat->GetRenderProxy();


			UMaterialInstanceDynamic* Bone_Height_Mat = UMaterialInstanceDynamic::Create(GEngine->ArrowMaterial, GEngine->ArrowMaterial);
			Bone_Height_Mat->SetVectorParameterValue("GizmoColor", FLinearColor(FColor::Cyan));

			const FMaterialRenderProxy* BoneHeightMaterialProxy = Bone_Height_Mat->GetRenderProxy();


			//PDI->SetHitProxy(new HDragonFootSolverHandleHitProxy(i, 0));

			FTransform StartTransform = FTransform::Identity;
			if (RuntimeNode->Chain.Num() > i)
			{

				
				
				StartTransform = RuntimeNode->Chain[i].OriginalPosition;
			//	StartTransform = FTransform::Identity;

				FVector Line_Pos_Up = StartTransform.GetLocation()+ RuntimeNode->character_direction_vector_CS * RuntimeNode->Trace_Up_Height;
				FVector Line_Pos_Down = StartTransform.GetLocation() - RuntimeNode->character_direction_vector_CS * RuntimeNode->Trace_Down_Height;

				FVector Bone_Height = StartTransform.GetLocation() - RuntimeNode->character_direction_vector_CS * RuntimeNode->Tail_Bone_Height;


				if(i < RuntimeNode->Custom_Bone_Heights.Tail_Chain_Heights.Num())
				{
					Bone_Height = StartTransform.GetLocation() - RuntimeNode->character_direction_vector_CS * RuntimeNode->Custom_Bone_Heights.Tail_Chain_Heights[i];
				}


				//StartTransform.AddToTranslation(RuntimeNode->dragon_input_data.FeetBones[i].Knee_Direction_Offset);
				

				const float Scale = View->WorldToScreen(StartTransform.GetLocation()).W * (4.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]);
			//	DrawSphere(PDI, StartTransform.GetLocation(), FRotator::ZeroRotator, FVector(8.0f) * Scale, 64, 64, SphereMaterialProxy, SDPG_Foreground);

				if (RuntimeNode->Use_Sphere_Trace)
				{


					FVector Base = UKismetMathLibrary::VLerp(Line_Pos_Up, Line_Pos_Down,0.5f);  // Center position


					//FVector X = FVector(1, 0, 0);
					//FVector Y = FVector(0, 1, 0);
					//FVector Z = FVector(0, 0, 1);
					


					// Direction vector from StartPoint to EndPoint
					FVector Z = (Line_Pos_Up - Line_Pos_Down).GetSafeNormal();

					// Compute perpendicular vectors (X and Y) to define the orientation
					FVector X, Y;
					FRotationMatrix RotationMatrix(Z.Rotation());
					X = RotationMatrix.GetUnitAxis(EAxis::X);
					Y = RotationMatrix.GetUnitAxis(EAxis::Y);


					
					
					float HalfHeight = (Line_Pos_Up- Line_Pos_Down).Size()*0.5f;
					int32 NumSides = 8;  // Smoothness
					FLinearColor Color = FLinearColor::Red;
					uint8 DepthPriority = SDPG_World;

					// Draw the wireframe cylinder
					DrawWireCylinder(PDI, Base, X, Y, Z, FColor::Red, RuntimeNode->Sphere_Trace_Radius, HalfHeight, NumSides, SDPG_Foreground);


					FTransform SphereUpTransform = FTransform::Identity;
					SphereUpTransform.SetLocation(Line_Pos_Up);

					DrawWireSphere(PDI, SphereUpTransform, FLinearColor::Red, RuntimeNode->Sphere_Trace_Radius, NumSides, SDPG_Foreground);

					SphereUpTransform.SetLocation(Line_Pos_Down);

					DrawWireSphere(PDI, SphereUpTransform, FLinearColor::Red, RuntimeNode->Sphere_Trace_Radius, NumSides, SDPG_Foreground);

					//DrawDebugCylinder(PDI, Line_Pos_Up, Line_Pos_Down, RuntimeNode->Sphere_Trace_Radius, 4,FColor::Red);
				}
				else
				{
					DrawDashedLine(PDI, Line_Pos_Up, Line_Pos_Down, FLinearColor::Red, 2, 5);
				}


				DrawCylinder(PDI, StartTransform.GetLocation(), Bone_Height, 5, 4, BoneHeightMaterialProxy, SDPG_Foreground);









			}
		}

		RuntimeNode->ConditionalDebugDraw(PDI, SkelComp);

	}




	//UDebugSkelMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();

	/*
	if (RuntimeNode)
	{
		UMaterialInstanceDynamic* head_mat = UMaterialInstanceDynamic::Create(GEngine->ArrowMaterial, GEngine->ArrowMaterial);
		head_mat->SetVectorParameterValue("GizmoColor", FLinearColor(FColor::Yellow));
		//const FMaterialRenderProxy* TargetMaterialProxy = GEngine->ArrowMaterialYellow->GetRenderProxy();

		const FMaterialRenderProxy* TargetMaterialProxy = head_mat->GetRenderProxy();

		PDI->SetHitProxy(new HTailSolverHandleHitProxy(RuntimeNode->CachedEffectorCSTransform));


		FTransform StartTransform = RuntimeNode->CachedEffectorCSTransform;
		//FVector Head_Transform = RuntimeNode->Head_Orig_Transform.GetLocation();
		{
			const float Scale = View->WorldToScreen(StartTransform.GetLocation()).W * (4.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]);
			DrawSphere(PDI, StartTransform.GetLocation(), FRotator::ZeroRotator, FVector(8.0f) * Scale, 64, 64, TargetMaterialProxy, SDPG_Foreground);
		//	DrawDashedLine(PDI, Head_Transform, StartTransform.GetLocation(), FLinearColor::Black, 2, 5);
		}

		FVector Calibration_Offset = RuntimeNode->character_direction_vector_CS*-1;

		DrawCylinder(PDI,FVector(0,0,-10000000)+ Calibration_Offset,FVector(0,0,100000)+ Calibration_Offset,2,4, TargetMaterialProxy, SDPG_Foreground);
	}
	*/

		RuntimeNode->ConditionalDebugDraw(PDI, SkelComp);



		

	

	PDI->SetHitProxy(nullptr);


}




bool FTailSolverEditMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	bool bResult = FDragonControlBaseEditMode::HandleClick(InViewportClient, HitProxy, Click);

	

	//	if ((GraphNode->ik_node.is_focus_debugtarget))
	{
		if (HitProxy != nullptr && HitProxy->IsA(HTailSolverHandleHitProxy::StaticGetType()))
		{

			bResult = true;

		}
	}
	

	return bResult;
}



FName FTailSolverEditMode::GetSelectedBone() const
{

	

		return "None";

	//return GraphNode->;
}



void FTailSolverEditMode::DoRotation(FRotator& InRotation)
{

	
	if (GraphNode != nullptr)
	{
		GraphNode->ik_node.CachedEffectorCSTransform.SetRotation(InRotation.Quaternion()*GraphNode->ik_node.CachedEffectorCSTransform.GetRotation());
		RuntimeNode->CachedEffectorCSTransform.SetRotation(InRotation.Quaternion()*RuntimeNode->CachedEffectorCSTransform.GetRotation());
	//	Target_Transform_Value = GraphNode->ik_node.CachedEffectorCSTransform.GetLocation();
	}


}

// @todo: will support this since now we have LookAtLocation
void FTailSolverEditMode::DoTranslation(FVector& InTranslation)
{


	if (GraphNode != nullptr)
	{
		GraphNode->ik_node.CachedEffectorCSTransform.AddToTranslation(InTranslation);
		RuntimeNode->CachedEffectorCSTransform.AddToTranslation(InTranslation);
	//	Target_Transform_Value = GraphNode->ik_node.CachedEffectorCSTransform.GetLocation();
	}
	

//	Target_Transform_Value = GraphNode->ik_node.Debug_LookAtLocation.GetLocation() + InTranslation;


}



void FTailSolverEditMode::DoScale(FVector& InTranslation)
{


	if (GraphNode != nullptr)
	{
		GraphNode->ik_node.CachedEffectorCSTransform.SetScale3D(GraphNode->ik_node.CachedEffectorCSTransform.GetScale3D()+InTranslation);
		RuntimeNode->CachedEffectorCSTransform.SetScale3D(RuntimeNode->CachedEffectorCSTransform.GetScale3D()+InTranslation);
	//	Target_Transform_Value = GraphNode->ik_node.CachedEffectorCSTransform.GetScale3D();
	}
	

	//	Target_Transform_Value = GraphNode->ik_node.Debug_LookAtLocation.GetLocation() + InTranslation;


}