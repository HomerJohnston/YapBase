#include "YapBase/YapSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "YapBase/YapBaseLog.h"
#include "YapBase/YapBroker.h"
#include "YapBase/YapAssets.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapCharacterManager.h"
#include "YapBase/YapDialogueHandler.h"
#include "YapBase/YapSettings.h"
#include "YapBase/YapSpeechConfig.h"

namespace YapSubsystemPrivate
{
	/** How long a finished line that nobody released is remembered. */
	static constexpr double FinishedLineMemorySeconds = 10.0;

	static bool UsesChildSafe(const UYapBroker& Broker, const UYapSpeechConfig& Config)
	{
		return !Config.bDisableChildSafe && Broker.GetMaturitySetting() == EYapMaturitySetting::ChildSafe;
	}

	/** Looks up a character for a request: None is fine (no character); an ID that isn't found is an error. */
	static bool LookUpCharacter(UYapCharacterManager* Manager, FName ID, const TCHAR* Role, UYapCharacterAsset*& OutCharacter, AActor*& OutActor, FString& OutError)
	{
		OutCharacter = nullptr;
		OutActor = nullptr;

		if (ID.IsNone())
		{
			return true;
		}

		if (!Manager)
		{
			OutError = TEXT("There's no Yap character manager in this world.");
			return false;
		}

		FString Error;
		OutCharacter = Manager->LookUpCharacter(ID, Error);
		if (!OutCharacter)
		{
			OutError = FString::Printf(TEXT("%s: %s"), Role, *Error);
			return false;
		}

		OutActor = Manager->FindActor(ID);
		return true;
	}
}

// ================================================================================================

UYapSubsystem* UYapSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UYapSubsystem>() : nullptr;
}

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UYapCharacterManager>();

	TSubclassOf<UYapBroker> BrokerClass = GetDefault<UYapSettings>()->BrokerClass.LoadSynchronous();
	if (!BrokerClass)
	{
		BrokerClass = UYapBroker::StaticClass();
	}
	Broker = NewObject<UYapBroker>(this, BrokerClass);
}

void UYapSubsystem::Deinitialize()
{
	Lines.Reset();
	FinishedLines.Reset();
	Prompts.Reset();
	Conversations.Reset();
	Handlers.Reset();

	Super::Deinitialize();
}

UYapBroker& UYapSubsystem::GetBroker() const
{
	check(Broker);
	return *Broker;
}

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UYapSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UYapSubsystem, STATGROUP_Tickables);
}

// ================================================================================================
// Handlers

void UYapSubsystem::RegisterHandler(UObject* Handler, FName ConfigName)
{
	if (!Handler || !Handler->GetClass()->ImplementsInterface(UYapDialogueHandler::StaticClass()))
	{
		UE_LOG(LogYapBase, Error, TEXT("RegisterHandler: %s doesn't implement the Yap Dialogue Handler interface."), *GetNameSafe(Handler));
		return;
	}

	for (const FYapHandlerEntry& Entry : Handlers)
	{
		if (Entry.Handler.Get() == Handler && Entry.ConfigName == ConfigName)
		{
			return;
		}
	}

	FYapHandlerEntry& Entry = Handlers.AddDefaulted_GetRef();
	Entry.Handler = Handler;
	Entry.ConfigName = ConfigName;
}

void UYapSubsystem::UnregisterHandler(UObject* Handler)
{
	Handlers.RemoveAll([Handler](const FYapHandlerEntry& Entry) { return !Entry.Handler.IsValid() || Entry.Handler.Get() == Handler; });
}

void UYapSubsystem::ForEachHandler(const UYapSpeechConfig* Config, TFunctionRef<void(UObject*)> Func)
{
	const FName ConfigName = Config ? Config->GetConfigName() : NAME_None;

	// A handler may register or unregister handlers while it's called.
	const TArray<FYapHandlerEntry> Copy = Handlers;
	for (const FYapHandlerEntry& Entry : Copy)
	{
		UObject* Handler = Entry.Handler.Get();
		if (!Handler)
		{
			continue;
		}
		if (Config && !Entry.ConfigName.IsNone() && Entry.ConfigName != ConfigName)
		{
			continue;
		}
		Func(Handler);
	}
}

// ================================================================================================
// Configs

