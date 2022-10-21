# computer-animation-simulation

## Lab 2: Cloth Simulation

![Lab2 Simulation](./images/Lab2/Lab2.gif)

### Collisions

#### Particle-collider DONE

##### Sphere Collision
![Cloth Sphere Collision](./images/Lab2/sphere-collision.png)

##### Box Collision
![Box Sphere Collision](./images/Lab2/box-collision.png)

#### Prevent self-intersections
![Self Collision](./images/Lab2/self-collision.png)

##### Use Particles & Particle Hash DONE

##### Consider Rest Distance and avoid Jittering DONE

##### Use Substepping, not CCD DONE

##### Stable Cloth-Cloth Friction DONE

### Interactions

#### Grab and move particles DONE

![Grab & Move Particles](./images/Lab2/grab-and-move-particles.gif)


#### Fix/Release particles DONE

![Fix & Release Particles](./images/Lab2/fix-and-release-particles.gif)


#### Blackhole DONE

![Blackhole Interaction](./images/Lab2/blackhole-interaction.gif)
---

## Lab 1: Particle Systems
I have chosen the Midpoint Integrator since it has more appealing results when activating the blackhole gravitational attraction force with the lowest computation (RK2 and RK4 also give appealing results).

![Lab1 Simulation](./images/Lab1/Lab1.gif)

### Particle systems with 1000s of particles DONE
Tested with 2000 particles smoothly in release mode

### Collisions: test & response DONE
- Plane
- AABB
- Sphere

### Gravity + another force DONE
- Gravity
- Linear Drag Force
- Quadratic Drag Force
- Gravitational attraction (Blackhole)

### Mouse interaction DONE
By selecting the object in the UI, we can move it by dragging the mouse and pressing the right-click

### Bonus: Spatial Hash Table DONE
### Bonus: Snowball Scene DONE

![Lab0 Simulation](./images/Lab1/SnowballScene.gif)

---

## Lab 0: Warm Up

![Lab0 Simulation](./images/Lab0/Lab0.png)

### Integrators DONE
The integrator solvers can be chosen from the comboBox next to each SolverN at the UI and pressing restart.

- Symplectic Euler
- Midpoint
- Verlet
- RK2
- RK4

### Forces DONE
Wind force can be set in the UI as accelerations in each X,Y,Z axis. As for the air resistance, you can choose between no drag, linear drag, and quadratic drag.

- Wind as constant acceleration force
- Linear Drag
- Quadratic Drag

### SceneProjectiles DONE

- Added an additional particle for more testing
- Simple collision to plane with condition y<=0
- Added bouncing and friction coefficients editable in UI
