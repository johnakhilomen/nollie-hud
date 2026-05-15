#include "NollieAPIClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformHttp.h"

static const FString NollieAPIBase = TEXT("https://gi8u814nqf.execute-api.us-east-1.amazonaws.com/dev");

// ── Helpers ───────────────────────────────────────────────────────────────────

FNollieFactor UNollieFetchVerdict::ParseFactor(const TSharedPtr<FJsonObject>& Obj)
{
    FNollieFactor Factor;
    if (!Obj.IsValid()) return Factor;

    Obj->TryGetStringField(TEXT("label"),       Factor.Label);
    Obj->TryGetStringField(TEXT("key_finding"), Factor.KeyFinding);
    Obj->TryGetStringField(TEXT("rationale"),   Factor.Rationale);

    int32 Score = 0;
    Obj->TryGetNumberField(TEXT("raw_score"), Score);
    Factor.RawScore = Score;

    double W = 0.0, Wt = 0.0;
    Obj->TryGetNumberField(TEXT("weight"),   W);
    Obj->TryGetNumberField(TEXT("weighted"), Wt);
    Factor.Weight   = (float)W;
    Factor.Weighted = (float)Wt;

    return Factor;
}

// ── UNollieFetchVerdict ───────────────────────────────────────────────────────

UNollieFetchVerdict* UNollieFetchVerdict::FetchVerdict(UObject* WorldContextObject, const FString& CompanyName)
{
    UNollieFetchVerdict* Node = NewObject<UNollieFetchVerdict>();
    Node->CompanyName = CompanyName;
    return Node;
}

void UNollieFetchVerdict::Activate()
{
    const FString URL = FString::Printf(
        TEXT("%s/verdict?company=%s"),
        *NollieAPIBase,
        *FGenericPlatformHttp::UrlEncode(CompanyName)
    );

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->OnProcessRequestComplete().BindUObject(this, &UNollieFetchVerdict::OnResponseReceived);
    Request->ProcessRequest();
}

void UNollieFetchVerdict::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        OnFailure.Broadcast(TEXT("Network error — check your connection."));
        return;
    }

    if (Response->GetResponseCode() == 404)
    {
        OnFailure.Broadcast(FString::Printf(TEXT("No verdict found for '%s'. Run the pipeline first."), *CompanyName));
        return;
    }

    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
    {
        OnFailure.Broadcast(TEXT("Failed to parse API response."));
        return;
    }

    FNollieVerdict Verdict;
    JsonObj->TryGetStringField(TEXT("company"), Verdict.Company);
    JsonObj->TryGetStringField(TEXT("verdict"), Verdict.Verdict);

    int32 Score = 0;
    JsonObj->TryGetNumberField(TEXT("composite_score"), Score);
    Verdict.CompositeScore = Score;

    const TSharedPtr<FJsonObject>* FactorsObj;
    if (JsonObj->TryGetObjectField(TEXT("factors"), FactorsObj))
    {
        const TSharedPtr<FJsonObject>* F;
        if ((*FactorsObj)->TryGetObjectField(TEXT("governance_disclosure"),   F)) Verdict.GovernanceDisclosure  = ParseFactor(*F);
        if ((*FactorsObj)->TryGetObjectField(TEXT("provenance_debt_ratio"),   F)) Verdict.ProvenanceDebtRatio   = ParseFactor(*F);
        if ((*FactorsObj)->TryGetObjectField(TEXT("shadow_it_signal"),        F)) Verdict.ShadowITSignal        = ParseFactor(*F);
        if ((*FactorsObj)->TryGetObjectField(TEXT("operational_drift"),       F)) Verdict.OperationalDrift      = ParseFactor(*F);
    }

    OnSuccess.Broadcast(Verdict);
}

// ── UNollieFetchCompanies ─────────────────────────────────────────────────────

UNollieFetchCompanies* UNollieFetchCompanies::FetchAllCompanies(UObject* WorldContextObject)
{
    return NewObject<UNollieFetchCompanies>();
}

void UNollieFetchCompanies::Activate()
{
    const FString URL = FString::Printf(TEXT("%s/companies"), *NollieAPIBase);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->OnProcessRequestComplete().BindUObject(this, &UNollieFetchCompanies::OnResponseReceived);
    Request->ProcessRequest();
}

void UNollieFetchCompanies::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        OnFailure.Broadcast(TEXT("Network error."));
        return;
    }

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonArray))
    {
        OnFailure.Broadcast(TEXT("Failed to parse companies list."));
        return;
    }

    TArray<FNollieVerdict> Companies;
    for (const TSharedPtr<FJsonValue>& Val : JsonArray)
    {
        const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
        if (!Obj.IsValid()) continue;

        FNollieVerdict V;
        Obj->TryGetStringField(TEXT("company"), V.Company);
        Obj->TryGetStringField(TEXT("verdict"), V.Verdict);

        int32 Score = 0;
        Obj->TryGetNumberField(TEXT("composite_score"), Score);
        V.CompositeScore = Score;

        Companies.Add(V);
    }

    OnSuccess.Broadcast(Companies);
}

// ── UNollieStandardLibrary ────────────────────────────────────────────────────

FLinearColor UNollieStandardLibrary::GetVerdictColor(const FString& Verdict)
{
    if (Verdict == TEXT("GREEN"))   return FLinearColor(0.0f,  1.0f,  0.53f, 1.0f);
    if (Verdict == TEXT("SUSPECT")) return FLinearColor(0.96f, 0.62f, 0.04f, 1.0f);
    if (Verdict == TEXT("RED"))     return FLinearColor(0.94f, 0.27f, 0.27f, 1.0f);
    return FLinearColor(0.29f, 0.38f, 0.47f, 1.0f);
}

bool UNollieStandardLibrary::IsVerdictValid(const FNollieVerdict& Verdict)
{
    return !Verdict.Company.IsEmpty();
}

float UNollieStandardLibrary::ScoreToPercent(int32 Score)
{
    return FMath::Clamp(Score / 100.0f, 0.0f, 1.0f);
}
