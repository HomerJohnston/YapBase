#pragma once

#include "GameplayTagsManager.h"

namespace YapBaseEditor
{
	/** The tag's last part ("Mood.Happy" -> "Happy"). */
	inline FName GetLeafOfTag(const FGameplayTag& Tag)
	{
		const TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(Tag);
		return Node.IsValid() ? Node->GetSimpleTagName() : NAME_None;
	}

	/** Opens the gameplay tag manager, filtered to these roots (all tags if there are none). */
	void OpenGameplayTagsEditor(const FGameplayTagContainer& Roots);
}
