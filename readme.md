# My Robot - Crawl the ball using robot's arms

## requirement
- c++ 11
- opengl 3.2
- freeglut
- glew

## build
- Use `make` command to compile.
- make `clean` & `make clean_object` to clean.

## run
There are two modes: 'Freedom Mode' & 'Fetch Mode'

### Freedom Mode
Execute the binary file directly

```shell
$ ./myrobot
```

### Fetch Mode
```shell
$ ./myrobot old_x old_y old_z new_x new_y new_z <-tv | -sv>
# e.g.
$ ./myrobot 1 1 1 2 2 2 -tv
```

The last argument specifies whether “top” (-tv) or “side view” (-sv) is used.

When the program is run, it should first display a solid sphere (with its material  and reflectance properties at your choice) centered at (old_x, old_y, old_z) whose diameter matches the side length of top face of the upper arm (see figure below).
Then starting from its initial position, the robot base and arms should undergoappropriate motion (i.e., rotations) so that the tip of the upper arm touches the sphere. At this moment, the sphere should be picked up and attached to the upper arm and then it is moved, by having the robot base and arms undergo appropriate motions, to the new location (new_x, new_y, new_z). When this is done, the robot should return to its initial position and the sphere should remain at its new location (it does not drop due to gravity).

Note: NO collision detection is used!

## control
`right click` to open menu. In the menu you can:
- Change which arm to control
- Switch view between top / side view

`arrow key (left / right)` to rotate the arm you selected.

`esc` to exit.
