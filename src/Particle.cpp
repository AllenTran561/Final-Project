#include "Particle.h"


Particle::Particle() {

	// initialize particle with some reasonable values first;
	//
	velocity.set(0, 0, 0);
	acceleration.set(0, 0, 0);
	position.set(0, 0, 0);
	forces.set(0, 0, 0);

	rotation = 0;
	angularVelocity = 0;
	angularAcceleration = 0;
	angularForce = 0;

	lifespan = 5;
	birthtime = 0;
	radius = .1;
	damping = .99;
	mass = 1;
	color = ofColor::aquamarine;
}

void Particle::draw() {
//	ofSetColor(color);
	ofSetColor(ofMap(age(), 0, lifespan, 255, 10), 0, 0);
	ofDrawSphere(position, radius);
}

// write your own integrator here.. (hint: it's only 3 lines of code)
//
void Particle::integrate() {

	
	// interval for this step
	//
	float dt = 1.0 / ofGetFrameRate();

	// update position based on velocity
	//
	position += (velocity * dt);

	// update acceleration with accumulated paritcles forces
	// remember :  (f = ma) OR (a = 1/m * f)
	//
	ofVec3f accel = acceleration;    // start with any acceleration already on the particle
	accel += (forces * (1.0 / mass));
	velocity += accel * dt;

	// add a little damping for good measure
	//
	velocity *= damping;

	rotation += (angularVelocity * dt);
	float a = angularAcceleration;
	a += (angularForce * 1.0 / mass);
	angularVelocity += a * dt;
	angularVelocity *= damping;

	// clear forces on particle (they get re-added each step)
	//
	forces.set(0, 0, 0);
	angularForce = 0;
}

void Particle::addForces(glm::vec3 f) {
	forces = f;
}
void Particle::addAngularForces(float f) {
	angularForce = f;
}

ofVec3f Particle::heading() {
	glm::mat4 rot2 = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	return glm::normalize(rot2 * glm::vec4(0, 0, -1, 1));
}

//  return age in seconds
//
float Particle::age() {
	return (ofGetElapsedTimeMillis() - birthtime)/1000.0;
}


