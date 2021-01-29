namespace cube {

std::map<const MainController*, Cube> CubeApi::cubes = {};
std::mutex CubeApi::mutex = {};

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
    ADD_API_METHOD_2(setRippleAmount);
    ADD_API_METHOD_1(setWiggleAmount);
    ADD_API_METHOD_1(setEther);
    ADD_API_METHOD_2(setCornerData);
    ADD_API_METHOD_1(setCornerButtonCallback);
    ADD_API_METHOD_1(setOrbDragCallback);
}

CubeApi::~CubeApi() {}

Cube CubeApi::getCubeData(const MainController* mc) {
    //const std::lock_guard<std::mutex> lock(mutex);
    return getCubeDataUnsafe(mc);
}

void CubeApi::removeCubeData(const MainController* mc) {
    //const std::lock_guard<std::mutex> lock(mutex);
    cubes.erase(mc);
}

void CubeApi::setOrbPosition(const MainController* mc,
                             float x, float y, float z) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeDataUnsafe(mc);
    cube.orb.x = x;
    cube.orb.y = y;
    cube.orb.z = z;
}

void CubeApi::setOrbPosition(float x, float y, float z) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orb.x = x;
    cube.orb.y = y;
    cube.orb.z = z;
}

Array<var> CubeApi::getOrbPosition() {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    Array<var> orbPosition;
    orbPosition.add(cube.orb.x);
    orbPosition.add(cube.orb.y);
    orbPosition.add(cube.orb.z);
    return orbPosition;
}

void CubeApi::setOrbTime(float t) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orb.orbitTime = t;
}

void CubeApi::showOrbit() {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.visible = true;
}

void CubeApi::hideOrbit() {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.visible = false;
}

void CubeApi::enableDragging() {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.draggingEnabled = true;
}

void CubeApi::disableDragging() {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.draggingEnabled = false;
}

void CubeApi::setLfo(int axis, String waveType, float frequency,
                     float phaseOffset) {
    //const std::lock_guard<std::mutex> lock(mutex);
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
    //const std::lock_guard<std::mutex> lock(mutex);
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    orbitAxis->lfo.min = min;
    orbitAxis->lfo.max = max;
}

void CubeApi::setEmptyPath(int axis) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Orbit::Axis* orbitAxis = getAxis(axis);
    if (orbitAxis == nullptr) {
        return;
    }
    orbitAxis->type = Orbit::Axis::Path;
    orbitAxis->path.keyframes.clear();
}

void CubeApi::addPathKeyframe(int axis, float time, float pos, float curve) {
    //const std::lock_guard<std::mutex> lock(mutex);
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
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.rotation.x = x;
    cube.orbit.rotation.y = y;
    cube.orbit.rotation.z = z;
}

void CubeApi::setOrbitMirror(bool x, bool y, bool z) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.mirror.x = x;
    cube.orbit.mirror.y = y;
    cube.orbit.mirror.z = z;
}

void CubeApi::setOrbitIntensity(float intensity) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orbit.intensity = intensity;
}

void CubeApi::setRippleAmount(int id, float rippleAmount) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    if (id < cube.orb.rippleAmounts.size()) {
        cube.orb.rippleAmounts[id] = rippleAmount;
    }
}

void CubeApi::setWiggleAmount(float wiggleAmount) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.orb.wiggleAmount = wiggleAmount;
}

void CubeApi::setEther(float ether) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.ether = ether;
}

void CubeApi::setCornerData(String id, var data) {
    //const std::lock_guard<std::mutex> lock(mutex);
    Cube& cube = getCubeData();
    cube.cornerData[id] = data;
}

void CubeApi::setCornerButtonCallback(var callback) {
    //const std::lock_guard<std::mutex> lock(mutex);
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

void CubeApi::setOrbDragCallback(var callback) {
    //const std::lock_guard<std::mutex> lock(mutex);
    if (!HiseJavascriptEngine::isJavascriptFunction(callback)) {
        return;
    }

    auto* engine = dynamic_cast<JavascriptMidiProcessor*>(
        getScriptProcessor())->getScriptEngine();
    if (engine == nullptr) {
        return;
    }

    Cube& cube = getCubeData();
    cube.orbDragCallback =
        [this, engine, callback](float x, float y, float z) {
            var thisObject(this);
            var data[3] = { var(x), var(y), var(z) };
            var::NativeFunctionArgs args(thisObject, data, 3);
            Result result = Result::ok();
            engine->maximumExecutionTime = RelativeTime(0.5);
            engine->callExternalFunction(callback, args, &result);
        };
}

Cube& CubeApi::getCubeDataUnsafe(const MainController* mc) {
    if (cubes.find(mc) == cubes.end()) {
        cubes[mc] = {};
    }
    return cubes[mc];
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
    return getCubeDataUnsafe(getScriptProcessor()->getMainController_());
}

}  // namespace cube
