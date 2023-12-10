
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   < Allen Tran >
//  Date: <date of last version>


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup() {
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
	//	ofSetWindowShape(1024, 768);
	bDefaultCam = true;
	bThirdPersonCam = false;
	bTopDownCam = false;
	//Default Cam
	cam.setDistance(20);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//

	ofLoadImage(particleTex, "images/dot.png");

	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	#ifdef TARGET_OPENGLES
		shader.load("shaders_gles/shader");
	#else
		shader.load("shaders/shader");
	#endif

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	mars.loadModel("geo/mars-low-5x-v2.obj");
	//mars.loadModel("geo/moon-houdini.obj");

	mars.setScaleNormalization(false);

	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	//	gui.add(velocity.setup("Initial Velocity", ofVec3f(0, 20, 0), ofVec3f(0, 0, 0), ofVec3f(100, 100, 100)));	
	//	gui.add(lifespan.setup("Lifespan", 2.0, .1, 10.0));
	//	gui.add(rate.setup("Rate", 1.0, .5, 60.0));
	gui.add(numParticles.setup("Number of Particles", 15, 0, 50));
	gui.add(lifespanRange.setup("Lifespan Range", ofVec2f(1, 6), ofVec2f(.1, .2), ofVec2f(3, 10)));
	gui.add(mass.setup("Mass", 1, .1, 10));
	gui.add(damping.setup("Damping", .99, .8, 1.0));
	gui.add(gravity.setup("Gravity", 0, -20, 20));
	gui.add(radius.setup("Radius", .1, 1, 10));
	gui.add(turbMin.setup("Turbulence Min", ofVec3f(0, 0, 0), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));
	gui.add(turbMax.setup("Turbulence Max", ofVec3f(0, 0, 0), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));
	gui.add(radialForceVal.setup("Radial Force", 1000, 100, 5000));
	gui.add(radialHight.setup("Radial Height", .2, .1, 1.0));
	gui.add(cyclicForceVal.setup("Cyclic Force", 0, 10, 500));
	bHide = false;

	//  Create Octree for testing.
	//
	float time = ofGetElapsedTimeMillis();
	octree.create(mars.getMesh(0), 10);
	time = ofGetElapsedTimeMillis() - time;
	cout << "Octree Creation Time: " << time << endl;
	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

	colorMap = {
		{1, ofColor::red},
		{2, ofColor::blue},
		{3, ofColor::green},
		{4, ofColor::yellow},
		{5, ofColor::orange},
		{6, ofColor::aquamarine},
		{7, ofColor::purple},
		{8, ofColor::pink},
		{9, ofColor::brown},
		{10, ofColor::white}
	};
	
	//Sets up Gravity and Impulse Force
	gravityForce = new GravityForce(ofVec3f(0, -5, 0));
	neutralForce = new GravityForce(ofVec3f(0, .001, 0));
	impulseForce = new ImpulseRadialForce(0);
	turbulenceForce = new TurbulenceForce(ofVec3f(-5, -5, -5), ofVec3f(5, 5, 5));
	cyclicForce = new CyclicForce(0);


	//Sets up Lander Emitter
	landerEmitter.sys->addForce(gravityForce);
	landerEmitter.setEmitterType(SingleEmitter);
	landerEmitter.setGroupSize(1);
	landerEmitter.start();
	landerEmitter.spawn(ofGetElapsedTimeMillis());

	exhaustEmitter.sys->addForce(turbulenceForce);
	//exhaustEmitter.sys->addForce(gravityForce);
	exhaustEmitter.sys->addForce(impulseForce);
	exhaustEmitter.sys->addForce(cyclicForce);

	//exhaustEmitter.particleColor = ofColor::red;
	exhaustEmitter.setVelocity(ofVec3f(0, -8, 0));
	exhaustEmitter.setOneShot(true);
	//exhaustEmitter.setEmitterType(DirectionalEmitter);
	exhaustEmitter.setEmitterType(RingEmitter);
	//exhaustEmitter.setGroupSize(numParticles);
	//exhaustEmitter.setRandomLife(true);
	exhaustEmitter.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
	exhaustEmitter.setPosition(lander.getPosition());

	height = 0;
}

