#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter() {
	sys = new ParticleSystem();
	createdSys = true;
	init();
}

ParticleEmitter::ParticleEmitter(ParticleSystem *s) {
	if (s == NULL)
	{
		cout << "fatal error: null particle system passed to ParticleEmitter()" << endl;
		ofExit();
	}
	sys = s;
	createdSys = false;
	init();
}

ParticleEmitter::~ParticleEmitter() {

	// deallocate particle system if emitter created one internally
	//
	if (createdSys) delete sys;
}

void ParticleEmitter::init() {
	rate = 1;
	velocity = ofVec3f(0, 20, 0);
	lifespan = 3;
	started = false;
	oneShot = false;
	fired = false;
	lastSpawned = 0;
	radius = 1;
	particleRadius = .1;
	visible = true;
	type = SingleEmitter;
	groupSize = 1;
	position = ofVec3f(0, 0, 0);
	mass = 1;
	damping = .99;
	particleColor = ofColor::red;

}

void ParticleEmitter::draw() {

	Particle particle;

	if (visible) {
		switch (type) {

		case DirectionalEmitter:
			particle.velocity = velocity;
			particle.position.set(position);  // just draw a small sphere for point emitters 
			break;
		case SphereEmitter:
		case RadialEmitter:
			ofDrawSphere(position, radius/10);  // just draw a small sphere as a placeholder
			break;
		default:
			break;
		}

	}
	sys->draw();  
}

void ParticleEmitter::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

void ParticleEmitter::stop() {
	started = false;
	fired = false;
}
void ParticleEmitter::update() {

	float time = ofGetElapsedTimeMillis();

	if (oneShot && started) {
		if (!fired) {
			// spawn a new particle(s)
			//
			for (int i = 0; i < groupSize; i++)
				spawn(time);

			lastSpawned = time;
		}
		fired = true;
		stop();
	}
	else if (SingleEmitter) {

	}
	else if (((time - lastSpawned) > (1000.0 / rate)) && started) {

		// spawn a new particle(s)
		//
		for (int i= 0; i < groupSize; i++)
			spawn(time);
	
		lastSpawned = time;
	}
	sys->update();
}

// spawn a single particle.  time is current time of birth
//
void ParticleEmitter::spawn(float time) {

	Particle particle;

	// set initial velocity and position
	// based on emitter type
	//
	switch (type) {
	case RadialEmitter:
	{
		ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
		float speed = velocity.length();
		particle.velocity = dir.getNormalized() * speed;
		particle.position.set(position);
		particle.lifespan = lifespan;
	}
	break;
	case SphereEmitter:
		break;
	case DirectionalEmitter:
		particle.velocity = velocity;
		particle.position.set(position);
		particle.lifespan = lifespan;

		break;
	//Lander Emitter of 1 particle
	case SingleEmitter:
		particle.velocity = glm::vec3(0, 0, 0);
		particle.position.set(glm::vec3(0,0,0));
		//Makes particle last forever
		particle.lifespan = -1;

	}

	// other particle attributes
	//
	particle.birthtime = time;
	particle.radius = particleRadius;

	// add to system
	//
	sys->add(particle);
}