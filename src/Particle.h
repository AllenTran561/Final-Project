#pragma once

#include "ofMain.h"

class ParticleForceField;

class Particle {
public:
	Particle();

	ofVec3f position;
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float	angularVelocity;
	float	angularAcceleration;
	float	angularForce;
	float	rotation;
	float	damping;
	float   mass;
	float   lifespan;
	float   radius;
	float   birthtime;
	void	addForces(glm::vec3);
	void	addAngularForces(float f);
	ofVec3f heading();
	void    integrate();
	void    draw();
	float   age();        // sec
	ofColor color;
};


