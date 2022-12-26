// Shoot Them Up Game, All Rights Reserved.


#include "Components/STUWeaponComponent.h"
#include "Camera/CameraActor.h"
#include "Weapon/STUBaseWeapon.h"
#include "GameFramework/Character.h"
#include "Animations/STUEquipFinishedAnimNotify.h"
#include "Animations/STUChangeWeaponFinishAnimNotify.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeaponComponent, All, All);

USTUWeaponComponent::USTUWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTUWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentWeaponIndex = 0;
    InitAnimations();
    SpawnWeapons();
    // EquipWeapon();
    OnWeaponChange(GetCharacter()->GetMesh());
}


void USTUWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CurrentWeapon = nullptr;
    for (auto Weapon : Weapons)
    {
        Weapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
        Weapon->Destroy();
    }
    Weapons.Empty();

    Super::EndPlay(EndPlayReason);
}

void USTUWeaponComponent::SpawnWeapons()
{
    ACharacter* Character = GetCharacter();
    if (!Character || !GetWorld())
        return;

    for (auto OneWeaponData : WeaponData)
    {
        auto Weapon = GetWorld()->SpawnActor<ASTUBaseWeapon>(OneWeaponData.WeaponClass);
        if (!Weapon)
            continue;

        Weapon->SetOwner(Character);
        Weapons.Add(Weapon);

        AttachWeaponToSocket(Weapon, Character->GetMesh(), WeaponArmorySocketName);
    }
}

void USTUWeaponComponent::AttachWeaponToSocket(ASTUBaseWeapon* Weapon, USceneComponent* SceneComponent, const FName& SocketName)
{
    if (!Weapon || !SceneComponent)
        return;
    FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
    Weapon->AttachToComponent(SceneComponent, AttachmentRules, SocketName);
}

void USTUWeaponComponent::EquipWeapon()
{
    if (!GetCharacter())
        return;

    // if (CurrentWeapon)
    // {
    //     CurrentWeapon->StopFire();
    //     AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponArmorySocketName);
    // }
    //
    // CurrentWeapon = Weapons[WeaponIndex];
    // AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponEquipSocketName);
    EquipAnimInprogress = true;
    PlayAnimMontage(EquipAnimMontage);
}


void USTUWeaponComponent::StartFire()
{
    if (!CanFire())
        return;
    CurrentWeapon->StartFire();
}

void USTUWeaponComponent::StopFire()
{
    if (!CanFire())
        return;
    CurrentWeapon->StopFire();
}

void USTUWeaponComponent::NextWeapon()
{
    if (!CanEquip())
        return;
    CurrentWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
    EquipWeapon();
}



void USTUWeaponComponent::PlayAnimMontage(UAnimMontage* Animation)
{
    ACharacter* Character = GetCharacter();
    if (!Character)
        return;

    Character->PlayAnimMontage(Animation);
}

void USTUWeaponComponent::InitAnimations()
{
    if (!EquipAnimMontage)
        return;

    const auto NotifyEvents = EquipAnimMontage->Notifies;
    for (auto NotifyEvent : NotifyEvents)
    {
        auto EquipFinishedNotify = Cast<USTUEquipFinishedAnimNotify>(NotifyEvent.Notify);
        auto ChangeWeapon = Cast<USTUChangeWeaponFinishAnimNotify>(NotifyEvent.Notify);

        if (EquipFinishedNotify)
        {
            EquipFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnEquipFinished);
        }
        else if (ChangeWeapon)
        {
            ChangeWeapon->OnWeaponNotified.AddUObject(this, &USTUWeaponComponent::OnWeaponChange);
        }
    }
}

void USTUWeaponComponent::OnEquipFinished(USkeletalMeshComponent* MeshComp)
{
    ACharacter* Character = GetCharacter();
    if (!Character || MeshComp != Character->GetMesh())
        return;

    EquipAnimInprogress = false;
}

bool USTUWeaponComponent::CanFire() const
{
    return CurrentWeapon && !EquipAnimInprogress;
}

bool USTUWeaponComponent::CanEquip() const
{
    return !EquipAnimInprogress;
}

ACharacter* USTUWeaponComponent::GetCharacter() const
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
        return false;
    return Character;
}

void USTUWeaponComponent::OnWeaponChange(USkeletalMeshComponent* MeshComp)
{
    if (CurrentWeaponIndex < 0 || CurrentWeaponIndex >= Weapons.Num())
    {
        UE_LOG(LogWeaponComponent, Display, TEXT("Invalid weapon index"));
        return;
    }

    ACharacter* Character = GetCharacter();
    if (!GetCharacter() || Character->GetMesh() != MeshComp)
        return;

    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
        AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponArmorySocketName);
    }

    CurrentWeapon = Weapons[CurrentWeaponIndex];
    AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponEquipSocketName);

    // CurrentWeapon = WeaponData.
    // CurrentReloadAnimMontage = WeaponData[CurrentWeaponIndex].ReloadAnimMontage;
    const auto CurrentWeaponData = WeaponData.FindByPredicate([&](const FWeaponData& Data){  return Data.WeaponClass == CurrentWeapon->GetClass(); });
    CurrentReloadAnimMontage = CurrentWeaponData ? CurrentWeaponData->ReloadAnimMontage : nullptr;
    
}

void USTUWeaponComponent::Reload()
{
    PlayAnimMontage(CurrentReloadAnimMontage);
}