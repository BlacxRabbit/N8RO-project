# N8RO Student Human Animations

This project implements a custom N8RO simulation plugin that drives a Nathan human GLB character with student-defined animation states.

## Motion States

The model registers and plays three custom motion states:

1. `Right Arm Wave`
   - The character raises the right arm and performs a controlled waving motion.
2. `Walk In Place`
   - The character alternates left and right leg movement to demonstrate a walking-in-place cycle.
3. `Both Arms Raise`
   - The character raises and lowers both arms together.

The companion Lua mission script, `student_animation_loop.lua`, switches between these states every 3 seconds.

## Controlled Joints

The animation model controls these 10 joints:

1. `rightShoulder`
2. `rightElbow`
3. `leftShoulder`
4. `leftElbow`
5. `leftHip`
6. `rightHip`
7. `leftKnee`
8. `rightKnee`
9. `leftAnkle`
10. `rightAnkle`

## Project Contents

- `src/SimStudentHumanAnimationsPlugin.cpp`: animation model implementation and animation registration.
- `include/SimStudentHumanAnimationsPlugin.h`: plugin class declaration.
- `student_animation_loop.lua`: N8RO mission script that cycles through the three motion states.
- `assets/human_model_nathan.glb`: GLB human model used for the demonstration.
- `bin/release/sim-student-human-animations.dll`: built plugin DLL for N8RO.

## Build

Requirements:

- N8RO installed at `C:\N8RO`
- Visual Studio 2022 Build Tools with C++ tools

Build command:

```bat
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" sim-student-human-animations.vcxproj /p:Configuration=Release /p:Platform=x64 /p:N8RO_RELEASE=C:\N8RO
```

Build output:

```bat
bin\release\sim-student-human-animations.dll
```

## Install In N8RO

Copy the plugin DLL to:

```bat
C:\N8RO\userPlugins\sim\sim-student-human-animations.dll
```

Copy the Lua mission script to:

```bat
C:\N8RO\resources\missions\student_animation_loop.lua
```

In N8RO, load the `GenericCivilianPresence` scenario and assign the mission script to the civilian entity. The GLB viewer should show the human character cycling through `Right Arm Wave`, `Walk In Place`, and `Both Arms Raise`.

## Submission Video

Screen capture video:

```text
demo/MOTIONVIDEO.mp4
```

The screen capture video should show:

- The selected motion states: `Right Arm Wave`, `Walk In Place`, `Both Arms Raise`
- The human character performing the motions in the N8RO GLB viewer
- The custom plugin-driven animation states, not only the default idle animation
