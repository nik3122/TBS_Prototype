#include "CustomEditors.h"


IMPLEMENT_GAME_MODULE(FModule_CustomEditors, CustomEditors)

DEFINE_LOG_CATEGORY(CustomEditorsLog)


void FModule_CustomEditors::StartupModule()
{
#if UE_BUILD_DEBUG
	UE_LOG(CustomEditorsLog, Warning, TEXT("STARTUP MPDULE \"CustomEditors\""));
	UE_LOG(LogTemp, Warning, TEXT("STARTUP MPDULE \"CustomEditors\""));
#endif UE_BUILD_DEBUG
}

void FModule_CustomEditors::ShutdownModule()
{
#if UE_BUILD_DEBUG
	UE_LOG(CustomEditorsLog, Warning, TEXT("SHUTDOWN MODULE \"CustomEditors\""));
	UE_LOG(LogTemp, Warning, TEXT("SHUTDOWN MODULE \"CustomEditors\""));
#endif UE_BUILD_DEBUG
}
