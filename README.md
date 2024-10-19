# Push Pawn <img align="right" width=128, height=128 src="https://github.com/Vaei/PushPawn/blob/main/Resources/Icon128.png">

> [!IMPORTANT]
> **PushPawn provides net-predicted soft organic collisions!**
> <br>Which eliminates inappropriate de-syncing when running near other pawns in multiplayer games
> <br>And results in great game feel!
> <br>PushPawn is aimed at both singleplayer and multiplayer games
> <br>Fully tested in production with both Player-Controlled Characters and AI-Controlled Characters.

> [!TIP]
> ***Due to the use of Git LFS, do not download a zip or your content will be missing***
> <br>You will need to clone this via `git clone https://github.com/Vaei/PushPawn.git` if you want the content
> <br>Using C++ only is viable also as of `2.0.0`

## Features
* Net predicted Pawn vs. Pawn collisions via GAS to prevent de-sync
* Soft organic collisions instead of the engine's brick wall collisions
* Capsule, Box, Sphere collisions all supported for Pawn v Pawn collisions (even with CMC)
* Abilities that can respond to gameplay events, e.g. playing a push animation, procedural skeletal physics, or yelling at the other Pawn, etc.
* Allows pawns to push each other out of the way - great for groups of NPCs attacking a player
* Lets players push each other off cliffs, if you want that...
* Exceptionally customizable

## Test-Cases
* \>200ms latency without problematic de-sync
* Shippable production-ready multiplayer game tested with high player counts with lots of NPCs

## Example
_Examples are based on 1.0.0_

This was initially created for a personal project that would make the AI play a push animation and yell at the player, which is not included, however the helper functions for building this are included.

