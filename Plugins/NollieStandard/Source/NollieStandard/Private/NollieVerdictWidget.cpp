#include "NollieVerdictWidget.h"
#include "NollieAPIClient.h"
#include "Components/EditableTextBox.h"
#include "Components/CanvasPanelSlot.h"

// ── Layout helpers ────────────────────────────────────────────────────────────

static void SetSlot(UWidget* W, float X, float Y, float SizeX, float SizeY)
{
    if (!W) return;
    if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(W->Slot))
    {
        S->SetPosition(FVector2D(X, Y));
        S->SetSize(FVector2D(SizeX, SizeY));
    }
}

static void StyleText(UTextBlock* T, FLinearColor Color, float Size = 16.f)
{
    if (!T) return;
    T->SetColorAndOpacity(FSlateColor(Color));
    FSlateFontInfo Font = T->GetFont();
    Font.Size = Size;
    T->SetFont(Font);
}

// ── Widget colours ────────────────────────────────────────────────────────────
namespace NC
{
    static const FLinearColor White    { 1.f, 1.f,  1.f,  1.f };
    static const FLinearColor Yellow   { 1.f, 0.85f,0.f,  1.f };
    static const FLinearColor Dim      { 0.7f,0.7f, 0.7f, 1.f };
    static const FLinearColor DarkBg   { 0.05f,0.05f,0.08f,0.92f };
    static const FLinearColor Green    { 0.f, 1.f,  0.53f,1.f };
}

// ── NativeConstruct ───────────────────────────────────────────────────────────

void UNollieVerdictWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ── Root widget positions (canvas coords, 1280×720 baseline) ──────────────

    // Search box — top-left
    SetSlot(SearchBox,      20,  20, 420,  44);

    // Status line — just below search box
    SetSlot(StatusText,     20,  72, 420,  28);

    // Verdict card — below status, full panel
    SetSlot(VerdictCard,    20, 108, 620, 420);

    // ── Inside VerdictCard ────────────────────────────────────────────────────

    SetSlot(CompanyNameText, 16,  12, 400,  48);
    SetSlot(VerdictBadge,    16,  66, 160,  36);
    SetSlot(VerdictText,      0,   0, 160,  36);   // inside Badge (Border child)
    SetSlot(ScoreText,       190,  66, 140,  36);
    SetSlot(ScoreBar,         16, 110, 584,  14);

    // Factor rows: Governance | Provenance | ShadowIT | Drift
    // Each row: Score(50) | Bar(120) | Finding(380)  spaced 60px apart
    const float RowY0   = 136.f;
    const float RowH    =  58.f;
    const float ScoreW  =  50.f;
    const float BarW    = 120.f;
    const float FindW   = 360.f;
    const float BarX    =  74.f;
    const float FindX   = 202.f;

    auto Row = [&](UTextBlock* Score, UProgressBar* Bar, UTextBlock* Finding, int Idx)
    {
        float Y = RowY0 + Idx * RowH;
        SetSlot(Score,   16,       Y,       ScoreW, 20);
        SetSlot(Bar,     BarX,     Y + 24,  BarW,   12);
        SetSlot(Finding, FindX,    Y,       FindW,  44);
    };

    Row(GovernanceScore, GovernanceBar, GovernanceFinding, 0);
    Row(ProvenanceScore, ProvenanceBar, ProvenanceFinding, 1);
    Row(ShadowITScore,   ShadowITBar,   ShadowITFinding,   2);
    Row(DriftScore,      DriftBar,      DriftFinding,       3);

    // ── Text colours & sizes ──────────────────────────────────────────────────

    StyleText(StatusText,       NC::Yellow,  15.f);
    StyleText(CompanyNameText,  NC::White,   26.f);
    StyleText(VerdictText,      NC::White,   18.f);
    StyleText(ScoreText,        NC::White,   20.f);
    StyleText(GovernanceScore,  NC::Dim,     14.f);
    StyleText(ProvenanceScore,  NC::Dim,     14.f);
    StyleText(ShadowITScore,    NC::Dim,     14.f);
    StyleText(DriftScore,       NC::Dim,     14.f);
    StyleText(GovernanceFinding,NC::White,   13.f);
    StyleText(ProvenanceFinding,NC::White,   13.f);
    StyleText(ShadowITFinding,  NC::White,   13.f);
    StyleText(DriftFinding,     NC::White,   13.f);

    // ── Card background ───────────────────────────────────────────────────────
    if (VerdictBadge)
        VerdictBadge->SetBrushColor(NC::DarkBg);  // dark panel until verdict loads

    // ── Initial state ─────────────────────────────────────────────────────────
    if (VerdictCard)
        VerdictCard->SetVisibility(ESlateVisibility::Hidden);

    if (StatusText)
        StatusText->SetText(FText::FromString(TEXT("Enter a company name and press Enter.")));
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
