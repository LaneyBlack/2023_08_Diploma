# The Neon Shadows - Diploma Project in Unreal Engine 5

## About the project
### Core Idea
The following project inspired by the game **Ghostrunner** is a mix of parkour and precise Katana combat allowing players to quicly kill their opponents while using the enviroment to their advantage. With the one-hit-one-kill mechanic players can feel unstoppable, while still thinking ahead to make perform the most approprite combo to survive each encounter. If they fail, once per level, they can rewind the time to take a step back and return to action without needing to respawn.

So _slide, run and slice_ until nothing is left alive and compare your score with your friends on leaderboards thanks to Steam Integration and Epic Leaderboard.

### Gameplay
<p align="center">
  <img src="https://github.com/user-attachments/assets/32cb9947-4b84-4774-a1ed-72890f531463" width = "800">
</p>

## Game Features
### Fluid Parkour
A gameplay system that allows players to perform various moves including
* **Wallrun** that can be perfomed on any vertical surface
* **Slide** is used to glide under obstacles or gain a significant speed boost on slopes
* **Mantle** which can help players climb obstacles or reach higher platforms with ease
### Katana Combat
A system that enhances melee combat encounters with:
* **Dynamic sword swings** for fluid and responsive attacks
* **Directional camera shake** to amplify impact and immersion that matches the direction of combat animation
* **Automatic guided dash** that swiftly propels players toward their target giving the sense of speed and agility
### Perfect Parry 
A defensive mechanic that, when executed with precise timing:  
* **Counters melee attacks**, temporarily stunning enemies.  
* **Deflects bullets**, sending them back to the shooter.
### Special Abilities
Unique enhancements to gameplay trigger by specific amount of Combo Points, including:  
* **Time Stop** – Temporarily halts time for each enemy, allowing for strategic movement and attacks.  
* **Precision Strike** – Triggers a targeting mode, where up to 4 enemies can be targeted and chain-eliminated. 
### Time Rewind
A special mechanic that grants a second chance, activating upon the player’s first death. It allows the player to rewind time by a few seconds, giving them the opportunity to avoid the fatal mistake without needing to restart the checkpoint.
### Behaviour Tree powered Enemies
Enemies with advanced AI, driven by behavior trees, including:  
* **Melee Enemies** – Engage in close combat with aggressive tactics. Split into two types: the regular and teleporting that rapidly dashes towards the player  
* **Ranged Enemy** – Attacks from a distance with a rifle, using precision and evasion.  
* **Shielded Enemy** – Protected by a shield, requiring different strategy as it can only be defeated from the back.
### Online Services
A set of features that enhance connectivity and player progression, including:  
* **Leaderboard Integration** – Allows players' scores to be saved and displayed on an online leaderboard for global competition.  
* **Steam Profile Integration** – Automatically detects the logged-in user and binds their Steam profile to game saves, ensuring seamless tracking and synchronization.

## Tools
Project was created in Unreal Engine 5.3.2 with addition of
* C++
* Blueprints
* Control Rig
* Behaviour Trees

## Highlighed Tech

### Procedural Hand Animation in Control Rig
Our procedural animation system allows to position both hands in the game viewport allowing for the two handed Katana grip. It allowed us to improve the quality of the animations that were at our disposal. Furhtermore, the system can procedurally adjust the combat animations trajectory, so that any point withing the player characters arm range can be targeted and hit. It was implememted inside the Unreal Engines Control Rig, by using Two Bone IK, vector math includind vector projection and rejection as well as quaterions to calculate correct hands rotation along the sword grip.

<p align="center">
  <img src="https://github.com/user-attachments/assets/d404c6e9-bb12-4004-a944-87fcc4888186" width = "600">
</p>

### Time Rewind
System allows to give player a second change once they made a fatal mistake. During gameplay, for each actor that can be affected by Time Rewind, it saves the snapshots containing the basic actor information such as location and velocities as well a flag whether or the position is safe (not in the air for instance). Each snapshot is stored in a Rind Buffer, which allows for overwriting old snapshots when the structure becomes full. When the rewind process is initiated, for each actor all of the available snapshots are interpolated, giving the illusion of reversing the time.

<p align="center">
  <img src="https://github.com/user-attachments/assets/9e69af87-4453-4671-9b4f-a1ef765e6b05" width = "600">
</p>

[//]: <> (### Steam + Epic leaderboards?)
[//]: <> (todo)


## Authors
Project was created by the group of 4 programmers over the course of two semesters of PJAIT Game Developement specialization area.