UYapSpeechConfig* UYapSubsystem::FindSpeechConfig(FName ConfigName, FString& OutError)
{
	const bool bDefault = ConfigName.IsNone();
	const FName Name = bDefault ? GetDefault<UYapSettings>()->DefaultSpeechConfig : ConfigName;

	if (const TObjectPtr<UYapSpeechConfig>* Cached = Configs.Find(Name))
	{
		return *Cached;
	}

	UYapSpeechConfig* Config = Cast<UYapSpeechConfig>(Yap::FindNamedAsset(UYapSpeechConfig::GetPrimaryAssetType(), UYapSpeechConfig::StaticClass(), Name));
	if (Config)
	{
		Configs.Add(Name, Config);
		return Config;
	}

	if (bDefault)
	{
		return GetDefaultSpeechConfig();
	}

	OutError = FString::Printf(TEXT("There's no speech config '%s': no Yap speech config asset has that name."), *ConfigName.ToString());
	return nullptr;
}

UYapSpeechConfig* UYapSubsystem::GetDefaultSpeechConfig()
{
	const FName Name = GetDefault<UYapSettings>()->DefaultSpeechConfig;

	if (const TObjectPtr<UYapSpeechConfig>* Cached = Configs.Find(Name))
	{
		return *Cached;
	}

	if (!Name.IsNone())
	{
		if (UYapSpeechConfig* Config = Cast<UYapSpeechConfig>(Yap::FindNamedAsset(UYapSpeechConfig::GetPrimaryAssetType(), UYapSpeechConfig::StaticClass(), Name)))
		{
			Configs.Add(Name, Config);
			return Config;
		}
	}

	if (!FallbackConfig)
	{
		UE_LOG(LogYapBase, Warning, TEXT("The default speech config '%s' (Project Settings > Plugins > Yap) wasn't found, so every setting is at its default."), *Name.ToString());
		FallbackConfig = NewObject<UYapSpeechConfig>(this, TEXT("FallbackSpeechConfig"));
	}
	return FallbackConfig;
}

// ================================================================================================
// Lines

FYapActiveLine* UYapSubsystem::FindLine(const FYapSpeechHandle& Speech)
{
	return Lines.FindByPredicate([&Speech](const FYapActiveLine& Line) { return Line.Data.Handle == Speech; });
}

const FYapActiveLine* UYapSubsystem::FindLine(const FYapSpeechHandle& Speech) const
{
	return Lines.FindByPredicate([&Speech](const FYapActiveLine& Line) { return Line.Data.Handle == Speech; });
}

