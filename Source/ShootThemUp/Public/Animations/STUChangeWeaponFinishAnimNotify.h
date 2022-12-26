// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "STUChangeWeaponFinishAnimNotify.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponNotifiedSignature, USkeletalMeshComponent*)
UCLASS()
class SHOOTTHEMUP_API USTUChangeWeaponFinishAnimNotify : public UAnimNotify
{
    GENERATED_BODY()
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

    FOnWeaponNotifiedSignature OnWeaponNotified;
};
