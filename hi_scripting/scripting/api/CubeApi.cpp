namespace cube {

std::map<const MainController*, Cube> CubeApi::cubes = {};

CubeApi::CubeApi(ProcessorWithScriptingContent *p) :
        ScriptingObject(p), ApiClass(0) {
    ADD_API_METHOD_3(setOrbPosition);
    ADD_API_METHOD_0(getOrbPosition);
    ADD_API_METHOD_0(showOrbit);
    ADD_API_METHOD_0(hideOrbit);
    ADD_API_METHOD_0(enableDragging);
    ADD_API_METHOD_0(disableDragging);
    ADD_API_METHOD_4(setLfo);
    ADD_API_METHOD_3(setLfoRange);
    ADD_API_METHOD_1(setEmptyPath);
    ADD_API_METHOD_5(addPathKeyframe);
    ADD_API_METHOD_3(setOrbitRotation);
    ADD_API_METHOD_3(setOrbitMirror);
    ADD_API_METHOD_1(setOrbitIntensity);
    ADD_API_METHOD_1(setRippleAmount);
    ADD_API_METHOD_1(setWiggleAmount);
    ADD_API_METHOD_1(setEther);
    ADD_API_METHOD_2(setCornerData);
    ADD_API_METHOD_1(setCornerButtonCallback);
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

void CubeApi::addPathKeyframe(int axis, float time, float pos,
                              bool easeIn, bool easeOut) {
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    Orbit::Path::Keyframe keyframe;
    keyframe.time = time;
    keyframe.pos = pos;
    keyframe.easeIn = easeIn;
    keyframe.easeOut = easeOut;
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

void CubeApi::setRippleAmount(float rippleAmount) {
    Cube& cube = getCubeData();
    cube.orb.rippleAmount = rippleAmount;
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
    cube.cornerButtonCallback =
        [this, engine, callback](String id, String button) {
            var thisObject(this);
            var data[2] = { var(id), var(button) };
            var::NativeFunctionArgs args(thisObject, data, 2);
            Result result = Result::ok();
            engine->maximumExecutionTime = RelativeTime(0.5);
            engine->callExternalFunction(callback, args, &result);
        };
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

}  // namespace cube
