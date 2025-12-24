/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/

#include "AnimGraphNode_DragonFatSolver.h"


#include "AnimationGraphSchema.h"



void UAnimGraphNode_DragonFatSolver::CreateOutputPins()
{
	CreatePin(EGPD_Output, UAnimationGraphSchema::PC_Struct, FComponentSpacePoseLink::StaticStruct(), TEXT("Pose"));
}


UAnimGraphNode_DragonFatSolver::UAnimGraphNode_DragonFatSolver(const FObjectInitializer & ObjectInitializer)
{
}


//FEditorModeID UAnimGraphNode_DragonFatSolver::GetEditorMode() const
//{
//	return AnimNodeEditModes::ModifyBone;
//}



void UAnimGraphNode_DragonFatSolver::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;


}


FText UAnimGraphNode_DragonFatSolver::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("DragonIK Dragon Fat Solver"));
}

FText UAnimGraphNode_DragonFatSolver::GetTooltipText() const
{
	return FText::FromString(FString("A Solver that can make a character fat or lean via bone scaling while preserving bone positions and rotations"));
}

FString UAnimGraphNode_DragonFatSolver::GetNodeCategory() const
{
	return FString("Dragon.IK Plugin");
}

FLinearColor UAnimGraphNode_DragonFatSolver::GetNodeTitleColor() const
{
	return FLinearColor(10.0f / 255.0f, 127.0f / 255.0f, 248.0f / 255.0f);

}


