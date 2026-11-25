#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "YapBase/YapTypes.h"
#include "YapSubsystem.generated.h"

class UYapBroker;
class UYapSpeechConfig;

DECLARE_MULTICAST_DELEGATE_TwoParams(FYapOnSpeechEnded, const FYapSpeechHandle& /*Speech*/, EYapSpeechResult /*Result*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FYapOnLineFinished, const FYapSpeechHandle& /*Speech*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FYapOnPromptEnded, const FYapPromptHandle& /*Prompt*/, int32 /*ChosenEntry*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FYapOnConversationStateChanged, const FYapConversationHandle& /*Conversation*/, EYapConversationState /*NewState*/);

/** A registered dialogue handler. */
USTRUCT()
struct FYapHandlerEntry
{
	GENERATED_BODY()

	TWeakObjectPtr<UObject> Handler;

	/** Only events of this speech config (and conversation events). None: every event. */
	FName ConfigName;
};

/** A line YapBase is running. */
USTRUCT()
struct FYapActiveLine
{
	GENERATED_BODY()

	/** What the game was told when it began. */
	UPROPERTY(Transient)
	FYapSpeechData Data;

	/** Seconds played (speech and padding). */
	float Elapsed = 0.0f;

	/** The speech's length (Data.SpeechTime), and the padding after it (may be negative, at least -SpeechTime). */
	float SpeechTime = 0.0f;
	float Padding = 0.0f;
	bool bTimed = true;
	bool bInterruptible = true;
	bool bAutoAdvance = true;

	/** Its speech is still playing. */
	bool bSpeaking = true;
	/** What comes after it may start. */
	bool bFinished = false;
	bool bAwaitingAdvance = false;
	/** Its integration doesn't need to ask about it again: forget it as soon as its speech ends. */
	bool bReleased = false;
};

USTRUCT()
struct FYapActivePrompt
{
	GENERATED_BODY()

	FYapPromptHandle Handle;
	FYapConversationHandle Conversation;
	int32 NumEntries = 0;
	/** INDEX_NONE until one is chosen. */
	int32 Chosen = INDEX_NONE;
};

USTRUCT()
struct FYapActiveConversation
{
	GENERATED_BODY()

	FYapConversationHandle Handle;
	FName Name;
	EYapConversationState State = EYapConversationState::Queued;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UObject>> OpeningLocks;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UObject>> ClosingLocks;
};

/**
 * Yap's speech engine, one per game world.
 *
 * Integrations (BangoYap, FlowYap, C++) decide what's said and in what order, and hand it to this one line, prompt or
 * conversation at a time; this plays them (speech time, padding, waiting for the player), sends the game's registered
 * handlers (IYapDialogueHandler) events, and takes the game's input back (AdvanceConversation, ChoosePromptEntry).
 *
 * Conversations open one at a time: one opened while another is open is queued until that one closes. Speech that
 * isn't in a conversation is free speech: it always moves on by itself, and the player can't advance it.
 */