bool UYapSubsystem::RunLine(const FYapLineRequest& Request, FYapSpeechHandle& OutSpeech, FString& OutError)
{
	using namespace YapSubsystemPrivate;

	OutSpeech.Invalidate();

	UYapSpeechConfig* Config = Request.Config ? Request.Config.Get() : GetDefaultSpeechConfig();
	const bool bInConversation = Request.Conversation.IsValid();

	if (bInConversation && GetConversationState(Request.Conversation) != EYapConversationState::Open)
	{
		OutError = TEXT("The line's conversation isn't open.");
		return false;
	}

	FYapActiveLine Line;
	FYapSpeechData& Data = Line.Data;
	Data.Handle = FYapSpeechHandle::Make();
	Data.Conversation = Request.Conversation;
	Data.Config = Config;
	Data.SpeakerID = Request.Speaker;
	Data.DirectedAtID = Request.DirectedAt;
	Data.LineID = Request.LineID;
	Data.TitleText = Request.TitleText;

	UYapCharacterManager* Characters = UYapCharacterManager::Get(this);
	UYapCharacterAsset* Speaker = nullptr;
	UYapCharacterAsset* DirectedAt = nullptr;
	AActor* SpeakerActor = nullptr;
	AActor* DirectedAtActor = nullptr;
	if (!LookUpCharacter(Characters, Request.Speaker, TEXT("Speaker"), Speaker, SpeakerActor, OutError)
		|| !LookUpCharacter(Characters, Request.DirectedAt, TEXT("Directed at"), DirectedAt, DirectedAtActor, OutError))
	{
		return false;
	}
	Data.Speaker = Speaker;
	Data.DirectedAt = DirectedAt;
	Data.SpeakerActor = SpeakerActor;
	Data.DirectedAtActor = DirectedAtActor;

	if (!Config->ResolveMood(Request.Mood, Data.Mood, OutError))
	{
		return false;
	}

	// Text and audio, for the player's maturity setting
	const bool bChildSafe = UsesChildSafe(*Broker, *Config) && !Request.ChildSafeText.IsEmpty();
	Data.Text = bChildSafe ? Request.ChildSafeText : Request.MatureText;

	if (!Config->bDisableAudio && !Request.LineID.IsNone())
	{
		const FName AudioID = bChildSafe ? FName(*(Request.LineID.ToString() + GetDefault<UYapSettings>()->ChildSafeKeySuffix)) : Request.LineID;
		Data.Audio = Broker->FindLineAudio(AudioID, bChildSafe ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature, Config);
	}

	// Time
	EYapTimeMode TimeMode = Request.TimeMode == EYapTimeMode::Default ? Config->GetDefaultTimeMode() : Request.TimeMode;
	if (TimeMode == EYapTimeMode::Audio && !Data.Audio)
	{
		TimeMode = EYapTimeMode::Text;
	}

	switch (TimeMode)
	{
		case EYapTimeMode::Text:
		{
			const int32 Words = Broker->CalculateWordCount(Data.Text);
			Line.SpeechTime = FMath::Max(Broker->CalculateTextTime(Words, Data.Text.ToString().Len(), Config), Config->MinimumTextTime);
			break;
		}
		case EYapTimeMode::Audio:
		{
			Line.SpeechTime = FMath::Max(Broker->GetAudioDuration(Data.Audio), Config->MinimumAudioTime);
			break;
		}
		case EYapTimeMode::Manual:
		{
			if (!Request.bHasPadding)
			{
				OutError = TEXT("A line with manual time needs its time, given as its padding.");
				return false;
			}
			Line.SpeechTime = FMath::Max(Request.Padding, 0.0f);
			break;
		}
		case EYapTimeMode::None:
		{
			if (!bInConversation)
			{
				OutError = TEXT("A line with no time (time mode None) plays until the player advances it, so it must be in a conversation.");
				return false;
			}
			Line.bTimed = false;
			break;
		}
		default:
		{
			OutError = FString::Printf(TEXT("Unknown time mode %d."), static_cast<int32>(TimeMode));
			return false;
		}
	}

	if (Line.bTimed)
	{
		Line.SpeechTime = FMath::Max(Line.SpeechTime, Config->MinimumSpeakingTime);
	}

	if (TimeMode == EYapTimeMode::Manual || TimeMode == EYapTimeMode::None)
	{
		Line.Padding = 0.0f;
	}
	else
	{
		Line.Padding = FMath::Max(Request.bHasPadding ? Request.Padding : Config->DefaultPadding, -Line.SpeechTime);
	}

	// Interruptible and advance: conversations only. Free speech always moves on, and code can always stop it.
	if (bInConversation)
	{
		Line.bInterruptible = Request.Interruptible == EYapToggle::Default ? Config->bInterruptible : Request.Interruptible == EYapToggle::Yes;
		Line.bAutoAdvance = Request.AutoAdvance == EYapToggle::Default ? Config->bAutoAdvance : Request.AutoAdvance == EYapToggle::Yes;
		if (!Line.bTimed)
		{
			Line.bAutoAdvance = false;
		}
	}

	Data.SpeechTime = Line.bTimed ? Line.SpeechTime : 0.0f;
	Data.bTimed = Line.bTimed;
	Data.bInterruptible = Line.bInterruptible;
	Data.bAutoAdvance = Line.bAutoAdvance;
	Data.StartOffset = FMath::Max(Request.StartOffset, 0.0f);
	Line.Elapsed = Data.StartOffset;

	// A speaker says one thing at a time, unless the config allows more.
	if (!Config->bPermitOverlappingSpeech && !Request.Speaker.IsNone())
	{
		TArray<FYapSpeechHandle> ToCancel;
		for (const FYapActiveLine& Other : Lines)
		{
			if (Other.bSpeaking && Other.Data.SpeakerID == Request.Speaker)
			{
				ToCancel.Add(Other.Data.Handle);
			}
		}
		for (const FYapSpeechHandle& Other : ToCancel)
		{
			CancelSpeech(Other);
		}
	}

	// Carrying on after a load, its speech may be over already.
	const bool bAlreadySpoken = Line.bTimed && Line.Elapsed >= Line.SpeechTime;
	Line.bSpeaking = !bAlreadySpoken;

	OutSpeech = Data.Handle;
	const FYapSpeechData DataCopy = Data;
	Lines.Add(MoveTemp(Line));

	if (!bAlreadySpoken)
	{
		ForEachHandler(Config, [&DataCopy](UObject* Handler) { IYapDialogueHandler::NotifySpeechBegins(Handler, DataCopy); });
	}

	UpdateLine(OutSpeech);
	return true;
}