![example usage](https://github.com/Vaei/repo_files/blob/main/PushPawn/preview_isekai.gif)

Here is the preview from the minimal Third Person Example project below. This footage was obtained with >200ms and `p.netshowcorrections 1` - as you can see, it doesn't de-sync abnormally.

![example usage](https://github.com/Vaei/repo_files/blob/main/PushPawn/preview.gif)

## Limitations
### Changing Collision Size
If the collision dimensions change between prediction frames it can de-sync. For most of us sending this data is an unnecessary cost and can be ignored, behaviour is still reasonable because its not a 'brick wall' collision.

Adding this is completely untested, but adding the shape dimensions to `FPushOption` through `IPusheeInstigator::GatherPushOptions` is the first thing you will want to do.

## Performance
PushPawn can very frequently activate abilities. As of 2.0.0 abilities with lightweight implementations and C++ only are available for this purpose. Furthermore for the sake of maximizing performance, classes have been marked with `final` but you may fork/remove this yourself.

Easy wins for performance:
	* Reduce the scan rates
	* Use varying radiuses between character types, so that two overlaps don't occur simultaneously

Not-so-easy wins for performance:
	* Use the included C++ classes instead of blueprint - its fine to subclass as blueprint to change properties but avoid overriding functions and implementing logic

### Large Character Counts
There is no real limitation with large character counts and PushPawn, but it does need to be factored in and some additional work will be required to manage this. How you handle this will depend entirely on your project and use-case.

With the standard implementation, the scan ability keeps the AbilitySystemComponent ticking due to it's `WaitForPushTargets` task. You should pause or resume the scanning based on significance and other such factors.

There is no need to search for nearby characters if:
* We're a Non-Player Character and no players are aware of us at all (when reactivating, they will eject each other if overlapping)
* Other characters are sufficiently far away that they won't de-sync within their latency vs max velocity thresholds, meaning we can detect they're close enough and activate the ability sufficiently before they reach us and we actually need to push them

You can disable the scan using `IPusheeInstigator::GetPushPawnScanPausedDelegate()`.

## LyraShooter
*Or anyone not using `UGameplayAbility` as an allowed base class*

*This has been tested and is confirmed working with LyraShooter's framework*

LyraShooter requires that `GameplayAbilities` extend `ULyraGameplayAbility`; you will likely need to duplicate the included abilities into your project and have it inherit `ULyraGameplayAbility`. This will also mean reparenting the blueprint content to your new classes, if you're using those (recommend using C++ versions instead).

Add to the ctor of duplicated ability: `ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;` and add to the appropriate attribute sets so that it always runs. Don't forget to change the `PUSHPAWN_API` macro to your own!

## How to Use
_This how to was written from the viewpoint of a brand new 5.2 "Third Person" template. Many of these steps may be unnecessary for your project, this is for the purpose of documenting the entire process without leaving anything out._

_These instructions are based on 1.0.0 but should be similar for 2.0.0_

### Create New Project
1. Launch Unreal 5.2 and create C++ Third Person template named `PushPawnProject`
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
  * Any time your character de-syncs, it will draw red/green capsules representing the correction, we now know if this plugin can de-sync us or not and how it handles de-sync when it should occur

### AI Setup
1. Duplicate `BP_ThirdPersonCharacter` and name it `BP_ThirdPersonBot`
1. Change it's MaxWalkSpeed to 100cm/s for easier testing
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
1. Extend `IAbilitySystemInterface` from `PushPawnProjectCharacter`
  * This is confirmed working with `UAbilitySystemComponent` on `APlayerState`
1. Add `UAbilitySystemComponent* AbilitySystemComponent` to `PushPawnProjectCharacter`
  * `AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));`
  * `AbilitySystemComponent->SetIsReplicated(true);`
  * `AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);`
1. Override `virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;` to return `AbilitySystemComponent`
1. Create `PushPawnAssetManager` derived from `UAssetManager`
1. Override `virtual void StartInitialLoading() override;` and call `UAbilitySystemGlobals::Get().InitGlobalData();`

### PushPawn
1. Extend `PushPawnProjectCharacter` to `MyPlayerCharacter` and also `MyBotCharacter`
1. `MyBotCharacter` extend `IPusherTarget`
1. `MyPlayerCharacter` extend `IPusheeInstigator`

#### MyBotCharacter
```cpp
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> PushAbilityToGrant;

	//~ Begin IPusherTarget Interface
	virtual void GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder) override
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
	virtual void CustomizePushEventData(const FGameplayTag& PushEventTag, FGameplayEventData& InOutEventData) override {}
	virtual bool CanPush(const AActor* PusheeActor) const override { return !IsPendingKillPending(); }
	//~ End IPusherTarget Interface
```

#### MyPlayerCharacter
```cpp
	//~ Begin IPusherTarget Interface
	virtual bool CanBePushed(const AActor* PusherActor) const override { return !IsPendingKillPending(); }
	//~ End IPusherTarget Interface
```

### Ability
1. Add `UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TSubclassOf<UGameplayAbility> PushScanAbilityClass;` to `AMyPlayerCharacter`
1. Override `PossessedBy` and call:
  * `AbilitySystemComponent->InitAbilityActorInfo(this, this);`
  * `AbilitySystemComponent->K2_GiveAbility(PushScanAbilityClass);`
1. Compile and launch editor

### Assignments
1. Reparent `BP_ThirdPersonCharacter` to `AMyPlayerCharacter`
1. Assign `PushScanAbilityClass` -> `GA_PushPawn_Scan`
1. Reparent `BP_ThirdPersonBot` to `AMyBotCharacter`
1. Assign `PushAbilityToGrant` -> `GA_PushPawn_Action`
1. Open `GA_PushPawn_Scan` and change `TraceChannel` to `PushPawn`
  * You probably want to duplicate `GA_PushPawn_Scan` instead of modifying the original

### Asset Manager
1. Open Project Settings and change `Asset Manager Class` to `PushPawnAssetManager`
1. Restart Editor

### Activate Ability
1. Open `BP_ThirdPersonCharacter` and add a `Sequence` before `Is Locally Controlled`
1. `Sequence` -> `1` -> `Delay 2.0` -> `Try Activate Ability by Class` -> `Push Scan Ability Class`
  * No sane game uses a `Delay` node to ensure initialization, this is only for testing, if the PIE initialization takes too long this will never activate

### Optional
Duplicate the gameplay ability from the content folder to your project and extend as required. If you don't need to extend it, no need to do this step.

Any character that can be pushed must implement the IPusheeTarget interface (C++ only)
Any character that can do the pushing must implement the IPusherTarget interface (C++ only)

In `GA_Push_Action`, assign a trace channel. You probably do not want to leave this as `ECC_Visibility`, but instead setup a channel that only valid Pushers will block.

## Changelog

### 2.1.0-beta
* Support non-capsule collision shapes to allow for Pawn v Pawn collisions to use custom shape types such as boxes

### 2.0.2-beta
* Remove dependency on ACharacter/UCharacterMovementComponent
	* Built-in Push ability still maintains this dependency because it uses root motion sources
	* Acceleration is normalized 
	* Remove searching for UCapsuleComponent* as root
* Add UPushStatic getters using IPusheeInstigator instead of APawn to reduce casting
* Add IPusheeInstigator::GetPusheeCapsuleShape() allowing non-capsule root components, as well as using a capsule that doesn't represent the collision
* Add IPusheeInstigator::GetPusheeVelocity() to remove need to casting to Pawn to get it
* Added sanity checks for 0 scan range
* Refactor ability tasks
	* AbilityTask_WaitForPushTargets ➜ UAbilityTask_PushPawnScan_Base
	* UAbilityTask_WaitForPushTargets_CapsuleTrace ➜ UAbilityTask_PushPawnScan
	* UAbilityTask_GrantNearbyPush ➜ UAbilityTask_GrantPushAbility

### 2.0.1-beta
* Ability itself pauses along with the scan ability, aimed at preventing ASC from continuing to tick, pending testing
* Possible engine crash fix for ability tasks

### 2.0.0-beta
_BREAKING CHANGES_
_Significant perf improvements_

* Overhauled the internal system
	* Created lightweight ability classes with details customization to reduce perf overhead
		* PushPawn_Ability as the parent class for all abilities
		* PushPawn_Scan_Base
		* PushPawn_Ability_Base
	* Created C++ variations of classes to remove overhead from BP VM
		* PushPawn_Scan
		* PushPawn_Ability
		* Content is included primarily for example purposes and non-C++ users
	* Previous ability classes have been removed, including the corresponding content
		* GameplayAbility_Push_Scan removed in favour of PushPawn_Scan_Base, UPushPawn_Scan_Core, and PushPawn_Scan
	* Instead of sending the direction as an FHitResult's Normal, a lighter class with only a normal vector has been added (FPushPawnAbilityTargetData)
	* Added pre-activation interface checks that don't require pusher - use to check if alive, etc. to prevent activation entirely
		* IPusheeInstigator: CanBePushed() refactored to CanBePushedBy(), and IsPushable() added
		* IPusherTarget: CanPush() refactored to CanPushPawn(), and IsPushCapable() added
	* Added TRACE_CPUPROFILER_EVENT_SCOPE throughout for profiling
	* Added IPusheeInstigator::GetPushPawnScanPausedDelegate() that can be used to conditionally disable scanning when no one is nearby

* Improved synchronization
	* Scan task awaits net sync intermittently, see UPushPawn_Scan_Base::ShouldWaitForNetSync() and ConsumeWaitForNetSync() to customize behaviour

* Quality of life changes
	* Improved overall feel of default values
	* Added FPushPawnActionParams & FPushPawnScanParams structs
	* Improved handling for runtime parameters
	* Overhauled handling of scan rate and range to allow for runtime changes based on acceleration
	* Added PushPawnTags to better define reusable native gameplay tags
	* Categories changed from Push to PushPawn
	* General refactoring to improve readability
	* Improved code comments throughout

### 1.3.1
* Add Icon

### 1.3.0
_If you're on 1.2.0 you need to update_

* Refactor GA_Push_x to GA_PushPawn_x
* Push force curve no longer normalized, better defaults set
* Fixed bug where valid direction wasn't obtained when on top of each other
* Update blueprint content with changes
* Brought forward fixes from 1.2.0 that were accidentally not fully implemented or missed - if on 1.2.0 you must update
* Enabled gravity during root motion force by default
* Added helper functions to GA_PushPawn_Action

### 1.2.0
* Fix major init order bug
* Add radius scaling under acceleration
* Add radius scaling under velocity
* Add separate scan rate under acceleration
* Remove unused scan range
* Fix starting ability before actor has begun play

### 1.1.2
* Switch to Cvar to show debug

### 1.1.1
* Add missing include

### 1.1.0
* Move static helpers from `GameplayAbility_Push_Action` to `PushStatics`
* Fix include paths and const correctness
* Add inline cpp includes

### 1.0.0
* Initial Release