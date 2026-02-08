/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/

#include "AnimGraphNode_DragonTransformRelayer.h"



#include "AnimationGraphSchema.h"



void UAnimGraphNode_DragonTransformRelayer::CreateOutputPins()
{
	CreatePin(EGPD_Output, UAnimationGraphSchema::PC_Struct, FComponentSpacePoseLink::StaticStruct(), TEXT("Pose"));
}


UAnimGraphNode_DragonTransformRelayer::UAnimGraphNode_DragonTransformRelayer(const FObjectInitializer & ObjectInitializer)
{
}



/*
void UAnimGraphNode_DragonTransformRelayer::Draw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent * PreviewSkelMeshComp) const
{
	
	if (PreviewSkelMeshComp)
	{
		if (FAnimNode_DragonTransformRelayer* ActiveNode = GetActiveInstanceNode<FAnimNode_DragonTransformRelayer>(PreviewSkelMeshComp->GetAnimInstance()))
		{
			ActiveNode->ConditionalDebugDraw(PDI, PreviewSkelMeshComp);
		}
	}
}
*/


FText UAnimGraphNode_DragonTransformRelayer::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString("DragonIK Transform Relayer"));
}

FText UAnimGraphNode_DragonTransformRelayer::GetTooltipText() const
{
	return FText::FromString(FString("Send the bone transforms from this given point of the animgraph into the dragonik transform receiver component"));
}

FString UAnimGraphNode_DragonTransformRelayer::GetNodeCategory() const
{
	return FString("Dragon.IK Plugin");
}

FLinearColor UAnimGraphNode_DragonTransformRelayer::GetNodeTitleColor() const
{
	return FLinearColor(10.0f / 255.0f, 127.0f / 255.0f, 248.0f / 255.0f);
}
