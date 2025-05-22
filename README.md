# CREW Network Framework

*CREW Network Framework* is an Unreal Engine plugin that creates a communication between different Unreal Applications. 
 
**It allows:**
 * Multicasting and unicasting between Unreal applications and projects
 * Broadcasting pose replication over network
 * Client to client commands system

The **advantages of this system** are:
* it works on Embbed apps on headsets for example
* it eases the multiplayer set up for a multiple headseats VR experience
* it doesn't saturate the network with broadcasted packages

***
Requirements:
- Unreal Engine 5.3
- [Download](https://github.com/CREW-Brussels/CREWNetworkFramework.git) the plugin's folder in the Plugins folder of your project here or if your project is on git, add this command in the Plugins folder of your project:
```
git add submodule https://github.com/CREW-Brussels/CREWNetworkFramework.git
```
***
## How it works

As soon as the plugin is in the project, the plugin is implemented in the Engine Subsystem and it will create a network discovery of other instances:
- Unreal applications and projects can receive and send their application name: applications with the same name will communicate the messages with each other. You can edit it in project settings > Plugins > CREW Network > Application Name 

![Screenshot 2025-05-21 165601](https://github.com/user-attachments/assets/c13125ef-baa0-49a5-a815-27365d4f6835)

ICI EXPLIQUER MIEUX LE TRUC DE AUTOCONNECT OU PAS QUE G TJRS PAS COMPRIS
***
## Use case #1: Broadcast Pose Replication over Network 
Directly with an Animation Blueprint, the *Replicate Pose* node allows live replication of an animation, wether it uses an animation from a MOCAP suit or any other type of animation, at the only condition that the streamed skeleton mesh and the receiver **posses the same Skeleton**.

### Animation Blueprint for the sender

1. Create an Animation Blueprint with your animation sequence or a live animation. Here, we called it *ABP_Broadcaster*

![Screenshot 2025-05-21 165734](https://github.com/user-attachments/assets/72b67a74-fd97-412b-8324-67e47578508b)

> Additional note: the animation must be already retargeted and ready to use, documentation on the subject is dependent on what MOCAP or animation system you use, but you may have to modify a few things in the ABP or in some of the node details to have your animation working.

2. Chose a strean name, here we chose "performer" but it could be anything else.
3. Chose your fps, 10 or 20 is good
4. Tick streaming
5. Create a new Blueprint Actor -> Add a new component -> Skeleton mesh component -> Add the ABP you've previously created in the Animation details, and the mesh. In details > optimisation, make sure the Visibility Based Anim Tick Option is on *Always Tick Pose and Refresh Bones*. This will allow the animation data to be sent, even if the editor, om play mode, is not targeted to the animation or if it's hidden in game.

![Screenshot 2025-05-22 165456](https://github.com/user-attachments/assets/6512018e-0cfa-47fd-8bb9-535e0009f5ce)

![Screenshot 2025-05-22 165515](https://github.com/user-attachments/assets/e3f30596-5efd-45af-a126-ad7d1806baa8)

6. Your animation is ready to be broadcasted over multiple UE applications!
