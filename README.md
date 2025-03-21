# The Neon Shadows - Diploma Project in Unreal Engine 5

## About the project
### Core Idea
The following project inspired by the game Ghostrunner is a mix of parkour and precise Katana combat allowing players to quicly kill their opponents while using the enviroment to their advantage. With the one-hit-one-kill mechanic players can feel unstoppable, while still thinking ahead to make perform the most approprite combo to survive each encounter. If they fail, once per level, they can rewind the time to take a step back and return to action without needing to respawn.
So slide, run and slice until nothing is left alive and compare your score with your friends on leaderboards thanks to Steam Integration and Epic Leaderboard.

### Gameplay
<p align="center">
  <img src="https://github.com/user-attachments/assets/32cb9947-4b84-4774-a1ed-72890f531463" width = "800">
</p>

## Game Features
* Fluid Parkour - players can perform various moves including wallrun, sliding and mantle.
* Katana Combat - mainly consists of dynamic sword swings, directional camera shake and automatic guided dash to target
* Perfect Parry - if timed right, melee attack can be parried to temporarily stun enemies and bullets can be deflected back to shooter
* Special Abilities - include Time Stop ability and Percision Strike 
* Time Rewind - a second chance, that activetes upon player's first death allowing to rewind the time few seconds
* Behaviour Tree powered Enemies - two types of melee enemies, one ranged and one protected by shield
* Leaderboards via Epic Leaderboard - a system allowing for players score to be saved and displayed on a online leaderboard

## Tools
Project was created in Unreal Engine 5.3.2 with addition of
* C++
* Blueprints
* Control Rig
* Behaviour Trees

## Highlighed Tech

### Procedural Hand Animation in Control Rig
Our procedural animation system allows to position both hands in the game viewport allowing for the two handed Katana grip. It allowed us to improve the quality of the animations that were at our disposal. Furhtermore, the system can procedurally adjust the combat animations trajectory, so that any point withing the player characters arm range can be targeted and hit. It was implememted inside the Unreal Engines Control Rig, by using Two Bone IK, vector math includind vector projection and rejection as well as quaterions to calculate correct hands rotation along the sword grip.

### Time Rewind
System allows to give player a second change once they made a fatal mistake. During gameplay, for each actor that can be affected by Time Rewind, it saves the snapshots containing the basic actor information such as location and velocities as well a flag whether or the position is safe (not in the air for instance). Each snapshot is stored in a Rind Buffer, which allows for overwriting old snapshots when the structure becomes full. When the rewind process is initiated, for each actor all of the available snapshots are interpolated, giving the illusion of reversing the time.

[//]: <> (### Steam + Epic leaderboards)
[//]: <> (todo)


## Authors
Project was created by the group of 4 programmers over the course of two semesters of PJAIT Game Developement specialization area.
