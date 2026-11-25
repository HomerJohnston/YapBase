#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "YapTypes.generated.h"

class AActor;
class UYapCharacterAsset;
class UYapSpeechConfig;

// ================================================================================================
// Settings

UENUM(BlueprintType)
enum class EYapMaturitySetting : uint8
{
	Unspecified UMETA(Hidden),
	ChildSafe,
	Mature,
};

/** How a line's speaking time is worked out. The numbers are the ones scripts pass; 0 is not a mode. */
UENUM(BlueprintType)
enum class EYapTimeMode : uint8
{
	/** The speech config's default. */
	Default = 0 UMETA(Hidden),
	/** From the text's length (words per minute, the broker's playback speed). */
	Text = 1,
	/** The audio's length, or the text's when the line has no audio. */
	Audio = 2,
	/** A time given with the line (its padding value). */
	Manual = 3,
	/** No time: in a conversation, the line plays until the player advances. Not allowed for free speech. */
	None = 4,
};

/** A setting a line or speech can override. The numbers are the ones scripts pass; 0 is not a value. */
UENUM(BlueprintType)
enum class EYapToggle : uint8
{
	Default = 0 UMETA(Hidden),
	Yes = 1,
	No = 2,
};

// ================================================================================================
// State

UENUM(BlueprintType)
enum class EYapSpeechResult : uint8
{
	/** The speech played to its end. */
	Normal,
	/** The player advanced the conversation while it was playing. */
	Advanced,
	/** Stopped by code (CancelSpeech, overlapping speech, its conversation closing). */
	Cancelled,
};

UENUM(BlueprintType)
enum class EYapLineState : uint8
{
	/** There's no such line here: it never ran in this world, or it was released, or the world was reset (a save was loaded). */
	Unknown,
	/** It still holds up what comes after it. */
	Running,
	/** In a conversation, set to wait for the player: done playing, waiting to be advanced. */
	AwaitingAdvance,
	/** What comes after it may start. Its speech may still be playing (negative padding). */
	Finished,
};

UENUM(BlueprintType)
enum class EYapConversationState : uint8
{
	/** There's no such conversation: it never opened, or it has closed. */
	None,
	/** Waiting for the conversation before it to close. */
	Queued,
	/** Opening: waiting for opening interlocks to be released. */
	Opening,
	Open,
	/** Closing: waiting for closing interlocks to be released. */
	Closing,
};

// ================================================================================================
// Handles

