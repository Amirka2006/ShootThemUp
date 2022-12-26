// Shoot Them Up Game, All Rights Reserved.


#include "Animations/STUChangeWeaponFinishAnimNotify.h"

void USTUChangeWeaponFinishAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    OnWeaponNotified.Broadcast(MeshComp);
    Super::Notify(MeshComp, Animation);
}
