// Fill out your copyright notice in the Description page of Project Settings.


#include "ANW_TraceWindow.h"

void UANW_TraceWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	FBranchingPointNotifyPayload payload;
	//BranchingPointNotifyBegin(payload);
	MeshComp->GetAnimInstance()->OnPlayMontageNotifyBegin.Broadcast(NotifyName, payload);
}

void UANW_TraceWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	FBranchingPointNotifyPayload payload;
	//BranchingPointNotifyEnd(payload);
	MeshComp->GetAnimInstance()->OnPlayMontageNotifyEnd.Broadcast(NotifyName, payload);
}