UCLASS()
class YAPBASE_API UYapSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UYapSubsystem* Get(const UObject* WorldContext);

	// =========================================
	// The game: handlers and input
	// =========================================

	/** Registers a handler (an object implementing IYapDialogueHandler) for every event, or only for one speech config's (and conversation events). */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	void RegisterHandler(UObject* Handler, FName ConfigName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Yap")
	void UnregisterHandler(UObject* Handler);

	/** The player's "continue": ends the conversation's lines that may be cut short, and moves on from those waiting. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	void AdvanceConversation(const FYapConversationHandle& Conversation);

	/** The player's choice for a prompt (FYapPromptEntryData::EntryIndex). False if it isn't waiting for one, or the index is wrong. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	bool ChoosePromptEntry(const FYapPromptHandle& Prompt, int32 EntryIndex);

	/** Holds a conversation that's opening (call it from Conversation Opening), until the lock is released or destroyed. */
	UFUNCTION(BlueprintCallable, Category = "Yap", meta = (DefaultToSelf = "Lock"))
	bool ApplyOpeningInterlock(const FYapConversationHandle& Conversation, UObject* Lock);

	UFUNCTION(BlueprintCallable, Category = "Yap", meta = (DefaultToSelf = "Lock"))
	void ReleaseOpeningInterlock(const FYapConversationHandle& Conversation, UObject* Lock);

	/** Holds a conversation that's closing (call it from Conversation Closing), until the lock is released or destroyed. */
	UFUNCTION(BlueprintCallable, Category = "Yap", meta = (DefaultToSelf = "Lock"))
	bool ApplyClosingInterlock(const FYapConversationHandle& Conversation, UObject* Lock);

	UFUNCTION(BlueprintCallable, Category = "Yap", meta = (DefaultToSelf = "Lock"))
	void ReleaseClosingInterlock(const FYapConversationHandle& Conversation, UObject* Lock);

	/** Stops a line (a character who's been shot stops talking). What comes after it may start. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	bool CancelSpeech(const FYapSpeechHandle& Speech);

	/** Stops everything a character is saying. How many lines it stopped. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	int32 CancelSpeechBySpeaker(FName SpeakerID);

	UFUNCTION(BlueprintPure, Category = "Yap")
	EYapConversationState GetConversationState(const FYapConversationHandle& Conversation) const;

	/** The conversation that's open (or opening or closing). Unset if there's none. */
	UFUNCTION(BlueprintPure, Category = "Yap")
	FYapConversationHandle GetActiveConversation() const;

	UFUNCTION(BlueprintPure, Category = "Yap")
	bool IsSpeaking(const FYapSpeechHandle& Speech) const;

	// =========================================
	// Integrations
	// =========================================

	/** The speech config with this name; None: the project's default. Null if there's none (OutError says why). */
	UYapSpeechConfig* FindSpeechConfig(FName ConfigName, FString& OutError);

	/** The default config: the project's, or, if it can't be found, a config with every setting at its default. */
	UYapSpeechConfig* GetDefaultSpeechConfig();

	UYapBroker& GetBroker() const;

	/** Starts a line. False, with OutError, if the request is wrong (nothing is started then). */
	bool RunLine(const FYapLineRequest& Request, FYapSpeechHandle& OutSpeech, FString& OutError);

	EYapLineState GetLineState(const FYapSpeechHandle& Speech) const;

	/** Seconds the line has played (speech and padding), for saving it. Negative if it isn't running. */
	float GetLineElapsed(const FYapSpeechHandle& Speech) const;

	/** The integration won't ask about the line again: it's forgotten once its speech ends. */
	void ReleaseLine(const FYapSpeechHandle& Speech);

	/** Shows a prompt (it must be in an open conversation). False, with OutError, if the request is wrong. */
	bool StartPrompt(const FYapPromptRequest& Request, FYapPromptHandle& OutPrompt, FString& OutError);

	/** False if there's no such prompt. Otherwise OutChosen is the chosen entry, or INDEX_NONE while it's waiting. */
	bool GetPromptChoice(const FYapPromptHandle& Prompt, int32& OutChosen) const;

	/** Takes a waiting prompt away (Prompt Ended with INDEX_NONE), and forgets it. */
	void CancelPrompt(const FYapPromptHandle& Prompt);

	/** Forgets a prompt that's been chosen from. */
	void ReleasePrompt(const FYapPromptHandle& Prompt);

	/** Opens a conversation, or queues it if another one is open. */
	FYapConversationHandle OpenConversation(FName Name);

	/**
	 * Closes a conversation (a queued one is dropped). Its lines and prompts are stopped. With bImmediate, closing
	 * interlocks are ignored (the script running it was stopped).
	 */
	void CloseConversation(const FYapConversationHandle& Conversation, bool bImmediate = false);

	FYapOnSpeechEnded OnSpeechEnded;
	FYapOnLineFinished OnLineFinished;
	FYapOnPromptEnded OnPromptEnded;
	FYapOnConversationStateChanged OnConversationStateChanged;

	// =========================================
	// Subsystem
	// =========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	FYapActiveLine* FindLine(const FYapSpeechHandle& Speech);
	const FYapActiveLine* FindLine(const FYapSpeechHandle& Speech) const;
	FYapActivePrompt* FindPrompt(const FYapPromptHandle& Prompt);
	int32 FindConversationIndex(const FYapConversationHandle& Conversation) const;

	/** Moves a line on: its speech ending, its padding, waiting; forgets it when it's done. */
	void UpdateLine(const FYapSpeechHandle& Speech);
	void EndSpeech(const FYapSpeechHandle& Speech, EYapSpeechResult Result);
	void FinishLine(const FYapSpeechHandle& Speech);
	void RemoveLineIfDone(const FYapSpeechHandle& Speech);

	void StartOpeningNext();
	void TryFinishOpening(const FYapConversationHandle& Conversation);
	void TryFinishClosing(const FYapConversationHandle& Conversation, bool bForce);
	void StopConversationContents(const FYapConversationHandle& Conversation);
	void SetConversationState(FYapActiveConversation& Conversation, EYapConversationState NewState);

	/** Calls Func for each registered handler that wants an event of Config (null: every handler). */
	void ForEachHandler(const UYapSpeechConfig* Config, TFunctionRef<void(UObject*)> Func);

	UPROPERTY(Transient)
	TObjectPtr<UYapBroker> Broker;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UYapSpeechConfig>> Configs;

	UPROPERTY(Transient)
	TObjectPtr<UYapSpeechConfig> FallbackConfig;

	UPROPERTY(Transient)
	TArray<FYapHandlerEntry> Handlers;

	UPROPERTY(Transient)
	TArray<FYapActiveLine> Lines;

	/** Lines that are done but not released yet, with the world time they finished (forgotten after a while). */
	TMap<FYapSpeechHandle, double> FinishedLines;

	UPROPERTY(Transient)
	TArray<FYapActivePrompt> Prompts;

	/** The open (or opening or closing) conversation first, then the queued ones in order. */
	UPROPERTY(Transient)
	TArray<FYapActiveConversation> Conversations;
};
