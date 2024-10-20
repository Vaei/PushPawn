# Push Pawn <img align="right" width=128, height=128 src="https://github.com/Vaei/PushPawn/blob/main/Resources/Icon128.png">

> [!IMPORTANT]
> **Net-predicted soft organic collisions!**
> <br>Which eliminates inappropriate de-syncing when running near other pawns in multiplayer games
> <br>And results in great game feel!

> [!TIP]
> <br>Suitable for both singleplayer and multiplayer games

## Features
* Net predicted Pawn vs. Pawn collisions via GAS to prevent de-sync
* Soft organic collisions instead of the engine's brick wall collisions
* Capsule, Box, Sphere collisions all supported for Pawn v Pawn collisions (even with CMC)
* Abilities that can respond to gameplay events, e.g. playing a push animation, procedural skeletal physics, or yelling at the other Pawn, etc.
* Allows pawns to push each other out of the way - great for groups of NPCs attacking a player
* Lets players push each other off cliffs, if you want that...
* Exceptionally customizable

> [!NOTE]
> Fully tested in production with both Player-Controlled Characters and AI-Controlled Characters and latency exceeding 200ms

## How To Use

> [!TIP]
> ***Due to the use of Git LFS, do not download a zip or your content will be missing***
> <br>You will need to clone this via `git clone https://github.com/Vaei/PushPawn.git` if you want the content
> <br>Using C++ only is viable also as of `2.0.0`

> [!IMPORTANT]
> [Read the Wiki to Learn How to use Push Pawn](https://github.com/Vaei/PushPawn/wiki)

## Example
_Examples are based on 1.0.0_

Here is the preview from the included minimal Third Person Example project. This footage was obtained with >200ms and `p.netshowcorrections 1` - as you can see, it doesn't de-sync abnormally.

![example usage](https://github.com/Vaei/repo_files/blob/main/PushPawn/preview.gif)

This was initially created for a personal project that would make the AI play a push animation and yell at the player, which is not included, however the helper functions for building this are included.

![example usage](https://github.com/Vaei/repo_files/blob/main/PushPawn/preview_isekai.gif)

## Changelog

### 2.1.2-beta
* PushPawn now compatible with blueprint only projects
* Added optional UPusheeComponent
	* Extended UPusheeComponentHelper to implement BP-exposed IPusheeInstigator interface
	* Added UPushStatics::GetPusheeInstigator as a central method for retrieving IPusheeInstigator
* Rename BlueprintDemo folder to Blueprints

### 2.1.1-beta
* Add PushPawnEditor module
* Add Details Customization to hide properties we removed support for in our lightweight base ability
	* Or irrelevant properties

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