/** One running line of speech. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapSpeechHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FGuid Guid;

	bool IsValid() const { return Guid.IsValid(); }
	void Invalidate() { Guid.Invalidate(); }
	FString ToString() const { return Guid.ToString(EGuidFormats::Digits); }
	static FYapSpeechHandle Make() { FYapSpeechHandle Handle; Handle.Guid = FGuid::NewGuid(); return Handle; }
	static FYapSpeechHandle FromString(const FString& String) { FYapSpeechHandle Handle; FGuid::Parse(String, Handle.Guid); return Handle; }

	bool operator==(const FYapSpeechHandle& Other) const { return Guid == Other.Guid; }
	bool operator!=(const FYapSpeechHandle& Other) const { return Guid != Other.Guid; }
	friend uint32 GetTypeHash(const FYapSpeechHandle& Handle) { return GetTypeHash(Handle.Guid); }
};

/** One conversation, from when it's opened (or queued) until it's closed. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapConversationHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FGuid Guid;

	bool IsValid() const { return Guid.IsValid(); }
	void Invalidate() { Guid.Invalidate(); }
	FString ToString() const { return Guid.ToString(EGuidFormats::Digits); }
	static FYapConversationHandle Make() { FYapConversationHandle Handle; Handle.Guid = FGuid::NewGuid(); return Handle; }
	static FYapConversationHandle FromString(const FString& String) { FYapConversationHandle Handle; FGuid::Parse(String, Handle.Guid); return Handle; }

	bool operator==(const FYapConversationHandle& Other) const { return Guid == Other.Guid; }
	bool operator!=(const FYapConversationHandle& Other) const { return Guid != Other.Guid; }
	friend uint32 GetTypeHash(const FYapConversationHandle& Handle) { return GetTypeHash(Handle.Guid); }
};

/** One prompt: a set of entries shown to the player to choose from. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapPromptHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FGuid Guid;

	bool IsValid() const { return Guid.IsValid(); }
	void Invalidate() { Guid.Invalidate(); }
	FString ToString() const { return Guid.ToString(EGuidFormats::Digits); }
	static FYapPromptHandle Make() { FYapPromptHandle Handle; Handle.Guid = FGuid::NewGuid(); return Handle; }
	static FYapPromptHandle FromString(const FString& String) { FYapPromptHandle Handle; FGuid::Parse(String, Handle.Guid); return Handle; }

	bool operator==(const FYapPromptHandle& Other) const { return Guid == Other.Guid; }
	bool operator!=(const FYapPromptHandle& Other) const { return Guid != Other.Guid; }
	friend uint32 GetTypeHash(const FYapPromptHandle& Handle) { return GetTypeHash(Handle.Guid); }
};

// ================================================================================================
// Requests: what an integration (BangoYap, FlowYap, C++) hands YapBase

/** A line to speak, or a prompt entry. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapLineRequest
{
	GENERATED_BODY()

	/** The speech config. Null: the project's default config. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	TObjectPtr<UYapSpeechConfig> Config = nullptr;

	/** The conversation the line belongs to. Unset: free speech. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FYapConversationHandle Conversation;

	/** The speaking character's ID (UYapCharacterAsset::GetCharacterID). None: no speaker. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FName Speaker;

	/** Who it's said to. None: nobody in particular. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FName DirectedAt;

	/** The line's unique ID (its text's key). None: a line with no ID and no audio. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FName LineID;

	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FText MatureText;

	/** Empty: the mature text is used for child-safe players too. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FText ChildSafeText;

	/** A prompt entry's title text (what the player picks). */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FText TitleText;

	/** The mood tag's name under the config's mood root ("Happy" for Yap.Mood.Happy). None: the config's default mood. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FName Mood;

	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	EYapTimeMode TimeMode = EYapTimeMode::Default;

	/** If set, Padding overrides the config's default padding; in Manual time mode it's the line's time. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	bool bHasPadding = false;

	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	float Padding = 0.0f;

	/** Conversations only: can the player's advance cut the line short? Free speech always can be stopped by code. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	EYapToggle Interruptible = EYapToggle::Default;

	/** Conversations only: does the line move on by itself, or wait for the player? Free speech always moves on. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	EYapToggle AutoAdvance = EYapToggle::Default;

	/** Seconds already played (speech and padding), to carry on a line after a save is loaded. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	float StartOffset = 0.0f;
};

/** A prompt: entries for the player to choose from, in a conversation. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapPromptRequest
{
	GENERATED_BODY()

	/** Null: the project's default config. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	TObjectPtr<UYapSpeechConfig> Config = nullptr;

	/** Must be open. */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	FYapConversationHandle Conversation;

	/** At least one. Each one's Config and Conversation are ignored (the prompt's are used). */
	UPROPERTY(BlueprintReadWrite, Category = "Yap")
	TArray<FYapLineRequest> Entries;
};

// ================================================================================================
// Event data: what the game is sent

USTRUCT(BlueprintType)
struct YAPBASE_API FYapConversationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FYapConversationHandle Handle;

	/** None for a conversation opened without a name. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName Name;
};

/** A line beginning to play. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapSpeechData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FYapSpeechHandle Handle;

	/** Unset for free speech. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FYapConversationHandle Conversation;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapSpeechConfig> Config = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName SpeakerID;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapCharacterAsset> Speaker = nullptr;

	/** The speaker's actor in the world, if one is registered (UYapCharacterComponent). */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<AActor> SpeakerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName DirectedAtID;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapCharacterAsset> DirectedAt = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<AActor> DirectedAtActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName LineID;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FGameplayTag Mood;

	/** The text for the player's maturity setting. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FText Text;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FText TitleText;

	/** Null if the line has no audio. Cast it to your audio type. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UObject> Audio = nullptr;

	/** How long the speech plays, in seconds. 0 when bTimed is false. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	float SpeechTime = 0.0f;

	/** False for time mode None: it plays until advanced. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	bool bTimed = true;

	/** Seconds already played: start the audio this far in (non-zero after a save is loaded). */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	float StartOffset = 0.0f;

	/** Can the player's advance cut it short? (Conversation only; free speech is always true.) */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	bool bInterruptible = true;

	/** Does it move on by itself, or wait for the player? (Conversation only; free speech is always true.) */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	bool bAutoAdvance = true;
};

/** One entry of a prompt, for the game to show. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapPromptEntryData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FYapPromptHandle Prompt;

	/** Pass this to ChoosePromptEntry. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	int32 EntryIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FYapConversationHandle Conversation;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapSpeechConfig> Config = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName SpeakerID;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapCharacterAsset> Speaker = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName DirectedAtID;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapCharacterAsset> DirectedAt = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FName LineID;

	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FGameplayTag Mood;

	/** What will be said if it's chosen, for the player's maturity setting. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FText Text;

	/** What the player picks. Empty if the entry has none: show Text then. */
	UPROPERTY(BlueprintReadOnly, Category = "Yap")
	FText TitleText;
};