void UYapSubsystem::UpdateLine(const FYapSpeechHandle& Speech)
{
	FYapActiveLine* Line = FindLine(Speech);
	if (!Line)
	{
		return;
	}

	if (Line->bSpeaking && Line->bTimed && Line->Elapsed >= Line->SpeechTime)
	{
		EndSpeech(Speech, EYapSpeechResult::Normal);
		Line = FindLine(Speech);
		if (!Line)
		{
			return;
		}
	}

	if (!Line->bFinished)
	{
		if (!Line->bTimed)
		{
			// Plays until advanced.
			if (!Line->bAwaitingAdvance)
			{
				Line->bAwaitingAdvance = true;
				const FYapConversationHandle Conversation = Line->Data.Conversation;
				ForEachHandler(Line->Data.Config, [&](UObject* Handler) { IYapDialogueHandler::NotifyAwaitingAdvance(Handler, Speech, Conversation); });
			}
		}
		else if (Line->bAutoAdvance)
		{
			if (Line->Elapsed >= FMath::Max(Line->SpeechTime + Line->Padding, 0.0f))
			{
				FinishLine(Speech);
			}
		}
		else if (!Line->bAwaitingAdvance && Line->Elapsed >= FMath::Max(Line->SpeechTime, Line->SpeechTime + Line->Padding))
		{
			Line->bAwaitingAdvance = true;
			const FYapConversationHandle Conversation = Line->Data.Conversation;
			ForEachHandler(Line->Data.Config, [&](UObject* Handler) { IYapDialogueHandler::NotifyAwaitingAdvance(Handler, Speech, Conversation); });
		}
	}

	RemoveLineIfDone(Speech);
}

void UYapSubsystem::EndSpeech(const FYapSpeechHandle& Speech, EYapSpeechResult Result)
{
	FYapActiveLine* Line = FindLine(Speech);
	if (!Line || !Line->bSpeaking)
	{
		return;
	}

	Line->bSpeaking = false;
	const FYapConversationHandle Conversation = Line->Data.Conversation;
	UYapSpeechConfig* Config = Line->Data.Config;

	ForEachHandler(Config, [&](UObject* Handler) { IYapDialogueHandler::NotifySpeechEnds(Handler, Speech, Conversation, Result); });
	OnSpeechEnded.Broadcast(Speech, Result);
}

void UYapSubsystem::FinishLine(const FYapSpeechHandle& Speech)
{
	FYapActiveLine* Line = FindLine(Speech);
	if (!Line || Line->bFinished)
	{
		return;
	}

	Line->bFinished = true;
	Line->bAwaitingAdvance = false;
	OnLineFinished.Broadcast(Speech);
}

void UYapSubsystem::RemoveLineIfDone(const FYapSpeechHandle& Speech)
{
	const int32 Index = Lines.IndexOfByPredicate([&Speech](const FYapActiveLine& Line) { return Line.Data.Handle == Speech; });
	if (Index == INDEX_NONE)
	{
		return;
	}

	const FYapActiveLine& Line = Lines[Index];
	if (!Line.bFinished || Line.bSpeaking)
	{
		return;
	}

	if (!Line.bReleased)
	{
		const UWorld* World = GetWorld();
		FinishedLines.Add(Speech, World ? World->GetTimeSeconds() : 0.0);
	}
	Lines.RemoveAt(Index);
}

EYapLineState UYapSubsystem::GetLineState(const FYapSpeechHandle& Speech) const
{
	if (const FYapActiveLine* Line = FindLine(Speech))
	{
		if (Line->bFinished)
		{
			return EYapLineState::Finished;
		}
		return Line->bAwaitingAdvance ? EYapLineState::AwaitingAdvance : EYapLineState::Running;
	}

	return FinishedLines.Contains(Speech) ? EYapLineState::Finished : EYapLineState::Unknown;
}

