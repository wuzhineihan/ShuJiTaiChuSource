#include "AI/Animation/SacraWeaponAnimNotifies.h"

#include "AI/Component/SacraBowWeaponComponent.h"
#include "AI/Component/SacraMeleeWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"

FString USacraAnimNotify_CompleteEquip::GetNotifyName_Implementation() const
{
	return TEXT("SacraCompleteEquip");
}

void USacraAnimNotify_CompleteEquip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	if (USacraBowWeaponComponent* BowWeaponComponent = MeshComp->GetOwner()->FindComponentByClass<USacraBowWeaponComponent>())
	{
		BowWeaponComponent->CompleteEquipWeapon();
		return;
	}

	if (USacraMeleeWeaponComponent* MeleeWeaponComponent = MeshComp->GetOwner()->FindComponentByClass<USacraMeleeWeaponComponent>())
	{
		MeleeWeaponComponent->CompleteMeleeEquip();
	}
}

FString USacraAnimNotify_BowReleaseAttack::GetNotifyName_Implementation() const
{
	return TEXT("SacraBowReleaseAttack");
}

void USacraAnimNotify_BowReleaseAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	if (USacraBowWeaponComponent* BowWeaponComponent = MeshComp->GetOwner()->FindComponentByClass<USacraBowWeaponComponent>())
	{
		BowWeaponComponent->NotifyAttackRelease();
	}
}

FString USacraAnimNotify_MeleeAttackWindowBegin::GetNotifyName_Implementation() const
{
	return TEXT("SacraMeleeAttackWindowBegin");
}

void USacraAnimNotify_MeleeAttackWindowBegin::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	if (USacraMeleeWeaponComponent* MeleeWeaponComponent = MeshComp->GetOwner()->FindComponentByClass<USacraMeleeWeaponComponent>())
	{
		MeleeWeaponComponent->BeginAttackWindow();
	}
}

FString USacraAnimNotify_MeleeAttackWindowEnd::GetNotifyName_Implementation() const
{
	return TEXT("SacraMeleeAttackWindowEnd");
}

void USacraAnimNotify_MeleeAttackWindowEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	if (USacraMeleeWeaponComponent* MeleeWeaponComponent = MeshComp->GetOwner()->FindComponentByClass<USacraMeleeWeaponComponent>())
	{
		MeleeWeaponComponent->EndAttackWindow();
	}
}
