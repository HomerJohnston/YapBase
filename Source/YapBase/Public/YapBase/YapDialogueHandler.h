#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "YapBase/YapTypes.h"
#include "YapDialogueHandler.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UYapDialogueHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * Something in the game that responds to Yap: usually its dialogue UI. Register it with UYapSubsystem::RegisterHandler.
 *
 * In Blueprint, implement the events. In C++, override the On... functions (don't call Super).
 * Speech, prompts and conversations each say which conversation they belong to; free speech has none.
 */
class YAPBASE_API IYapDialogueHandler
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Conversation Opening")
	void K2_ConversationOpening(const FYapConversationData& Conversation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Conversation Opened")
	void K2_ConversationOpened(const FYapConversationData& Conversation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Conversation Closing")
	void K2_ConversationClosing(const FYapConversationData& Conversation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Conversation Closed")
	void K2_ConversationClosed(const FYapConversationData& Conversation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Speech Begins")
	void K2_SpeechBegins(const FYapSpeechData& Speech);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Speech Ends")
	void K2_SpeechEnds(const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation, EYapSpeechResult Result);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Awaiting Advance")
	void K2_AwaitingAdvance(const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Prompt Entry Created")
	void K2_PromptEntryCreated(const FYapPromptEntryData& Entry);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Prompt Ready")
	void K2_PromptReady(const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Yap", DisplayName = "Prompt Ended")
	void K2_PromptEnded(const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation, int32 ChosenEntry);

public:
	/** A conversation is opening. Apply an opening interlock (UYapSubsystem) to hold it while your UI opens. */
	virtual void OnConversationOpening(const FYapConversationData& Conversation) { K2_ConversationOpening(Conversation); }
	virtual void OnConversationOpened(const FYapConversationData& Conversation) { K2_ConversationOpened(Conversation); }
	/** A conversation is closing. Apply a closing interlock to hold it while your UI closes. */
	virtual void OnConversationClosing(const FYapConversationData& Conversation) { K2_ConversationClosing(Conversation); }
	virtual void OnConversationClosed(const FYapConversationData& Conversation) { K2_ConversationClosed(Conversation); }
	/** A line begins: show its text, play its audio (from Speech.StartOffset). */
	virtual void OnSpeechBegins(const FYapSpeechData& Speech) { K2_SpeechBegins(Speech); }
	/** A line's speech has ended: take its text down, stop its audio if it was cut short. */
	virtual void OnSpeechEnds(const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation, EYapSpeechResult Result) { K2_SpeechEnds(Speech, Conversation, Result); }
	/** A conversation line is waiting for the player to advance (show a "continue" prompt). */
	virtual void OnAwaitingAdvance(const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation) { K2_AwaitingAdvance(Speech, Conversation); }
	/** One entry of a prompt: add it to your list of choices. */
	virtual void OnPromptEntryCreated(const FYapPromptEntryData& Entry) { K2_PromptEntryCreated(Entry); }
	/** Every entry of the prompt has been sent: show the choices. */
	virtual void OnPromptReady(const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation) { K2_PromptReady(Prompt, Conversation); }
	/** The prompt is done: an entry was chosen (its index), or it was taken away (INDEX_NONE). Take the choices down. */
	virtual void OnPromptEnded(const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation, int32 ChosenEntry) { K2_PromptEnded(Prompt, Conversation, ChosenEntry); }

	// Calling a handler, whether it's implemented in C++ or only in Blueprint.
	static void NotifyConversationOpening(UObject* Handler, const FYapConversationData& Conversation);
	static void NotifyConversationOpened(UObject* Handler, const FYapConversationData& Conversation);
	static void NotifyConversationClosing(UObject* Handler, const FYapConversationData& Conversation);
	static void NotifyConversationClosed(UObject* Handler, const FYapConversationData& Conversation);
	static void NotifySpeechBegins(UObject* Handler, const FYapSpeechData& Speech);
	static void NotifySpeechEnds(UObject* Handler, const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation, EYapSpeechResult Result);
	static void NotifyAwaitingAdvance(UObject* Handler, const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation);
	static void NotifyPromptEntryCreated(UObject* Handler, const FYapPromptEntryData& Entry);
	static void NotifyPromptReady(UObject* Handler, const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation);
	static void NotifyPromptEnded(UObject* Handler, const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation, int32 ChosenEntry);
};
