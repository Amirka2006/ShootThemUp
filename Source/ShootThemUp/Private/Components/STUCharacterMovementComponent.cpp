// Shoot Them Up Game, All Rights Reserved.


#include "STUCharacterMovementComponent.h"
#include "Player/STUBaseCharacter.h"


float USTUCharacterMovementComponent::GetMaxSpeed() const
{
    const float MaxSpeed = Super::GetMaxSpeed();
    ASTUBaseCharacter* Player = Cast<ASTUBaseCharacter>(GetPawnOwner());
    return Player && Player->IsRunning() ? MaxSpeed * RunModifier : MaxSpeed;
}