void ofApp::loadVbo()
{
	if (exhaustEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < exhaustEmitter.sys->particles.size(); i++)
	{
		points.push_back(exhaustEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(20));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	// live update of emmitter parameters (with sliders)
	//
	exhaustEmitter.setParticleRadius(radius);
	//exhaustEmitter.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
	//exhaustEmitter.setMass(mass);
	//exhaustEmitter.setDamping(damping);
	//exhaustEmitter.setGroupSize(numParticles);

	// live update of forces  (with sliders)
	//
	gravityForce->set(ofVec3f(0, -gravity, 0));
	turbulenceForce->set(ofVec3f(turbMin->x, turbMin->y, turbMin->z), ofVec3f(turbMax->x, turbMax->y, turbMax->z));
	impulseForce->set(radialForceVal);
	impulseForce->setHeight(radialHight);
	cyclicForce->set(cyclicForceVal);

	//References to lander particle
	Particle& p = landerEmitter.sys->particles[0];
	//Checks keys
	//Space to move foward
	if (keymap[' ']) {
		p.addForces(10 * p.heading());

		//Offset exhaust particles position to appear inside spaceship exhaust 
		glm::vec3 landerPosition = lander.getPosition();
		float yOffset = -1;
		glm::vec3 exhaustEmitterPosition = landerPosition + glm::vec3(0, yOffset, 0);
		exhaustEmitter.setPosition(exhaustEmitterPosition);
	}
	//Control to move backwards
	if (keymap[OF_KEY_CONTROL]) {
		p.addForces(-10 * p.heading());
	}
	//Arrow up to pitch nose up
	if (keymap[OF_KEY_UP]) {
		p.addAngularForces(100);
	}
	//Arrow down to pitch nose down
	if (keymap[OF_KEY_DOWN]) {
		p.addAngularForces(-100);
	}
	//Arrow right to move right
	if (keymap[OF_KEY_RIGHT]) {
		p.addForces(10 * glm::vec3(1, 0, 0));
	}
	//Arrow left to move left 
	if (keymap[OF_KEY_LEFT]) {
		p.addForces(10 * glm::vec3(-1, 0, 0));
	}
	//Connects lander to landerEmitter particle
	if (bLanderLoaded && !bInDrag) {
		//ofVec3f min =  0.01 + ofVec3f(landerBounds.min().x(), landerBounds.min().y(), landerBounds.min().z());
		//ofVec3f max =  0.01 + ofVec3f(landerBounds.max().x(), landerBounds.max().y(), landerBounds.max().z());
		
		ofVec3f min = (ofVec3f(landerBounds.min().x(), landerBounds.min().y(), landerBounds.min().z()) + lander.getPosition());
		ofVec3f max = (ofVec3f(landerBounds.max().x(), landerBounds.max().y(), landerBounds.max().z()) + lander.getPosition());
		boundingBox = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		checkCollision();

		impulseForce->setMagnitude(p.velocity.length());
		neutralForce->set(-p.velocity);
		lander.setPosition(p.position.x, p.position.y, p.position.z);
		lander.setRotation(0, p.rotation, 1, 0, 0);
		landerEmitter.update();
		exhaustEmitter.update();

		if (collision) {
			landerEmitter.sys->addForce(neutralForce);
		//	landerEmitter.sys->addForce(impulseForce);
		}
		else {
			cout << landerEmitter.sys->forces[0] << endl;
		}
		if (bDefaultCam) {
			cam.setDistance(20);
		}
		if (bThirdPersonCam) {
			glm::vec3 pos = glm::vec3(p.position.x, p.position.y + 2.5, p.position.z + 10);
			cam.setPosition(pos);
			cam.setTarget(p.position + p.heading() * 20);
		}
		if (bTopDownCam) {
			glm::vec3 pos = glm::vec3(p.position.x, p.position.y + 25, p.position.z + 4);
			cam.setPosition(pos);
			cam.setTarget(p.position);
		}
		AGL.setOrigin(Vector3(p.position.x, p.position.y, p.position.z));
		pointSelected = octree.intersect(AGL, octree.root, selectedNode);
		if (pointSelected) {
			ofVec3f pointRet = octree.mesh.getVertex(selectedNode.points[0]);
			height = AGL.origin.y() - pointRet.y;
			cout << height << endl;
		}
	}
}
//--------------------------------------------------------------
void ofApp::draw() {

	loadVbo();

	landerEmitter.sys->draw();

	ofBackground(ofColor::black);
	glDepthMask(false);
	if (!bHide) gui.draw();
	glDepthMask(true);

	cam.begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					//ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {
				ofNoFill();
				ofSetColor(ofColor::white);
				ofPushMatrix();
				Octree::drawBox(boundingBox);
				ofPopMatrix();
				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
		ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(colorMap[numLevels]);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}
	ofPopMatrix();

	shader.begin();

	// draw exhaust particle emitter...
	//
	exhaustEmitter.draw();
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)exhaustEmitter.sys->particles.size());
	particleTex.unbind();
	
	shader.end();

	cam.end();


}

// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}

void ofApp::keyPressed(int key) {

	switch (key) {
	//Turns on top down cam
	case '1':
		bTopDownCam = true;
		bThirdPersonCam = false;
		bDefaultCam = false;
		break;
	//Turns on top down cam
	case '2':
		bTopDownCam = false;
		bThirdPersonCam = false;
		bDefaultCam = true;
		break;
	case '3':
		bTopDownCam = false;
		bThirdPersonCam = true;
		bDefaultCam = false;
		break;
	case '4':
		if (bLanderLoaded && collision) {
			glm::vec3 pos = lander.getPosition();
			lander.setPosition(pos.x, pos.y + 4, pos.z);
			ofVec3f min = lander.getSceneMin() + lander.getPosition();
			ofVec3f max = lander.getSceneMax() + lander.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

			colBoxList.clear();
			collision = octree.intersect(bounds, octree.root, colBoxList);
		}
		break;
	case 'z':
		toggleTimer();
		break;
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	case ' ':
		exhaustEmitter.sys->reset();
		exhaustEmitter.start();
		break;
	}
	keymap[key] = true;
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::toggleTimer() {
	bTimer = !bTimer;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;
	}
	keymap[key] = false;
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

    float time = ofGetElapsedTimeMillis();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	time = ofGetElapsedTimeMillis() - time;
	if (bTimer) {
		cout << "Time: " << time << endl;
	}
	if (pointSelected) {
		cout << true << endl;
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;
		checkCollision();
		/*
		if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}
		*/
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
	landerEmitter.sys->particles[0].position = lander.getPosition();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
	//Resets Forces
	landerEmitter.sys->reset();
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setRotation(1, -90, 1, 0, 0);
		lander.setScaleNormalization(false);
		lander.setScale(.8, .8, .8);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::mat4 a = lander.getModelMatrix();
			ofVec3f min = (lander.getSceneMin()) * a;
			ofVec3f max = (lander.getSceneMax()) * a;
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
			Particle &p = landerEmitter.sys->particles[0];
			p.position.set(lander.getPosition());
			AGL = Ray(Vector3(p.position.x, p.position.y, p.position.z), Vector3(0, -1, 0));
		}
	}
}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

void ofApp::checkCollision() {
	colBoxList.clear();
	collision = octree.intersect(landerBounds, octree.root, colBoxList);
	if (collision) cout << "collide" << endl;
}