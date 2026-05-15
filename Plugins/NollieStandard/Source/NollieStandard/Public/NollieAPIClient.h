#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "NollieTypes.h"
#include "NollieAPIClient.generated.h"

// ── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNollieOnVerdictReceived, const FNollieVerdict&, Verdict);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNollieOnCompaniesFetched, const TArray<FNollieVerdict>&, Companies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNollieOnFailed, const FString&, Error);

// ── Async node: Fetch single verdict ─────────────────────────────────────────

UCLASS()
class NOLLIESTANDARD_API UNollieFetchVerdict : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    /** Called when the verdict is successfully retrieved */
    UPROPERTY(BlueprintAssignable)
    FNollieOnVerdictReceived OnSuccess;

    /** Called when the request fails or company is not found */
    UPROPERTY(BlueprintAssignable)
    FNollieOnFailed OnFailure;

    /**
     * Fetch a verdict for a company from the Nollie Standard API.
     * @param CompanyName  e.g. "Disney", "Netflix", "Microsoft"
     */
    UFUNCTION(BlueprintCallable, Category = "Nollie Standard",
        meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
                DisplayName = "Fetch Verdict (Nollie)"))
    static UNollieFetchVerdict* FetchVerdict(UObject* WorldContextObject, const FString& CompanyName);

    virtual void Activate() override;

private:
    FString CompanyName;
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
    static FNollieFactor ParseFactor(const TSharedPtr<FJsonObject>& Obj);
};

// ── Async node: Fetch all companies ──────────────────────────────────────────

UCLASS()
class NOLLIESTANDARD_API UNollieFetchCompanies : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FNollieOnCompaniesFetched OnSuccess;

    UPROPERTY(BlueprintAssignable)
    FNollieOnFailed OnFailure;

    /** Fetch all pre-scored companies from the Nollie Standard API. */
    UFUNCTION(BlueprintCallable, Category = "Nollie Standard",
        meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
                DisplayName = "Fetch All Companies (Nollie)"))
    static UNollieFetchCompanies* FetchAllCompanies(UObject* WorldContextObject);

    virtual void Activate() override;

private:
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};

// ── Helper function library ───────────────────────────────────────────────────

UCLASS()
class NOLLIESTANDARD_API UNollieStandardLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Returns the display colour for a verdict string (RED/SUSPECT/GREEN). */
    UFUNCTION(BlueprintPure, Category = "Nollie Standard")
    static FLinearColor GetVerdictColor(const FString& Verdict);

    /** Returns true if the verdict struct has been populated (non-empty company). */
    UFUNCTION(BlueprintPure, Category = "Nollie Standard")
    static bool IsVerdictValid(const FNollieVerdict& Verdict);

    /** Converts a composite score (0-100) to a 0.0-1.0 float for progress bars. */
    UFUNCTION(BlueprintPure, Category = "Nollie Standard")
    static float ScoreToPercent(int32 Score);
};
