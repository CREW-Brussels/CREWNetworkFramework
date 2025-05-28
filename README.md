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
Directly with an Animation Blueprint, the *Replicate Pose* node allows live replication of an animation, wether it uses an animation from a MOCAP suit or any other type of animation, at the only condition that the streamed Skeleton Mesh and the receiver **posses the same Skeleton and have no level of details (Number of LODs must be set at 1).
![Screenshot 2025-05-28 113152](https://github.com/user-attachments/assets/6697122d-df28-42df-9a57-a4d52ef8de3d)

### Animation Blueprint for the sender

1. Create an Animation Blueprint with your animation sequence or a live animation, and the node called *Replicate Pose*. Here, we called it *ABP_Rebroadcaster*

![Screenshot 2025-05-21 165734](https://github.com/user-attachments/assets/72b67a74-fd97-412b-8324-67e47578508b)

> Additional note: the animation must be already retargeted and ready to use, documentation on the subject is dependent on what MOCAP or animation system you use, but you may have to modify a few things in the ABP or in some of the node details to have your animation working.

2. Choose a stream name, here we choose "performer" but it could be anything else.
3. Choose your fps, 10 or 20 is good
4. Tick streaming
5. Create a new Blueprint Actor:
- Add a new component -> Skeleton mesh component
- Add the ABP you've previously created in the Animation details, and add the mesh.
- In details > optimisation, make sure the Visibility Based Anim Tick Option is on *Always Tick Pose and Refresh Bones*. This will allow the animation data to be sent, even if the editor, om play mode, is not targeted to the animation or if it's hidden in game.
- In the event graph, create the following nodes. These nodes make sure that only the server is broadcasting the pose, to not multiply the same replication.
![Screenshot 2025-05-22 164853](https://github.com/user-attachments/assets/e0693b21-ddd5-4612-9124-917f6e3366e0)

- This Blueprint will send the animation once put in a scene.

![Screenshot 2025-05-22 165456](https://github.com/user-attachments/assets/6512018e-0cfa-47fd-8bb9-535e0009f5ce)

![Screenshot 2025-05-22 165515](https://github.com/user-attachments/assets/e3f30596-5efd-45af-a126-ad7d1806baa8)

6. Your animation is ready to be broadcasted over multiple UE applications! Drag and drop the BP you've just created in your scene to broadcast your animation.



### Animation Blueprint for the receiver

1. Create a new Animation Blueprint. Add the node *Replicate Pose*
2. On this node, type in the same stream name as the sender and make sure **Streaming is NOT ticked**

![Screenshot 2025-05-21 170001](https://github.com/user-attachments/assets/4b70a71d-0fa9-4941-b1af-ac3e595b3518)

4. Create a new Blueprint Actor:
- Add a new component -> Skeleton mesh component
- Add the ABP you've just created in the Animation details, and add the mesh.
- This Blueprint will receive the animation.
5. Drag and drop it in your scene to have a skeleton mesh ready to receive the broadcasted animation!

***
## Use case #2: Multicast a list of commands
With the command system of this plugin, any client can send a command that will be visible to any client on the same application name. To do that, you will use one of the nodes send command.
***
For example, if you want to share inputs from the controller of a client, you can send the command over the network. It can be a selection of variable. In the player's BP event graph, you will send commands from different input actions.

![Screenshot 2025-05-21 170535](https://github.com/user-attachments/assets/71ecf888-808f-4649-87a2-395da6d430c4)
![Screenshot 2025-05-21 170440](https://github.com/user-attachments/assets/64bc7a94-2e4f-4990-b7ee-b203ddd8afd3)
![Screenshot 2025-05-21 170428](https://github.com/user-attachments/assets/9ea654b3-fe48-49d2-a376-0ce272a29101)
![Screenshot 2025-05-21 170420](https://github.com/user-attachments/assets/bed2971c-8d21-49cb-a884-c184630a7c96)

Gameplay Tags are a list of user defined tags, that are easy to edit and add. They can then be referenced at different points in the project. It's similar to Enum, but without a specific type. 
![Screenshot 2025-05-28 114639](https://github.com/user-attachments/assets/bf478760-dc15-414a-b6de-3bdaab8f235b)