float UYapSubsystem::GetLineElapsed(const FYapSpeechHandle& Speech) const
{
	const FYapActiveLine* Line = FindLine(Speech);
	return Line ? Line->Elapsed : -1.0f;
}

void UYapSubsystem::ReleaseLine(const FYapSpeechHandle& Speech)
{
	FinishedLines.Remove(Speech);

	if (FYapActiveLine* Line = FindLine(Speech))
	{
		Line->bReleased = true;
	}
}

bool UYapSubsystem::IsSpeaking(const FYapSpeechHandle& Speech) const
{
	const FYapActiveLine* Line = FindLine(Speech);
	return Line && Line->bSpeaking;
}

bool UYapSubsystem::CancelSpeech(const FYapSpeechHandle& Speech)
{
	if (!FindLine(Speech))
	{
		return false;
	}

	EndSpeech(Speech, EYapSpeechResult::Cancelled);
	FinishLine(Speech);
	RemoveLineIfDone(Speech);
	return true;
}

int32 UYapSubsystem::CancelSpeechBySpeaker(FName SpeakerID)
{
	TArray<FYapSpeechHandle> ToCancel;
	for (const FYapActiveLine& Line : Lines)
	{
		if (Line.bSpeaking && Line.Data.SpeakerID == SpeakerID)
		{
			ToCancel.Add(Line.Data.Handle);
		}
	}

	for (const FYapSpeechHandle& Speech : ToCancel)
	{
		CancelSpeech(Speech);
	}
	return ToCancel.Num();
}

// ================================================================================================
// Prompts

FYapActivePrompt* UYapSubsystem::FindPrompt(const FYapPromptHandle& Prompt)
{
	return Prompts.FindByPredicate([&Prompt](const FYapActivePrompt& Active) { return Active.Handle == Prompt; });
}

bool UYapSubsystem::StartPrompt(const FYapPromptRequest& Request, FYapPromptHandle& OutPrompt, FString& OutError)
{
	using namespace YapSubsystemPrivate;

	OutPrompt.Invalidate();

	if (!Request.Conversation.IsValid() || GetConversationState(Request.Conversation) != EYapConversationState::Open)
	{
		OutError = TEXT("A prompt must be in an open conversation.");
		return false;
	}

	if (Request.Entries.Num() == 0)
	{
		OutError = TEXT("A prompt needs at least one entry.");
		return false;
	}

	UYapSpeechConfig* Config = Request.Config ? Request.Config.Get() : GetDefaultSpeechConfig();
	UYapCharacterManager* Characters = UYapCharacterManager::Get(this);
	const bool bChildSafe = UsesChildSafe(*Broker, *Config);

	const FYapPromptHandle Handle = FYapPromptHandle::Make();

	// Everything is checked before the game is told anything.
	TArray<FYapPromptEntryData> Entries;
	for (int32 Index = 0; Index < Request.Entries.Num(); ++Index)
	{
		const FYapLineRequest& EntryRequest = Request.Entries[Index];
		FYapPromptEntryData& Entry = Entries.AddDefaulted_GetRef();
		Entry.Prompt = Handle;
		Entry.EntryIndex = Index;
		Entry.Conversation = Request.Conversation;
		Entry.Config = Config;
		Entry.SpeakerID = EntryRequest.Speaker;
		Entry.DirectedAtID = EntryRequest.DirectedAt;
		Entry.LineID = EntryRequest.LineID;
		Entry.TitleText = EntryRequest.TitleText;
		Entry.Text = (bChildSafe && !EntryRequest.ChildSafeText.IsEmpty()) ? EntryRequest.ChildSafeText : EntryRequest.MatureText;

		UYapCharacterAsset* Speaker = nullptr;
		UYapCharacterAsset* DirectedAt = nullptr;
		AActor* UnusedActor = nullptr;
		FString Error;
		if (!LookUpCharacter(Characters, EntryRequest.Speaker, TEXT("Speaker"), Speaker, UnusedActor, Error)
			|| !LookUpCharacter(Characters, EntryRequest.DirectedAt, TEXT("Directed at"), DirectedAt, UnusedActor, Error)
			|| !Config->ResolveMood(EntryRequest.Mood, Entry.Mood, Error))
		{
			OutError = FString::Printf(TEXT("Prompt entry %d: %s"), Index + 1, *Error);
			return false;
		}
		Entry.Speaker = Speaker;
		Entry.DirectedAt = DirectedAt;
	}

	FYapActivePrompt& Active = Prompts.AddDefaulted_GetRef();
	Active.Handle = Handle;
	Active.Conversation = Request.Conversation;
	Active.NumEntries = Entries.Num();
	OutPrompt = Handle;

	const FYapConversationHandle Conversation = Request.Conversation;
	for (const FYapPromptEntryData& Entry : Entries)
	{
		ForEachHandler(Config, [&Entry](UObject* Handler) { IYapDialogueHandler::NotifyPromptEntryCreated(Handler, Entry); });
	}

	if (Entries.Num() == 1 && Config->bAutoSelectLastPrompt)
	{
		ChoosePromptEntry(Handle, 0);
	}
	else
	{
		ForEachHandler(Config, [&](UObject* Handler) { IYapDialogueHandler::NotifyPromptReady(Handler, Handle, Conversation); });
	}

	return true;
}

