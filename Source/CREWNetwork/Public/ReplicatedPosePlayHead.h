// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "BoneIndices.h"
#include "AnimNode_Replicate.h"

#include "ReplicatedPosePlayHead.generated.h"

USTRUCT()
struct FReplicatedPoseFrame
{
	GENERATED_BODY()
public:
	FReplicatedPoseFrame() { time = 0.0; }
	TArray<FTransform> pose;
	double time;
};

USTRUCT()
struct FReplicatedPosePlayHead
{
	GENERATED_BODY()
public:
	FReplicatedPosePlayHead() {
		time = 0.0;
		previousTime = 0.0;
		limiter = 0.0;
		head = 0;
		boneCount = 0;
		fragments_received = 0;
		fragments_index = -1;
		send_index = 0;
		source = nullptr;
<<<<<<< Updated upstream
=======
	}
	void Init(FName name) {
#if WITH_LIVE_LINK
		source = NewObject<UCustomLiveLinkSource>();
		source->Initialize(name);
#endif
>>>>>>> Stashed changes
	}
	void AddFragment(TArray<FTransform>& in, int16 total, int16 offset, int32 index, double t) {
		if (fragments_index < index || index < (fragments_index-10)) {
			received = 0;
			fragments.SetNumUninitialized(total);
			fragments_received = 0;
			fragments_index = index;
		}
		uint32 bitMask = 1 << (offset / 60);
		if (!(fragments_received & bitMask)) {
			fragments_received |= bitMask;
			received += in.Num();
			for (int i = 0; i < in.Num(); i++) {
				fragments[offset + i] = in[i];
			}
			if (received == total) {
				AddFrame(fragments, t);
			}
		}
	}
	void AddFrame(TArray<FTransform>& in, double t) {
		if (boneCount == 0) {
			if (head == 1) {
				time = t;
			}
			if (head == 3) {
				boneCount = in.Num();
			}
		}
		else {
			if (in.Num() != boneCount) {
				return;
			}
		}
		frames[head].pose = in;
		frames[head].time = t;
		head = (head + 1) % 4;
	}
	bool GetFrame(FPoseContext& Output, double t) {
		FReplicatedPoseFrame &f1 = frames[head];
		FReplicatedPoseFrame &f2 = frames[(head + 1) % 4];
		FReplicatedPoseFrame &f3 = frames[(head + 2) % 4];
		FReplicatedPoseFrame &f4 = frames[(head + 3) % 4];
		double dilation = 1.0;
		if (f1.time > time) {
			time = frames[head].time;
		}
		if (f2.time > time) {
			dilation = (f3.time - time) / (f3.time - f2.time);
		}
		if (f3.time < time) {
			dilation = (f4.time - time) / (f4.time - f3.time);
		}
		double dt = 0.0;
		if (previousTime > 0.0) {
			dt = t - previousTime;
		}
		time += dt * dilation;
		previousTime = t;
		if (f2.time > time) {
			return InterpolatePose(f1.pose, f2.pose, (time - f1.time) / (f2.time - f1.time), Output);
		}
		else if (f3.time > time) {
			return InterpolatePose(f2.pose, f3.pose, (time - f2.time) / (f3.time - f2.time), Output);
		}
		else {
			return InterpolatePose(f3.pose, f4.pose, (time - f3.time) / (f4.time - f3.time), Output);
		}
	}
	bool InterpolatePose(TArray<FTransform> &a, TArray<FTransform> &b, const float Alpha, FPoseContext &Output) {
		int32 count = Output.Pose.GetNumBones();
		if (a.Num() != count || b.Num() != count || boneCount != count) {
			//UE_LOG(LogTemp, Warning, TEXT("Input: %d, FrameA: %d, FrameB: %d"), count, a.Num(), b.Num());
			return false;
		}
		for (int32 i = 0; i < count; ++i)
		{
			Output.Pose[FCompactPoseBoneIndex(i)].Blend(a[i], b[i], Alpha);
		}
		return true;
	}

	FReplicatedPoseFrame frames[4];
	double time;
	double previousTime;
	double limiter;
	int32 head;
	int32 boneCount;
	TArray<FTransform> fragments;
	uint32 fragments_received;
	int32 fragments_index;
	int16 received;
	int32 send_index;
};

class TransformArraySerializer {
public:
	static void SerializeCompressedTransforms(FMemoryWriter& Writer, const TArray<FTransform>& Transforms)
	{
		static TArray<int16> compressed;

		compressed.Empty(Transforms.Num() * 10);

		FQuat r;
		FVector l;
		FVector s;

		for (const FTransform& t : Transforms) {
			r = t.GetRotation();
			l = t.GetTranslation();
			s = t.GetScale3D();

			compressed.Add(FMath::Clamp(FMath::RoundToInt(r.X * 32767.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(r.Y * 32767.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(r.Z * 32767.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(r.W * 32767.0), -32768, 32767));

			compressed.Add(FMath::Clamp(FMath::RoundToInt(l.X * 10.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(l.Y * 10.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(l.Z * 10.0), -32768, 32767));

			compressed.Add(FMath::Clamp(FMath::RoundToInt(s.X * 1000.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(s.Y * 1000.0), -32768, 32767));
			compressed.Add(FMath::Clamp(FMath::RoundToInt(s.Z * 1000.0), -32768, 32767));
		}

		Writer << compressed;
	}

	static void DeserializeCompressedTransforms(FMemoryReader& Reader, TArray<FTransform>& Transforms)
	{
		static TArray<int16> compressed;

		Reader << compressed;

		Transforms.SetNum(compressed.Num() / 10);

		FQuat r;
		FVector l;
		FVector s;

		for (int i = 0; i < Transforms.Num(); i++) {
			r.X = double(compressed[i * 10 + 0]) / 32767.0f;
			r.Y = double(compressed[i * 10 + 1]) / 32767.0f;
			r.Z = double(compressed[i * 10 + 2]) / 32767.0f;
			r.W = double(compressed[i * 10 + 3]) / 32767.0f;

			l.X = double(compressed[i * 10 + 4]) / 10.0f;
			l.Y = double(compressed[i * 10 + 5]) / 10.0f;
			l.Z = double(compressed[i * 10 + 6]) / 10.0f;

			s.X = double(compressed[i * 10 + 7]) / 1000.0f;
			s.Y = double(compressed[i * 10 + 8]) / 1000.0f;
			s.Z = double(compressed[i * 10 + 9]) / 1000.0f;

			Transforms[i].SetComponents(r, l, s);
			Transforms[i].NormalizeRotation();
		}
	}
};
