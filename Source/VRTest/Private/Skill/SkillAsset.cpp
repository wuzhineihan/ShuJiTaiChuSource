// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/SkillAsset.h"

#include "Logging/LogMacros.h"

#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

void USkillAsset::PostLoad()
{
	Super::PostLoad();
	RebuildTrailToSkillCache();
}

#if WITH_EDITOR
void USkillAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FProperty* ChangedProperty = PropertyChangedEvent.Property;
	if (!ChangedProperty)
	{
		return;
	}

	// 只在编辑 StarDrawTrailPairs 时重建，避免无谓开销
	if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(USkillAsset, StarDrawTrailPairs))
	{
		RebuildTrailToSkillCache();
	}
}
#endif

ESkillType USkillAsset::GetSkillTypeFromTrail(const TArray<EStarDrawDirection>& Trail) const
{
	if (Trail.Num() == 0)
	{
		return ESkillType::None;
	}

	const FString Key = BuildTrailKey(Trail);
	if (Key.IsEmpty())
	{
		return ESkillType::None;
	}

	if (const ESkillType* Found = TrailToSkill.Find(Key))
	{
		return *Found;
	}

	return ESkillType::None;
}

FString USkillAsset::BuildTrailKey(const TArray<EStarDrawDirection>& Trail) const
{
	FString Key;
	Key.Reserve(Trail.Num());
	for (const EStarDrawDirection Dir : Trail)
	{
		Key += FString::FromInt(StarDrawDirectionToInt(Dir));
	}
	return Key;
}

void USkillAsset::RebuildTrailToSkillCache()
{
	TrailToSkill.Empty(StarDrawTrailPairs.Num());

	auto AddOrOverrideWithWarning = [this](const FString& Key, ESkillType Skill, int32 PairIndex, const TCHAR* KeyLabel)
	{
		if (const ESkillType* Existing = TrailToSkill.Find(Key))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("SkillAsset[%s]: duplicate %s key '%s' at StarDrawTrailPairs[%d]. Existing=%s, New=%s. New overrides old."),
				*GetName(),
				KeyLabel,
				*Key,
				PairIndex,
				*UEnum::GetValueAsString(*Existing),
				*UEnum::GetValueAsString(Skill));
		}

		TrailToSkill.Add(Key, Skill);
	};

	for (int32 Index = 0; Index < StarDrawTrailPairs.Num(); ++Index)
	{
		const FStarDrawTrailPair& Pair = StarDrawTrailPairs[Index];
		const FString ForwardKey = BuildTrailKey(Pair.Trail);
		if (ForwardKey.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillAsset[%s]: StarDrawTrailPairs[%d] has empty trail, ignored."), *GetName(), Index);
			continue;
		}

		// 正向写入
		AddOrOverrideWithWarning(ForwardKey, Pair.Skill, Index, TEXT("forward"));

		// 反向写入：reverse + opposite
		TArray<EStarDrawDirection> ReverseOppositeTrail;
		ReverseOppositeTrail.Reserve(Pair.Trail.Num());
		for (int32 i = Pair.Trail.Num() - 1; i >= 0; --i)
		{
			ReverseOppositeTrail.Add(StarDrawDirectionGetOpposite(Pair.Trail[i]));
		}
		const FString ReverseKey = BuildTrailKey(ReverseOppositeTrail);
		if (!ReverseKey.IsEmpty() && ReverseKey != ForwardKey)
		{
			AddOrOverrideWithWarning(ReverseKey, Pair.Skill, Index, TEXT("reverse"));
		}
	}
}
