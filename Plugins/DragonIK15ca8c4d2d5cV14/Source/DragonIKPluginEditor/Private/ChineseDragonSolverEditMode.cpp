/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/

#include "ChineseDragonSolverEditMode.h"
#include "AnimGraphNode_ChineseDragonSolver.h"
#include "IPersonaPreviewScene.h"
#include "Animation/DebugSkelMeshComponent.h"

#include "SceneManagement.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
//#include "Materials/MaterialExpressionConstant3Vector.h"



const FEditorModeID ChineseDragonSolverEditModes::ChineseDragonSolver("AnimGraph.DragonControl.ChineseDragonSolver");


void FChineseDragonSolverEditMode::EnterMode(class UAnimGraphNode_Base* InEditorNode, struct FAnimNode_Base* InRuntimeNode)
{
	RuntimeNode = static_cast<FAnimNode_ChineseDragonSolver*>(InRuntimeNode);
	GraphNode = CastChecked<UAnimGraphNode_ChineseDragonSolver>(InEditorNode);

	FDragonControlBaseEditMode::EnterMode(InEditorNode, InRuntimeNode);
}

void FChineseDragonSolverEditMode::ExitMode()
{
	RuntimeNode = nullptr;
	GraphNode = nullptr;

	FDragonControlBaseEditMode::ExitMode();
}

FVector FChineseDragonSolverEditMode::GetWidgetLocation() const
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

UE::Widget::EWidgetMode FChineseDragonSolverEditMode::GetWidgetMode() const
{
	return UE::Widget::WM_Translate;
}





struct HChineseDragonSolverHandleHitProxy : public HHitProxy
{
	DECLARE_HIT_PROXY()

	FTransform Debug_Transform;


	HChineseDragonSolverHandleHitProxy(FTransform Debug_Transform_in): HHitProxy(HPP_World), Debug_Transform(Debug_Transform_in)
	{
	}

	// HHitProxy interface
	virtual EMouseCursor::Type GetMouseCursor() override { return EMouseCursor::CardinalCross; }
	// End of HHitProxy interface
};

IMPLEMENT_HIT_PROXY(HChineseDragonSolverHandleHitProxy, HHitProxy)





void FChineseDragonSolverEditMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	UDebugSkelMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();



	//UDebugSkelMeshComponent* SkelComp = GetAnimPreviewScene().GetPreviewMeshComponent();


	if (RuntimeNode)
	{
		UMaterialInstanceDynamic* head_mat = UMaterialInstanceDynamic::Create(GEngine->ArrowMaterial, GEngine->ArrowMaterial);
		head_mat->SetVectorParameterValue("GizmoColor", FLinearColor(FColor::Yellow));
		//const FMaterialRenderProxy* TargetMaterialProxy = GEngine->ArrowMaterialYellow->GetRenderProxy();

		const FMaterialRenderProxy* TargetMaterialProxy = head_mat->GetRenderProxy();

		PDI->SetHitProxy(new HChineseDragonSolverHandleHitProxy(RuntimeNode->CachedEffectorCSTransform));


		FTransform StartTransform = RuntimeNode->CachedEffectorCSTransform;
		//FVector Head_Transform = RuntimeNode->Head_Orig_Transform.GetLocation();
		{
			const float Scale = View->WorldToScreen(StartTransform.GetLocation()).W * (4.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]);
			DrawSphere(PDI, StartTransform.GetLocation(), FRotator::ZeroRotator, FVector(8.0f) * Scale, 64, 64, TargetMaterialProxy, SDPG_Foreground);
		//	DrawDashedLine(PDI, Head_Transform, StartTransform.GetLocation(), FLinearColor::Black, 2, 5);
		}

		FVector Calibration_Offset = RuntimeNode->Test_Ref_Forward_Axis*RuntimeNode->Pelvis_Positioning_Calibration_Value*-1;

		DrawCylinder(PDI,FVector(0,0,-10000000)+ Calibration_Offset,FVector(0,0,100000)+ Calibration_Offset,2,4, TargetMaterialProxy, SDPG_Foreground);
	}


		RuntimeNode->ConditionalDebugDraw(PDI, SkelComp);



		

	

	PDI->SetHitProxy(nullptr);


}




bool FChineseDragonSolverEditMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	bool bResult = FDragonControlBaseEditMode::HandleClick(InViewportClient, HitProxy, Click);

	

	//	if ((GraphNode->ik_node.is_focus_debugtarget))
	{
		if (HitProxy != nullptr && HitProxy->IsA(HChineseDragonSolverHandleHitProxy::StaticGetType()))
		{

			bResult = true;

		}
	}
	

	return bResult;
}



FName FChineseDragonSolverEditMode::GetSelectedBone() const
{

	

		return "None";

	//return GraphNode->;
}



void FChineseDragonSolverEditMode::DoRotation(FRotator& InRotation)
{

	
	if (GraphNode != nullptr)
	{
		GraphNode->ik_node.CachedEffectorCSTransform.SetRotation(InRotation.Quaternion()*GraphNode->ik_node.CachedEffectorCSTransform.GetRotation());
		RuntimeNode->CachedEffectorCSTransform.SetRotation(InRotation.Quaternion()*RuntimeNode->CachedEffectorCSTransform.GetRotation());
	//	Target_Transform_Value = GraphNode->ik_node.CachedEffectorCSTransform.GetLocation();
	}


}

// @todo: will support this since now we have LookAtLocation
void FChineseDragonSolverEditMode::DoTranslation(FVector& InTranslation)
{


	if (GraphNode != nullptr)
	{
		GraphNode->ik_node.CachedEffectorCSTransform.AddToTranslation(InTranslation);
		RuntimeNode->CachedEffectorCSTransform.AddToTranslation(InTranslation);
	//	Target_Transform_Value = GraphNode->ik_node.CachedEffectorCSTransform.GetLocation();
	}
	

//	Target_Transform_Value = GraphNode->ik_node.Debug_LookAtLocation.GetLocation() + InTranslation;


}



void FChineseDragonSolverEditMode::DoScale(FVector& InTranslation)
{


	if (GraphNode != nullptr)
	{
		GraphNode->ik_node.CachedEffectorCSTransform.SetScale3D(GraphNode->ik_node.CachedEffectorCSTransform.GetScale3D()+InTranslation);
		RuntimeNode->CachedEffectorCSTransform.SetScale3D(RuntimeNode->CachedEffectorCSTransform.GetScale3D()+InTranslation);
	//	Target_Transform_Value = GraphNode->ik_node.CachedEffectorCSTransform.GetScale3D();
	}
	

	//	Target_Transform_Value = GraphNode->ik_node.Debug_LookAtLocation.GetLocation() + InTranslation;


}