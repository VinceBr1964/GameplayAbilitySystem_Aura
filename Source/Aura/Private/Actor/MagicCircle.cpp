// Copyright Vince Bracken


#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"


AMagicCircle::AMagicCircle()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	MagicCircleDecal = CreateDefaultSubobject<UDecalComponent>("MagicCircleDecal");
	MagicCircleDecal->SetupAttachment(RootComponent);


}


void AMagicCircle::BeginPlay()
{
	Super::BeginPlay();
	
}


void AMagicCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