bool UYapSubsystem::ChoosePromptEntry(const FYapPromptHandle& Prompt, int32 EntryIndex)
{
	FYapActivePrompt* Active = FindPrompt(Prompt);
	if (!Active)
	{
		UE_LOG(LogYapBase, Warning, TEXT("ChoosePromptEntry: there's no prompt {%s} (it was already chosen from, or taken away)."), *Prompt.ToString());
		return false;
	}

	if (Active->Chosen != INDEX_NONE)
	{
		return false;
	}

	if (EntryIndex < 0 || EntryIndex >= Active->NumEntries)
	{
		UE_LOG(LogYapBase, Error, TEXT("ChoosePromptEntry: the prompt has %d entries, so %d isn't one."), Active->NumEntries, EntryIndex);
		return false;
	}

	Active->Chosen = EntryIndex;
	const FYapConversationHandle Conversation = Active->Conversation;

	ForEachHandler(nullptr, [&](UObject* Handler) { IYapDialogueHandler::NotifyPromptEnded(Handler, Prompt, Conversation, EntryIndex); });
	OnPromptEnded.Broadcast(Prompt, EntryIndex);
	return true;
}

bool UYapSubsystem::GetPromptChoice(const FYapPromptHandle& Prompt, int32& OutChosen) const
{
	const FYapActivePrompt* Active = Prompts.FindByPredicate([&Prompt](const FYapActivePrompt& Entry) { return Entry.Handle == Prompt; });
	if (!Active)
	{
		OutChosen = INDEX_NONE;
		return false;
	}

	OutChosen = Active->Chosen;
	return true;
}

void UYapSubsystem::CancelPrompt(const FYapPromptHandle& Prompt)
{
	const int32 Index = Prompts.IndexOfByPredicate([&Prompt](const FYapActivePrompt& Active) { return Active.Handle == Prompt; });
	if (Index == INDEX_NONE)
	{
		return;
	}

	const bool bWasWaiting = Prompts[Index].Chosen == INDEX_NONE;
	const FYapConversationHandle Conversation = Prompts[Index].Conversation;
	Prompts.RemoveAt(Index);

	if (bWasWaiting)
	{
		ForEachHandler(nullptr, [&](UObject* Handler) { IYapDialogueHandler::NotifyPromptEnded(Handler, Prompt, Conversation, INDEX_NONE); });
		OnPromptEnded.Broadcast(Prompt, INDEX_NONE);
	}
}

void UYapSubsystem::ReleasePrompt(const FYapPromptHandle& Prompt)
{
	Prompts.RemoveAll([&Prompt](const FYapActivePrompt& Active) { return Active.Handle == Prompt; });
}

// ================================================================================================
// Conversations

int32 UYapSubsystem::FindConversationIndex(const FYapConversationHandle& Conversation) const
{
	return Conversations.IndexOfByPredicate([&Conversation](const FYapActiveConversation& Active) { return Active.Handle == Conversation; });
}

EYapConversationState UYapSubsystem::GetConversationState(const FYapConversationHandle& Conversation) const
{
	const int32 Index = FindConversationIndex(Conversation);
	return Index == INDEX_NONE ? EYapConversationState::None : Conversations[Index].State;
}

