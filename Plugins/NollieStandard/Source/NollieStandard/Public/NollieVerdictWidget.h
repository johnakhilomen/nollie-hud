#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/EditableTextBox.h"
#include "Components/CanvasPanel.h"
#include "NollieTypes.h"
#include "NollieVerdictWidget.generated.h"

/**
 * Base class for the Nollie Standard HUD widget.
 * Create a Blueprint Widget (WBP_NollieHUD) inheriting from this class.
 * Name your UMG components exactly as the UPROPERTY names below.
 */
UCLASS(Blueprintable, BlueprintType)
class NOLLIESTANDARD_API UNollieVerdictWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    // ── Search ────────────────────────────────────────────────────────────────

    /** Text box the user types a company name into */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> SearchBox;

    // ── Verdict card ──────────────────────────────────────────────────────────

    /** Panel containing the verdict card — hidden until a result is loaded */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> VerdictCard;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CompanyNameText;

    /** Coloured border behind the verdict label */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> VerdictBadge;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> VerdictText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ScoreText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ScoreBar;

    // ── Factor rows ───────────────────────────────────────────────────────────

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> GovernanceScore;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> GovernanceBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> GovernanceFinding;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ProvenanceScore;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ProvenanceBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ProvenanceFinding;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ShadowITScore;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ShadowITBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ShadowITFinding;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DriftScore;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> DriftBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DriftFinding;

    // ── Status ────────────────────────────────────────────────────────────────

    /** Shown while a request is in-flight */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatusText;

    // ── Blueprint-callable API ────────────────────────────────────────────────

    /** Call this from Blueprint's OnTextCommitted event on SearchBox */
    UFUNCTION(BlueprintCallable, Category = "Nollie")
    void SearchForCompany(const FText& CompanyName);

    /** Called automatically after a successful API response */
    UFUNCTION(BlueprintCallable, Category = "Nollie")
    void DisplayVerdict(const FNollieVerdict& Verdict);

    /** Called on API failure */
    UFUNCTION(BlueprintCallable, Category = "Nollie")
    void DisplayError(const FString& Error);

    /** Blueprint event fired when a new verdict is displayed — hook up animations here */
    UFUNCTION(BlueprintImplementableEvent, Category = "Nollie")
    void OnVerdictDisplayed(const FNollieVerdict& Verdict);

protected:
    virtual void NativeConstruct() override;

private:
    void SetFactor(UTextBlock* ScoreTB, UProgressBar* Bar, UTextBlock* FindingTB, const FNollieFactor& Factor);
};
