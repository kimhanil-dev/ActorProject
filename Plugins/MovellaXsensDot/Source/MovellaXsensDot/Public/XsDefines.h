#pragma once

#include "CoreMinimal.h"
#include "movelladot_pc_sdk.h"

#pragma warning(disable : 4127)

DECLARE_LOG_CATEGORY_EXTERN(XsDot, Log, All); 

#define XDLOG_MINIMAL(Verbosity, Format, ...) UE_LOG(XsDot, Verbosity, Format, __VA_ARGS__)
#define XDLOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define XDLOG_S(Verbosity) UE_LOG(XsDot, Verbosity, TEXT("%s"), *XDLOG_CALLINFO)
#define XDLOG(Verbosity, Format, ...) UE_LOG(XsDot, Verbosity, TEXT("%s %s"), *XDLOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define XDOBJ_TO_UESTR(obj) FString(obj.toString().c_str())
#define XDSTR_TO_UESTR(str) FString(str.c_str())