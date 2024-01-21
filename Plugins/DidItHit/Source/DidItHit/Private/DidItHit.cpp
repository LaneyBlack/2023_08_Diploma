// Copyright 2021, Project Zero. All Rights Reserved.

#include "DidItHit.h"

#define LOCTEXT_NAMESPACE "FDidItHitModule"

void FDidItHitModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FDidItHitModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDidItHitModule, DidItHit)