FYapConversationHandle UYapSubsystem::GetActiveConversation() const
{
	if (Conversations.Num() > 0 && Conversations[0].State != EYapConversationState::Queued)
	{
		return Conversations[0].Handle;
	}
	return FYapConversationHandle();
}

void UYapSubsystem::SetConversationState(FYapActiveConversation& Conversation, EYapConversationState NewState)
{
	Conversation.State = NewState;
	OnConversationStateChanged.Broadcast(Conversation.Handle, NewState);
}

FYapConversationHandle UYapSubsystem::OpenConversation(FName Name)
{
	FYapActiveConversation& Conversation = Conversations.AddDefaulted_GetRef();
	Conversation.Handle = FYapConversationHandle::Make();
	Conversation.Name = Name;
	Conversation.State = EYapConversationState::Queued;

	const FYapConversationHandle Handle = Conversation.Handle;

	if (Conversations.Num() == 1)
	{
		StartOpeningNext();
	}

	return Handle;
}

void UYapSubsystem::StartOpeningNext()
{
	if (Conversations.Num() == 0 || Conversations[0].State != EYapConversationState::Queued)
	{
		return;
	}

	SetConversationState(Conversations[0], EYapConversationState::Opening);

	FYapConversationData Data;
	Data.Handle = Conversations[0].Handle;
	Data.Name = Conversations[0].Name;

	// Handlers may apply opening interlocks here.
	ForEachHandler(nullptr, [&Data](UObject* Handler) { IYapDialogueHandler::NotifyConversationOpening(Handler, Data); });

	TryFinishOpening(Data.Handle);
}

void UYapSubsystem::TryFinishOpening(const FYapConversationHandle& Handle)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE || Conversations[Index].State != EYapConversationState::Opening)
	{
		return;
	}

	FYapActiveConversation& Conversation = Conversations[Index];
	Conversation.OpeningLocks.RemoveAll([](const TWeakObjectPtr<UObject>& Lock) { return !Lock.IsValid(); });
	if (Conversation.OpeningLocks.Num() > 0)
	{
		return;
	}

	SetConversationState(Conversation, EYapConversationState::Open);

	FYapConversationData Data;
	Data.Handle = Conversation.Handle;
	Data.Name = Conversation.Name;
	ForEachHandler(nullptr, [&Data](UObject* Handler) { IYapDialogueHandler::NotifyConversationOpened(Handler, Data); });
}

void UYapSubsystem::CloseConversation(const FYapConversationHandle& Handle, bool bImmediate)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE)
	{
		return;
	}

	// A queued conversation was never opened: it just goes.
	if (Conversations[Index].State == EYapConversationState::Queued)
	{
		Conversations.RemoveAt(Index);
		OnConversationStateChanged.Broadcast(Handle, EYapConversationState::None);
		return;
	}

	if (Conversations[Index].State != EYapConversationState::Closing)
	{
		StopConversationContents(Handle);

		const int32 ClosingIndex = FindConversationIndex(Handle);
		if (ClosingIndex == INDEX_NONE)
		{
			return;
		}

		SetConversationState(Conversations[ClosingIndex], EYapConversationState::Closing);

		FYapConversationData Data;
		Data.Handle = Handle;
		Data.Name = Conversations[ClosingIndex].Name;

		// Handlers may apply closing interlocks here.
		ForEachHandler(nullptr, [&Data](UObject* Handler) { IYapDialogueHandler::NotifyConversationClosing(Handler, Data); });
	}

	TryFinishClosing(Handle, bImmediate);
}

void UYapSubsystem::TryFinishClosing(const FYapConversationHandle& Handle, bool bForce)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE || Conversations[Index].State != EYapConversationState::Closing)
	{
		return;
	}

	FYapActiveConversation& Conversation = Conversations[Index];
	Conversation.ClosingLocks.RemoveAll([](const TWeakObjectPtr<UObject>& Lock) { return !Lock.IsValid(); });
	if (!bForce && Conversation.ClosingLocks.Num() > 0)
	{
		return;
	}

	FYapConversationData Data;
	Data.Handle = Conversation.Handle;
	Data.Name = Conversation.Name;

	Conversations.RemoveAt(Index);
	OnConversationStateChanged.Broadcast(Handle, EYapConversationState::None);
	ForEachHandler(nullptr, [&Data](UObject* Handler) { IYapDialogueHandler::NotifyConversationClosed(Handler, Data); });

	StartOpeningNext();
}

