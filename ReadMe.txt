Final Project TEAM# 11

Team Members
  Amy Tsao UID: 604031740
  Andrew Adam UID: 803886818
  Jiayi Chen UID: 004080700
  Hee Hwang(Peter) UID: 804212513

(* Brief Overview *)
This is a maze with eight different levels. Each level has its own
map and texture. The maze can be navigated through a variety of means,
and has various functions. When all levels are completed, user will
be asked whether or not they wish to restart the game.

(* List of implemented interactions *)

/* Navigation */
1.) ARROW KEYS: Press 'UP_ARROW','DOWN_ARROW', 'LEFT_ARROW', and 
'RIGHT_ARROW' to navigate the map.
2.) MOTION: Move a white object to different parts of the boundary 
window to move in the corresponding direction in the maze.
3.) MOUSE: Clicking on a certain section of the maze window to move in 
the corresponding direction.
	- the screen is divided into a 3 by 3 grid (9 equal parts), 
	and clicking on that section of the screen will cause the 
	player to move as follows:

		N/A  | UP   | N/A
		LEFT | N/A  | RIGHT
		N/A  | DOWN | N/A

/* Other keyboard features */

o, O			toggles between prospective and orthogonal view
t				toggle between texture and no texture on the walls
c, C			turn camera detection on and off (2nd way of navigating)
q, Q, ESC		Exit the program
f, F			Toggle fullscreen
+, -			Increace/decrease zoom while in orthogonal view
n, N			Next level
s, S			Replay spin maze animation
e, E			Replay enter maze animation

Basic Topics (Lectures 1-6):
- Drawing QUAD-strip
- Lighting
- Texture mapping via file (Tri-linear filtering)
- Timer for constant speed between different computers
- User and System Event Handling
- Viewports, Points and Coordinate Systems
- Transformations
- Viewing and Projection
- Camera

Advanced Topics:
- Collision detection
- Picking