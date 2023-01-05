// Shoot Them Up Game, All Rights Reserved.


#include "Components/STUWeaponComponent.h"

#include <ThirdParty/CryptoPP/5.6.5/include/misc.h>

#include "Camera/CameraActor.h"
#include "Weapon/STUBaseWeapon.h"
#include "GameFramework/Character.h"
#include "Animations/STUEquipFinishedAnimNotify.h"
#include "Animations/STUChangeWeaponFinishAnimNotify.h"
#include "Animations/STUReloadFinishedAnimNotify.h"
#include "Animations/AnimUtils.h"


DEFINE_LOG_CATEGORY_STATIC(LogWeaponComponent, All, All);

constexpr static int32 WeaponNum = 2;

USTUWeaponComponent::USTUWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTUWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    checkf(WeaponData.Num() == WeaponNum, TEXT("Our character can hold only %i weapon items"), WeaponNum);

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

        Weapon->OnClipEmpty.AddUObject(this, &USTUWeaponComponent::ChangeClip);
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

    
    const auto CurrentWeaponData = WeaponData.FindByPredicate([&](const FWeaponData& Data){  //
        return Data.WeaponClass == CurrentWeapon->GetClass();                                         //
    });

    EquipAnimInprogress = false;
    AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponEquipSocketName);
    CurrentReloadAnimMontage = CurrentWeaponData ? CurrentWeaponData->ReloadAnimMontage : nullptr;
}

void USTUWeaponComponent::InitAnimations()
{
    auto EquipFinishedNotify = AnimUtils::FindNotifyByClass<USTUEquipFinishedAnimNotify>(EquipAnimMontage);
    auto ChangeWeaponFinishedNotify = AnimUtils::FindNotifyByClass<USTUChangeWeaponFinishAnimNotify>(EquipAnimMontage);
    
    if(ChangeWeaponFinishedNotify)
    {
        ChangeWeaponFinishedNotify->OnWeaponNotified.AddUObject(this, &USTUWeaponComponent::OnWeaponChange);
    }
    else if(EquipFinishedNotify)
    {
        EquipFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnEquipFinished);
    }
    else
    {
        UE_LOG(LogWeaponComponent, Error, TEXT("Equip anim notify is forgotten to set"));
        checkNoEntry();
    }

    for(auto OneWeaponData : WeaponData)
    {
        auto ReloadFinishedNotify = AnimUtils::FindNotifyByClass<USTUReloadFinishedAnimNotify>(OneWeaponData.ReloadAnimMontage);
        if(!ReloadFinishedNotify)
        {
            UE_LOG(LogWeaponComponent, Error, TEXT("Reload anim notify is forgotten to set"));
            checkNoEntry();
        }
        ReloadFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnReloadFinished);
    }

    // const auto NotifyEvents = EquipAnimMontage->Notifies;
    // for (auto NotifyEvent : NotifyEvents)
    // {
    //     auto EquipFinishedNotifyy = Cast<USTUEquipFinishedAnimNotify>(NotifyEvent.Notify);
    //     
    //     if(EquipFinishedNotifyy)
    //     {
    //         EquipFinishedNotifyy->OnNotified.AddUObject(this, &USTUWeaponComponent::OnEquipFinished);
    //     }
    //     else
    //     {
    //         UE_LOG(LogWeaponComponent, Error, TEXT("Equip anim notify is forgotten to set"));
    //     }
    // }

}


void USTUWeaponComponent::OnEquipFinished(USkeletalMeshComponent* MeshComp)
{
    ACharacter* Character = GetCharacter();
    if (!Character || MeshComp != Character->GetMesh())
        return;
    
    EquipAnimInprogress = false;

}

void USTUWeaponComponent::OnReloadFinished(USkeletalMeshComponent* MeshComp)
{
    UE_LOG(LogWeaponComponent, Error, TEXT("Reload"))
    ACharacter* Character = GetCharacter();
    if (!Character || MeshComp != Character->GetMesh())
        return;
    
    ReloadAnimInProgress = false;

}

bool USTUWeaponComponent::CanFire() const
{
    return CurrentWeapon && !EquipAnimInprogress && !ReloadAnimInProgress;
}

bool USTUWeaponComponent::CanEquip() const
{
    return !EquipAnimInprogress && !ReloadAnimInProgress;
}

ACharacter* USTUWeaponComponent::GetCharacter() const
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
        return false;
    return Character;
}

bool USTUWeaponComponent::CanReload() const
{
    return CurrentWeapon           //
    && !EquipAnimInprogress        //
    && !ReloadAnimInProgress       //
    && CurrentWeapon->CanReload(); //;
}

void USTUWeaponComponent::Reload()
{
    ChangeClip();
}

bool USTUWeaponComponent::GetCurrentWeaponUIData(FWeaponUIData& UIData) const
{
    if(CurrentWeapon)
    {
        UIData = CurrentWeapon->GetUIData();
        return true;
    }
    return false;
}

bool USTUWeaponComponent::GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const
{
    if(CurrentWeapon)
    {
        AmmoData = CurrentWeapon->GetAmmoData();
        return true;
    }
    return false;
}


void USTUWeaponComponent::OnEmptyClip()
{
    ChangeClip();
}

void USTUWeaponComponent::ChangeClip()
{
    if(!CanReload()) return;
    CurrentWeapon->StopFire();
    CurrentWeapon->ChangeClip();
    ReloadAnimInProgress = true;
    PlayAnimMontage(CurrentReloadAnimMontage);
}