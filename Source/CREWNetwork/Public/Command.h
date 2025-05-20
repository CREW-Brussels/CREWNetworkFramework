// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "GameplayTagContainer.h"
#include "Command.generated.h"

enum class ECommandType : uint8 {
	NoValue,
	Bool,
	Int,
	Float,
	Vector,
	Rotator,
	Transform,
	String
};

USTRUCT()
struct FCommand
{
	GENERATED_BODY()
public:
	FCommand() {}
	FCommand(FGameplayTag id_in, TSet<FIPv4Endpoint> &peers) {
		index = auto_inc++;
		id = id_in;
		type = ECommandType::NoValue;
		remaining = peers;
	}
	FCommand(FGameplayTag id_in, bool value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::Bool;
		value_bool = value;
	}
	FCommand(FGameplayTag id_in, int32 value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::Int;
		value_int = value;
	}
	FCommand(FGameplayTag id_in, float value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::Float;
		value_float = value;
	}
	FCommand(FGameplayTag id_in, FVector value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::Vector;
		value_vector = value;
	}
	FCommand(FGameplayTag id_in, FRotator value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::Rotator;
		value_rotator = value;
	}
	FCommand(FGameplayTag id_in, FTransform value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::Transform;
		value_transform = value;
	}
	FCommand(FGameplayTag id_in, FString value, TSet<FIPv4Endpoint> &peers) : FCommand(id_in, peers) {
		type = ECommandType::String;
		value_string = value;
	}
	uint32 index;
	FGameplayTag id;
	ECommandType type;
	bool value_bool;
	int value_int;
	float value_float;
	FVector value_vector;
	FRotator value_rotator;
	FTransform value_transform;
	FString value_string;
	TSet<FIPv4Endpoint> remaining;
private:
	static uint32 auto_inc;
};

FArchive& operator<<(FArchive& Ar, FCommand& A)
{
	Ar << A.index;
	Ar << A.id;
	Ar << A.type;
	switch (A.type) {
	case ECommandType::NoValue:
		break;
	case ECommandType::Bool:
		Ar << A.value_bool;
		break;
	case ECommandType::Int:
		Ar << A.value_int;
		break;
	case ECommandType::Float:
		Ar << A.value_float;
		break;
	case ECommandType::Vector:
		Ar << A.value_vector;
		break;
	case ECommandType::Rotator:
		Ar << A.value_rotator;
		break;
	case ECommandType::Transform:
		Ar << A.value_transform;
		break;
	case ECommandType::String:
		Ar << A.value_string;
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Invalid type in a CREW Network command"));
		break;
	}
	return Ar;
}