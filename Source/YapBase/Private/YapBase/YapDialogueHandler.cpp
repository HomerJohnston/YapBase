#include "YapBase/YapDialogueHandler.h"

// A C++ implementation (or a Blueprint subclass of one) is called through its virtual function, whose default calls
// the Blueprint event; an object that implements the interface only in Blueprint is called through Execute_.
#define YAP_NOTIFY(CppCall, K2Call) \
	if (!Handler) { return; } \
	if (IYapDialogueHandler* Native = Cast<IYapDialogueHandler>(Handler)) { Native->CppCall; } \
	else if (Handler->GetClass()->ImplementsInterface(UYapDialogueHandler::StaticClass())) { K2Call; }

void IYapDialogueHandler::NotifyConversationOpening(UObject* Handler, const FYapConversationData& Conversation)
{
	YAP_NOTIFY(OnConversationOpening(Conversation), Execute_K2_ConversationOpening(Handler, Conversation))
}

void IYapDialogueHandler::NotifyConversationOpened(UObject* Handler, const FYapConversationData& Conversation)
{
	YAP_NOTIFY(OnConversationOpened(Conversation), Execute_K2_ConversationOpened(Handler, Conversation))
}

void IYapDialogueHandler::NotifyConversationClosing(UObject* Handler, const FYapConversationData& Conversation)
{
	YAP_NOTIFY(OnConversationClosing(Conversation), Execute_K2_ConversationClosing(Handler, Conversation))
}

void IYapDialogueHandler::NotifyConversationClosed(UObject* Handler, const FYapConversationData& Conversation)
{
	YAP_NOTIFY(OnConversationClosed(Conversation), Execute_K2_ConversationClosed(Handler, Conversation))
}

void IYapDialogueHandler::NotifySpeechBegins(UObject* Handler, const FYapSpeechData& Speech)
{
	YAP_NOTIFY(OnSpeechBegins(Speech), Execute_K2_SpeechBegins(Handler, Speech))
}

void IYapDialogueHandler::NotifySpeechEnds(UObject* Handler, const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation, EYapSpeechResult Result)
{
	YAP_NOTIFY(OnSpeechEnds(Speech, Conversation, Result), Execute_K2_SpeechEnds(Handler, Speech, Conversation, Result))
}

void IYapDialogueHandler::NotifyAwaitingAdvance(UObject* Handler, const FYapSpeechHandle& Speech, const FYapConversationHandle& Conversation)
{
	YAP_NOTIFY(OnAwaitingAdvance(Speech, Conversation), Execute_K2_AwaitingAdvance(Handler, Speech, Conversation))
}

void IYapDialogueHandler::NotifyPromptEntryCreated(UObject* Handler, const FYapPromptEntryData& Entry)
{
	YAP_NOTIFY(OnPromptEntryCreated(Entry), Execute_K2_PromptEntryCreated(Handler, Entry))
}

void IYapDialogueHandler::NotifyPromptReady(UObject* Handler, const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation)
{
	YAP_NOTIFY(OnPromptReady(Prompt, Conversation), Execute_K2_PromptReady(Handler, Prompt, Conversation))
}

void IYapDialogueHandler::NotifyPromptEnded(UObject* Handler, const FYapPromptHandle& Prompt, const FYapConversationHandle& Conversation, int32 ChosenEntry)
{
	YAP_NOTIFY(OnPromptEnded(Prompt, Conversation, ChosenEntry), Execute_K2_PromptEnded(Handler, Prompt, Conversation, ChosenEntry))
}

#undef YAP_NOTIFY
