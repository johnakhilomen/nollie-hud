#pragma once

#include "CoreMinimal.h"
#include "NollieTypes.generated.h"

USTRUCT(BlueprintType)
struct NOLLIESTANDARD_API FNollieFactor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FString Label;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    int32 RawScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    float Weight = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    float Weighted = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FString KeyFinding;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FString Rationale;
};

USTRUCT(BlueprintType)
struct NOLLIESTANDARD_API FNollieVerdict
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FString Company;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    int32 CompositeScore = 0;

    /** "RED", "SUSPECT", or "GREEN" */
    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FString Verdict;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FNollieFactor GovernanceDisclosure;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FNollieFactor ProvenanceDebtRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FNollieFactor ShadowITSignal;

    UPROPERTY(BlueprintReadOnly, Category = "Nollie")
    FNollieFactor OperationalDrift;
};
