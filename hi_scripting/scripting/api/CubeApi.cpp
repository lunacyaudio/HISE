namespace cube {

// Removed and replaced with a better lifetime management
//std::map<const MainController*, Cube> CubeApi::cubes = {};

CubeApi::CubeApi(ProcessorWithScriptingContent *p) :
        ScriptingObject(p), ApiClass(0) {
    ADD_API_METHOD_3(setOrbPosition);
    ADD_API_METHOD_1(setOrbTime);
    ADD_API_METHOD_0(getOrbPosition);
    ADD_API_METHOD_0(showOrbit);
    ADD_API_METHOD_0(hideOrbit);
    ADD_API_METHOD_0(enableDragging);
    ADD_API_METHOD_0(disableDragging);
    ADD_API_METHOD_4(setLfo);
    ADD_API_METHOD_3(setLfoRange);
    ADD_API_METHOD_1(setEmptyPath);
    ADD_API_METHOD_4(addPathKeyframe);
    ADD_API_METHOD_3(setOrbitRotation);
    ADD_API_METHOD_3(setOrbitMirror);
    ADD_API_METHOD_1(setOrbitIntensity);
    ADD_API_METHOD_3(setGradientStart);
    ADD_API_METHOD_3(setGradientEnd);
    ADD_API_METHOD_2(setRippleAmount);
    ADD_API_METHOD_1(setWiggleAmount);
    ADD_API_METHOD_1(setEther);
    ADD_API_METHOD_2(setCornerData);
    ADD_API_METHOD_1(setCornerButtonCallback);
    ADD_API_METHOD_1(setOrbDragCallback);
}

CubeApi::~CubeApi() {}

void CubeApi::setOrbPosition(float x, float y, float z) {
    Cube& cube = getCubeData();
    cube.orb.x = x;
    cube.orb.y = y;
    cube.orb.z = z;
}

Array<var> CubeApi::getOrbPosition() {
    Cube& cube = getCubeData();
    Array<var> orbPosition;
    orbPosition.add(cube.orb.x);
    orbPosition.add(cube.orb.y);
    orbPosition.add(cube.orb.z);
    return orbPosition;
}

void CubeApi::setOrbTime(float t) {
    Cube& cube = getCubeData();
    cube.orb.orbitTime = t;
}

void CubeApi::showOrbit() {
    Cube& cube = getCubeData();
    cube.orbit.visible = true;
}

void CubeApi::hideOrbit() {
    Cube& cube = getCubeData();
    cube.orbit.visible = false;
}

void CubeApi::enableDragging() {
    Cube& cube = getCubeData();
    cube.orbit.draggingEnabled = true;
}

void CubeApi::disableDragging() {
    Cube& cube = getCubeData();
    cube.orbit.draggingEnabled = false;
}

void CubeApi::setLfo(int axis, String waveType, float frequency,
                     float phaseOffset) {
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    Orbit::Lfo::WaveType waveTypeEnum;
    if (waveType == "sin") {
        waveTypeEnum = Orbit::Lfo::Sin;
    } else if (waveType == "triangle") {
        waveTypeEnum = Orbit::Lfo::Triangle;
    } else if (waveType == "saw") {
        waveTypeEnum = Orbit::Lfo::Saw;
    } else if (waveType == "square") {
        waveTypeEnum = Orbit::Lfo::Square;
    } else {
        std::cout << "Error: unknown wave type: " << waveType << std::endl;
        return;
    }

    orbitAxis->type = Orbit::Axis::Lfo;
    orbitAxis->lfo.waveType = waveTypeEnum;
    orbitAxis->lfo.frequency = frequency;
    orbitAxis->lfo.phaseOffset = phaseOffset;
}

void CubeApi::setLfoRange(int axis, float min, float max) {
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    orbitAxis->lfo.min = min;
    orbitAxis->lfo.max = max;
}

void CubeApi::setEmptyPath(int axis) {
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    orbitAxis->type = Orbit::Axis::Path;
    orbitAxis->path.keyframes.clear();
}

void CubeApi::addPathKeyframe(int axis, float time, float pos, float curve) {
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    Orbit::Path::Keyframe keyframe;
    keyframe.time = time;
    keyframe.pos = pos;
    keyframe.curve = curve;
    orbitAxis->path.keyframes.push_back(keyframe);
}

void CubeApi::setOrbitRotation(float x, float y, float z) {
    Cube& cube = getCubeData();
    cube.orbit.rotation.x = x;
    cube.orbit.rotation.y = y;
    cube.orbit.rotation.z = z;
}

void CubeApi::setOrbitMirror(bool x, bool y, bool z) {
    Cube& cube = getCubeData();
    cube.orbit.mirror.x = x;
    cube.orbit.mirror.y = y;
    cube.orbit.mirror.z = z;
}

void CubeApi::setOrbitIntensity(float intensity) {
    Cube& cube = getCubeData();
    cube.orbit.intensity = intensity;
}

void CubeApi::setGradientStart(int r, int g, int b) {
    Cube& cube = getCubeData();
    cube.orbit.gradientStart.x = static_cast<float>(r) / 255.f;
    cube.orbit.gradientStart.y = static_cast<float>(g) / 255.f;
    cube.orbit.gradientStart.z = static_cast<float>(b) / 255.f;
}

void CubeApi::setGradientEnd(int r, int g, int b) {
    Cube& cube = getCubeData();
    cube.orbit.gradientEnd.x = static_cast<float>(r) / 255.f;
    cube.orbit.gradientEnd.y = static_cast<float>(g) / 255.f;
    cube.orbit.gradientEnd.z = static_cast<float>(b) / 255.f;
}

void CubeApi::setRippleAmount(int id, float rippleAmount) {
    Cube& cube = getCubeData();
    if (id < cube.orb.rippleAmounts.size()) {
        cube.orb.rippleAmounts[id] = rippleAmount;
    }
}

void CubeApi::setWiggleAmount(float wiggleAmount) {
    Cube& cube = getCubeData();
    cube.orb.wiggleAmount = wiggleAmount;
}

void CubeApi::setEther(float ether) {
    Cube& cube = getCubeData();
    cube.ether = ether;
}

void CubeApi::setCornerData(String id, var data) {
    Cube& cube = getCubeData();
    cube.cornerData[id] = data;
}

void CubeApi::setCornerButtonCallback(var callback) {
    if (!HiseJavascriptEngine::isJavascriptFunction(callback)) {
        return;
    }

    auto* engine = dynamic_cast<JavascriptMidiProcessor*>(
        getScriptProcessor())->getScriptEngine();
    if (engine == nullptr) {
        return;
    }

    Cube& cube = getCubeData();
	cube.cornerButtonCallback = new WeakCallbackHolder(getScriptProcessor(), callback, 2);
	cube.cornerButtonCallback->setThisObject(this);
	cube.cornerButtonCallback->incRefCount();
	cube.cornerButtonCallback->setHighPriority();
}

void CubeApi::setOrbDragCallback(var callback) {
    if (!HiseJavascriptEngine::isJavascriptFunction(callback)) {
        return;
    }

    auto* engine = dynamic_cast<JavascriptMidiProcessor*>(
        getScriptProcessor())->getScriptEngine();
    if (engine == nullptr) {
        return;
    }

    Cube& cube = getCubeData();
	cube.orbDragCallback = new WeakCallbackHolder(getScriptProcessor(), callback, 3);

	// make it use this API class as this object
	cube.orbDragCallback->setThisObject(this);

	// increment the ref-counting so that anonymous functions are not deleted
	cube.orbDragCallback->incRefCount();

	// make it execute on the high-priority lane (the low-priority lane might be clogged by paint routines)
	cube.orbDragCallback->setHighPriority();
}

Orbit::Axis* CubeApi::getAxis(int axis) {
    Cube& cube = getCubeData();
    if (axis == 0) {
        return &(cube.orbit.x);
    } else if (axis == 1) {
        return &(cube.orbit.y);
    } else if (axis == 2) {
        return &(cube.orbit.z);
    } else {
        std::cout << "Invalid orbit axis: " << axis << std::endl;
        return nullptr;
    }
}

Cube& CubeApi::getCubeData() {
    return getCubeData(getScriptProcessor()->getMainController_());
}

cube::Cube& CubeApi::getCubeData(const MainController* mc)
{
	// this is bad style, so you might want to remove the constness from the 
	// main controller argument along the call stack
	auto unConst = const_cast<MainController*>(mc);

	// Instead of the std::map we'll abuse the global object of each main controller
	// (this is used when you define a `global` variable in the script engine, but
	// it should do the trick and avoid a static storage type that comes with a few
	// lifetime issues
	static const Identifier cubeId("_mainCubeObject");

	if (auto gObj = unConst->getGlobalVariableObject())
	{
		auto obj = gObj->getProperty(cubeId);
		
		if (auto cObj = dynamic_cast<cube::Cube*>(obj.getObject()))
		{
			// It already exists
			return *cObj;
		}
		else
		{
			// It doesn't exist yet so we have to lazy-initialise it now
			// (the first time it's being requested)
			Cube::Ptr newObject = new Cube();

			// a var takes ownership of an object if it's derived by ReferenceCountedObject
			gObj->setProperty(cubeId, var(newObject));

			return *newObject;
		}
	}
	else
	{
		// the dynamic object is being created in the MainControllers' constructor
		// so it should be available as long as the MainController is alive...
		jassertfalse;
	}
}

}  // namespace cube
