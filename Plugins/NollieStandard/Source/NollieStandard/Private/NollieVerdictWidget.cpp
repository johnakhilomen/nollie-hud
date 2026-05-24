#include "NollieVerdictWidget.h"
#include "NollieAPIClient.h"
#include "Components/EditableTextBox.h"

void UNollieVerdictWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Hide the verdict card until a result is loaded
    if (VerdictCard)
    {
        VerdictCard->SetVisibility(ESlateVisibility::Hidden);
    }

    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("Enter a company name to begin.")));
    }
}

void UNollieVerdictWidget::SearchForCompany(const FText& CompanyName)
{
    const FString Name = CompanyName.ToString().TrimStartAndEnd();
    if (Name.IsEmpty()) return;

    // Show loading state
    if (VerdictCard)   VerdictCard->SetVisibility(ESlateVisibility::Hidden);
    if (StatusText)    StatusText->SetText(FText::FromString(TEXT("Fetching verdict...")));

    // Fire the async API call
    UNollieFetchVerdict* Node = UNollieFetchVerdict::FetchVerdict(this, Name);

    Node->OnSuccess.AddDynamic(this, &UNollieVerdictWidget::DisplayVerdict);
    Node->OnFailure.AddDynamic(this, &UNollieVerdictWidget::DisplayError);
    Node->Activate();
}

void UNollieVerdictWidget::DisplayVerdict(const FNollieVerdict& Verdict)
{
    // Clear status
    if (StatusText) StatusText->SetText(FText::GetEmpty());

    // Company + score
    if (CompanyNameText)
        CompanyNameText->SetText(FText::FromString(Verdict.Company.ToUpper()));

    if (ScoreText)
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d / 100"), Verdict.CompositeScore)));

    if (ScoreBar)
        ScoreBar->SetPercent(UNollieStandardLibrary::ScoreToPercent(Verdict.CompositeScore));

    // Verdict badge
    const FLinearColor Color = UNollieStandardLibrary::GetVerdictColor(Verdict.Verdict);
    if (VerdictBadge)
        VerdictBadge->SetBrushColor(Color);

    if (VerdictText)
    {
        VerdictText->SetText(FText::FromString(Verdict.Verdict));
        VerdictText->SetColorAndOpacity(FSlateColor(Color));
    }

    // Score bar tint
    if (ScoreBar)
        ScoreBar->SetFillColorAndOpacity(Color);

    // Four factors
    SetFactor(GovernanceScore, GovernanceBar, GovernanceFinding, Verdict.GovernanceDisclosure);
    SetFactor(ProvenanceScore, ProvenanceBar, ProvenanceFinding, Verdict.ProvenanceDebtRatio);
    SetFactor(ShadowITScore,   ShadowITBar,   ShadowITFinding,   Verdict.ShadowITSignal);
    SetFactor(DriftScore,      DriftBar,      DriftFinding,      Verdict.OperationalDrift);

    // Show the card
    if (VerdictCard)
        VerdictCard->SetVisibility(ESlateVisibility::Visible);

    // Notify Blueprint for animations
    OnVerdictDisplayed(Verdict);
}

void UNollieVerdictWidget::DisplayError(const FString& Error)
{
    if (VerdictCard) VerdictCard->SetVisibility(ESlateVisibility::Hidden);
    if (StatusText)  StatusText->SetText(FText::FromString(Error));
}

void UNollieVerdictWidget::SetFactor(
    UTextBlock* ScoreTB,
    UProgressBar* Bar,
    UTextBlock* FindingTB,
    const FNollieFactor& Factor)
{
    if (ScoreTB)
        ScoreTB->SetText(FText::FromString(FString::Printf(TEXT("%d"), Factor.RawScore)));

    if (Bar)
    {
        Bar->SetPercent(UNollieStandardLibrary::ScoreToPercent(Factor.RawScore));
        Bar->SetFillColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.53f, 1.0f));
    }

    if (FindingTB)
        FindingTB->SetText(FText::FromString(Factor.KeyFinding));
}
