# BP_BeginnersGuideManager

**Asset Path**: `/Game/VRGamePlay/BeginnersGuide/BP_BeginnersGuideManager.BP_BeginnersGuideManager`

---

## Metadata

- **Class**: BP_BeginnersGuideManager
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent

---

## Variables

- VRPawn : BP_VRPawn_C (Public) // VRPawn
- Controller Viz MIDLeft : MaterialInstanceDynamic (Public) // Controller Viz MIDLeft
- Controller Viz MIDRight : MaterialInstanceDynamic (Public) // Controller Viz MIDRight
- MenuActor : BP_BeginnersGuideMenu_C (Public) // Menu Actor
- LastFlickeringButtonName : E_ControllerButton (Public) // Last Flickering Button Name

---

## Functions

- UserConstructionScript -> void
- ButtonNameEnumToString -> string ( EnumValue: E_ControllerButton )
- Event BeginPlay (Event) -> void
- ButtonStartFlickering (Event) -> void ( WhichButton: E_ControllerButton, IsRight?: bool )
- ButtonStopFlickering (Event) -> void ( WhichButton: E_ControllerButton, IsRight?: bool )
- ToggleText (Event) -> void ( InitialText: text, Open?: bool )
- Change Text (Event) -> void ( Text: text )
- ButtonStartFlickeringBothHand (Event) -> void ( WhichButton: E_ControllerButton )
- ButtonStopFlickeringBothHand (Event) -> void ( WhichButton: E_ControllerButton )
- LastButtonStopFlickering (Event) -> void ( IsRight?: bool )
- LastButtonStopFlickeringBothHand (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 2

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event BeginPlay**

```blueprint
* Event BeginPlay
    * Cast (GetPlayerPawn()) To BP_VRPawn
        |-- then:
        |   * Set VRPawn:BP_VRPawn = Cast<BP_VRPawn>(GetPlayerPawn())
        |       * Set Controller Viz MIDLeft:MaterialInstanceDynamic = ControllerVizMIDLeft
        |           * Set Controller Viz MIDRight:MaterialInstanceDynamic = ControllerVizMIDRight
        |               * [Path ends]
        L-- CastFailed:
            * PrintString((InString=Not VRPawn)) (Target: KismetSystemLibrary)
                * [Path ends]
```
---

**Trace Start: ButtonStartFlickering
Custom Event** (CustomEvent)

```blueprint
* Event ButtonStartFlickering Args: (IsRight?:bool, WhichButton:E_ControllerButton)
    * (Select(Index=ButtonStartFlickering.IsRight?, Options={Option 0=Controller Viz MIDLeft, Option 1=Controller Viz MIDRight})).SetScalarParameterValue((ParameterName=Conv_StringToName(InString=(([ButtonNameEnumToString](#bp-beginnersguidemanager-buttonnameenumtostring)(EnumValue=ButtonStartFlickering.WhichButton)).StringValue + Flicker)), Value=1))
        * Set LastFlickeringButtonName:E_ControllerButton = ButtonStartFlickering.WhichButton
            * [Path ends]
```
---

**Trace Start: ButtonStartFlickeringBothHand
Custom Event** (CustomEvent)

```blueprint
* Event ButtonStartFlickeringBothHand Args: (WhichButton:E_ControllerButton)
    * Call Custom Event: [ButtonStartFlickering](#bp-beginnersguidemanager-buttonstartflickering)(IsRight?=true, WhichButton=ButtonStartFlickeringBothHand.WhichButton)
        * Call Custom Event: [ButtonStartFlickering](#bp-beginnersguidemanager-buttonstartflickering)(WhichButton=ButtonStartFlickeringBothHand.WhichButton)
            * [Path ends after call to custom event "ButtonStartFlickering"]
```
---

**Trace Start: ButtonStopFlickering
Custom Event** (CustomEvent)

```blueprint
* Event ButtonStopFlickering Args: (IsRight?:bool, WhichButton:E_ControllerButton)
    * (Select(Index=ButtonStopFlickering.IsRight?, Options={Option 0=Controller Viz MIDLeft, Option 1=Controller Viz MIDRight})).SetScalarParameterValue((ParameterName=Conv_StringToName(InString=(([ButtonNameEnumToString](#bp-beginnersguidemanager-buttonnameenumtostring)(EnumValue=ButtonStopFlickering.WhichButton)).StringValue + Flicker))))
        * [Path ends]
```
---

**Trace Start: ButtonStopFlickeringBothHand
Custom Event** (CustomEvent)

```blueprint
* Event ButtonStopFlickeringBothHand Args: (WhichButton:E_ControllerButton)
    * Call Custom Event: [ButtonStopFlickering](#bp-beginnersguidemanager-buttonstopflickering)(IsRight?=true, WhichButton=ButtonStopFlickeringBothHand.WhichButton)
        * Call Custom Event: [ButtonStopFlickering](#bp-beginnersguidemanager-buttonstopflickering)(WhichButton=ButtonStopFlickeringBothHand.WhichButton)
            * [Path ends after call to custom event "ButtonStopFlickering"]
```
---

**Trace Start: LastButtonStopFlickering
Custom Event** (CustomEvent)

```blueprint
* Event LastButtonStopFlickering Args: (IsRight?:bool)
    * Call Custom Event: [ButtonStopFlickering](#bp-beginnersguidemanager-buttonstopflickering)(IsRight?=LastButtonStopFlickering.IsRight?, WhichButton=LastFlickeringButtonName)
        * [Path ends after call to custom event "ButtonStopFlickering"]
```
---

**Trace Start: LastButtonStopFlickeringBothHand
Custom Event** (CustomEvent)

```blueprint
* Event LastButtonStopFlickeringBothHand
    * Call Custom Event: [LastButtonStopFlickering](#bp-beginnersguidemanager-lastbuttonstopflickering)(IsRight?=true)
        * Call Custom Event: [LastButtonStopFlickering](#bp-beginnersguidemanager-lastbuttonstopflickering)()
            * [Path ends after call to custom event "LastButtonStopFlickering"]
```
---

**Trace Start: ToggleText
Custom Event** (CustomEvent)

```blueprint
* Event ToggleText Args: (InitialText:text, Open?:bool)
    * If (ToggleText.Open?)
        |-- true:
        |   * Spawn Actor BP_BeginnersGuideMenu at (DefaultTransform)(Spawn Transform Scale=Vector(X=1, Y=1, Z=1), Text=ToggleText.InitialText)
        |       * Set MenuActor:BP_BeginnersGuideMenu = SpawnedActor(BP_BeginnersGuideMenu)
        |           * [Path ends]
        L-- false:
            * VariableGet (Get)
                |-- Is Valid:
                |   * MenuActor.Call Custom Event: [BP_BeginnersGuideMenu.CloseMenu](#bp-beginnersguidemenu-closemenu)()
                |       * [Path ends after call to custom event "BP_BeginnersGuideMenu.CloseMenu"]
                L-- Is Not Valid:
                    * PrintString((InString=MenuNotValid)) (Target: KismetSystemLibrary)
                        * [Path ends]
```
---

**Trace Start: Change Text
Custom Event** (CustomEvent)

```blueprint
* Event Change Text Args: (Text:text)
    * VariableGet (Get)
        |-- Is Valid:
        |   * MenuActor.Call Custom Event: [BP_BeginnersGuideMenu.Set Text](#bp-beginnersguidemenu-set-text)(Text=Change Text.Text)
        |       * [Path ends after call to custom event "BP_BeginnersGuideMenu.Set Text"]
        L-- Is Not Valid:
            * PrintString((InString=MenuNotValid)) (Target: KismetSystemLibrary)
                * [Path ends]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * [Path ends]
```
---

---

### [Function] ButtonNameEnumToString

**Trace Start: Button Name Enum to String** (FunctionEntry)

```blueprint
* Event Button Name Enum to String
    * Return(StringValue=Select(Index=ValueFrom(Button Name Enum to String.EnumValue), Options={NewEnumerator0=Stick, NewEnumerator1=Trigger, NewEnumerator2=Grip, NewEnumerator3=Button1, NewEnumerator4=Button2}))
```
---



---

## References

- /Game/VRGamePlay/BeginnersGuide/UI/BP_BeginnersGuideMenu.BP_BeginnersGuideMenu_C (BP_BeginnersGuideMenu_C) // EventGraph :: Get.MenuActor
- /Game/VRGamePlay/BeginnersGuide/UI/BP_BeginnersGuideMenu.SKEL_BP_BeginnersGuideMenu_C (SKEL_BP_BeginnersGuideMenu_C) // EventGraph :: Close Menu
- /Game/VRGamePlay/Player/BP_VRPawn.BP_VRPawn_C (BP_VRPawn_C) // EventGraph :: Cast To BP_VRPawn.AsBP VRPawn