void UYapSubsystem::StopConversationContents(const FYapConversationHandle& Handle)
{
	TArray<FYapSpeechHandle> LinesToCancel;
	for (const FYapActiveLine& Line : Lines)
	{
		if (Line.Data.Conversation == Handle)
		{
			LinesToCancel.Add(Line.Data.Handle);
		}
	}
	for (const FYapSpeechHandle& Speech : LinesToCancel)
	{
		CancelSpeech(Speech);
	}

	TArray<FYapPromptHandle> PromptsToCancel;
	for (const FYapActivePrompt& Prompt : Prompts)
	{
		if (Prompt.Conversation == Handle)
		{
			PromptsToCancel.Add(Prompt.Handle);
		}
	}
	for (const FYapPromptHandle& Prompt : PromptsToCancel)
	{
		CancelPrompt(Prompt);
	}
}

void UYapSubsystem::AdvanceConversation(const FYapConversationHandle& Conversation)
{
	if (GetConversationState(Conversation) != EYapConversationState::Open)
	{
		return;
	}

	TArray<FYapSpeechHandle> InConversation;
	for (const FYapActiveLine& Line : Lines)
	{
		if (Line.Data.Conversation == Conversation)
		{
			InConversation.Add(Line.Data.Handle);
		}
	}

	for (const FYapSpeechHandle& Speech : InConversation)
	{
		FYapActiveLine* Line = FindLine(Speech);
		if (!Line)
		{
			continue;
		}

		// A line that can't be cut short holds the advance until it's done playing.
		const bool bMayCut = Line->bInterruptible || !Line->bTimed;

		if (Line->bSpeaking)
		{
			if (!bMayCut)
			{
				continue;
			}
			EndSpeech(Speech, EYapSpeechResult::Advanced);
		}

		FinishLine(Speech);
		RemoveLineIfDone(Speech);
	}
}

bool UYapSubsystem::ApplyOpeningInterlock(const FYapConversationHandle& Handle, UObject* Lock)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE || Conversations[Index].State != EYapConversationState::Opening || !Lock)
	{
		return false;
	}

	Conversations[Index].OpeningLocks.AddUnique(Lock);
	return true;
}

void UYapSubsystem::ReleaseOpeningInterlock(const FYapConversationHandle& Handle, UObject* Lock)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE)
	{
		return;
	}

	Conversations[Index].OpeningLocks.Remove(Lock);
	TryFinishOpening(Handle);
}

bool UYapSubsystem::ApplyClosingInterlock(const FYapConversationHandle& Handle, UObject* Lock)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE || Conversations[Index].State != EYapConversationState::Closing || !Lock)
	{
		return false;
	}

	Conversations[Index].ClosingLocks.AddUnique(Lock);
	return true;
}

void UYapSubsystem::ReleaseClosingInterlock(const FYapConversationHandle& Handle, UObject* Lock)
{
	const int32 Index = FindConversationIndex(Handle);
	if (Index == INDEX_NONE)
	{
		return;
	}

	Conversations[Index].ClosingLocks.Remove(Lock);
	TryFinishClosing(Handle, false);
}

// ================================================================================================
// Tick

void UYapSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;

	for (auto It = FinishedLines.CreateIterator(); It; ++It)
	{
		if (Now - It.Value() > YapSubsystemPrivate::FinishedLineMemorySeconds)
		{
			It.RemoveCurrent();
		}
	}

	// Interlocks whose objects are gone don't hold anything.
	if (Conversations.Num() > 0)
	{
		const FYapConversationHandle Active = Conversations[0].Handle;
		if (Conversations[0].State == EYapConversationState::Opening)
		{
			TryFinishOpening(Active);
		}
		else if (Conversations[0].State == EYapConversationState::Closing)
		{
			TryFinishClosing(Active, false);
		}
	}

	// Lines may start, stop and be removed while lines are updated.
	TArray<FYapSpeechHandle> ToUpdate;
	ToUpdate.Reserve(Lines.Num());
	for (FYapActiveLine& Line : Lines)
	{
		Line.Elapsed += DeltaTime;
		ToUpdate.Add(Line.Data.Handle);
	}

	for (const FYapSpeechHandle& Speech : ToUpdate)
	{
		UpdateLine(Speech);
	}
}
