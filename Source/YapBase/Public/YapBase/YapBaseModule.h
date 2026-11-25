#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** The YapBase runtime module. */
class YAPBASE_API FYapBaseModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
