#include "Modules/ModuleManager.h"

/** The YapBase editor module. */
class FYapBaseEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FYapBaseEditorModule, YapBaseEditor)
