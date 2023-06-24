# PushPawn
***Due to the use of Git LFS, do NOT download a zip or your content will be missing.*** You will need to clone this via `git clone https://github.com/Vaei/PushPawn.git`

Allows pawns to push each other, uses GAS to prevent desyncs that often occur when colliding with AI due to lack of net prediction between Characters.

The primary purpose is to overcome desync when players collide with AI, but does provide a nice "soft collision" result instead of the rigid "hard collision" that most games have.

Initially created based on the 5.1 LyraShooter interaction system. Please note that some of the code comments are left-overs from LyraShooter's interaction system and may not make sense in this context.

Summary: When a pawn that can be pushed (pushee) finds a nearby pawn that can push them (pusher), the pushee will request the push options from the pusher, and apply those to itself via a `UGameplayAbility`.

Tested >200ms without problematic desync.

## Example
This was initially created for the upcoming project with the working title "Isekai" and is offered freely under the MIT License. This game also makes the AI play a push animation and yell at the player, which is not included, however the helper functions for building this are included.
![example usage](https://github.com/Vaei/PlayWidgetAnimation/blob/examples/Example1.png)

## Difficulty
This is very advanced C++ use, if you are a beginner you will struggle with this and dedicated support isn't available. It will benefit you if you understand how LyraShooter's interaction system works though it isn't a requirement. There may be some redundancy that came from duplicating that system as a starting point which may lead to confusion.

## Terminology
*The most confusing yet important aspect to wrap your head around is this: The ability `Instigator` is the one who gets pushed, and not the one who does the pushing.*

### AICharacter
An AI controlled `ACharacter` (possesed by `AAIController`)

### PlayerCharacter
A player controlled `ACharacter` (possesed by `APlayerController`)

### Standard Use Case
Refers to the use-case where an AICharacter is using this system specifically to push a PlayerCharacter.

### Pushee
This is the Pawn that is getting pushed by another Pawn. Implementing `IPusheeInstigator`. Under the Standard Use Case this is a PlayerCharacter.

This Pawn is the ability Instigator. Due to the inversion, this term was avoided because it could cause confusion.

This Pawn searches for anyone who should push them, effectively a psuedo collision detection, and then "Instigates" by handing the colliding Pawn the ability that is used to push them back.

This means that if a massive world boss needs to push a player back, they can push a player back in their own distinct way especially factoring their unique collision properties, instead of a single universal method for pushing back.

### Pusher
This is the Pawn that is pushing the other player. Implementing `IPusherTarget`. Under the Standard Use Case this is an AI Character.

## Warning
This has only been tested for a single use case: An AI controlled ACharacter pushing a player controlled ACharacter. No other use-case has currently been tested as of the time of writing this.

The content is only supported by Unreal Engine 5.2 and up. However, there is probably nothing stopping you from mimicking the blueprints and only making use of the source code.

## Limitations
If the capsule dimensions change between prediction frames it can desync. For most of us sending this data is an unnecessary cost, but if you need to do it, add the capsule `HalfHeight` and/or `Radius` to `FPushOption` and send it along with `IPusheeInstigator::GatherPushOptions`, however this may not be sufficient on it's own! Check where the `UCapsuleComponent` getters are being used and replace these too. Any data NOT send through the `FPushOption` is very unlikely to be predicted.

This may also not be sufficient as it remains untested.

## LyraShooter
*Or anyone not using `UGameplayAbility` as an allowed base class*

*This has been tested and is confirmed working with LyraShooter's framework*

LyraShooter requires that `GameplayAbilities` extend `ULyraGameplayAbility`; you will likely need to duplicate `UGameplayAbility_Push_Scan` into your project and have it inherit `ULyraGameplayAbility` and then duplicate the `GA_Push_Scan` ability in the content folder and reparent it under this one.

Add to the ctor of duplicated ability: `ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;` and add to the appropriate attribute sets so that it always runs. Don't forget to change the `PUSHPAWN_API` macro to your own!

## How to Use
*This how to was written from the viewpoint of a brand new 5.2 "Third Person" template. Many of these steps may be unnecessary for your project, this is for the purpose of documenting the entire process without leaving anything out.*

### Create New Project
1. Launch Unreal 5.2 and create C++ Third Person template
1. Close the editor and IDE

### Install Plugin
1. Create a "Plugins" folder in the project root folder
1. Clone using `git clone https://github.com/Vaei/PushPawn.git`.
  * Due to the use of Git LFS, do NOT download a zip or your content will be missing. You must clone!
1. Delete project Binaries & Intermediate folder
1. Right click .uproject and `Generate Visual Studio project files`
1. Open it in your IDE
1. Add `PushPawn` to your .uproject plugins and enable it
1. Add `PushPawn` to your .build.cs `PublicDependencyModuleNames`
1. Compile and launch the editor

### Collision Setup
1. Change `Pawn` profile and `CharacterMesh` profile to ignore `ECC_Camera`
  * Gets in the way when testing
1. Add trace channel `PushPawn` and set Default Response to `Ignore`
1. Change `Pawn` profile to block `PushPawn` and ignore `Pawn`

### Testing Settings
1. Open Editor Preferences and select `Level Editor` -> `Play`
1. Change `Play Net Mode` to `Play As Client`
1. Enable `Enable Network Emulation`
1. Set `Minimum Latency` to `90` and `Maximum Latency` to `110` for both `Incoming Traffic` and `Outgoing Traffic`
  * This ensures real-world networking conditions with considerable latency of over 200ms
1. Open `BP_ThirdPersonCharacter` and `Event BeginPlay` -> `Is Locally Controlled` -> `Execute Console Command` -> `p.netshowcorrections 1`
  * Any time your character desyncs, it will draw red/green capsules representing the correction, we now know if this plugin can desync us or not and how it handles desync when it should occur

### AI Setup
1. Duplicate `BP_ThirdPersonCharacter` and name it `BP_ThirdPersonBot`
1. Add a `Nav Mesh Bounds Volume` to the level
1. Press "P" to visualize
1. Scale the volume to fill the level
1. Delete the `Is Locally Controlled` and `Execute Console Command`
1. `Begin Play` -> `Switch Has Authority` -> `Authority` -> `AI MoveTo` -> (`Destination`) -> `GetRandomReachablePointInRadius` -> Set `Origin` to center of scene roughly & `Radius` to `3000`
1. Connect both `On Success` and `On Fail` to the same `AI MoveTo` node resulting in an infinite execution

The AI now runs around the level when simulating.

### AbilitySystem
1. Close Editor
1. Add `GameplayAbilities` to Build.cs `PrivateDependencyModuleNames`
1. Extend `IAbilitySystemInterface` from `MyCharacter`
  * This is confirmed working with `UAbilitySystemComponent` on `APlayerState`
1. Add `UAbilitySystemComponent* AbilitySystemComponent` to `MyCharacter`
  * `AbilitySystemComponent->CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));`
1. Override `virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;` to return `AbilitySystemComponent`
1. Create `PushPawnAssetManager` derived from `UAssetManager`
1. Override `virtual void StartInitialLoading() override;` and call `UAbilitySystemGlobals::Get().InitGlobalData();`

### PushPawn
1. Extend `MyCharacter` to `MyPlayerCharacter` and also `MyBotCharacter`
1. `MyBotCharacter` extend `IPusherTarget`
1. `MyPlayerCharacter` extend `IPusheeInstigator`

#### MyBotCharacter
```cpp
// .h
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> PushAbilityToGrant;

protected:
	//~ Begin IPusherTarget Interface
	virtual void GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder) override;
	virtual void CustomizePushEventData(const FGameplayTag& PushEventTag, FGameplayEventData& InOutEventData) override {}
	virtual bool CanPush(const AActor* PusheeActor) const override;
	//~ End IPusherTarget Interface
```
```cpp
// .cpp
void AMyBotCharacter::GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder)
{
	if (PushQuery.RequestingAvatar.IsValid())
	{
		FPushOption Push;
		Push.PushAbilityToGrant = PushAbilityToGrant;
		Push.PusheeActorLocation = PushQuery.RequestingAvatar->GetActorLocation();
		Push.PusheeForwardVector = GetActorForwardVector();
		Push.PusherActorLocation = GetActorLocation();
		OptionBuilder.AddPushOption(Push);
	}
}

bool AMyBotCharacter::CanPush(const AActor* PusheeActor) const
{
	return !IsPendingKillPending();
}
```

### Optional
Duplicate the gameplay ability from the content folder to your project and extend as required. If you don't need to extend it, no need to do this step.

Any character that can be pushed must implement the IPusheeTarget interface (C++ only)
Any character that can do the pushing must implement the IPusherTarget interface (C++ only)

In `GA_Push_Action`, assign a trace channel. You probably do not want to leave this as `ECC_Visibility`, but instead setup a channel that only valid Pushers will block.

## Changelog
### 1.0.0.0
Initial Release