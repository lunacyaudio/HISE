#ifndef HI_SCRIPTING_API_CUBE_API
#define HI_SCRIPTING_API_CUBE_API

namespace cube {

using namespace hise;

struct Orb {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float wiggleAmount = 1.0f;

    // How much the orb should ripple from each of the 8 corners.
    std::vector<float> rippleAmounts = {1.0f, 1.0f, 1.0f, 1.0f,
                                        1.0f, 1.0f, 1.0f, 1.0f};

    // Normalized time within the orbit, between 0 and 1.
    float orbitTime = 0.0f;
};

struct Orbit {
    struct Path {
        struct Keyframe {
            float time = 0;
            float pos = 0;
            float curve = 0.5f;
        };
        std::vector<Keyframe> keyframes;
    };

    struct Lfo {
        enum WaveType { Sin, Triangle, Saw, Square };
        WaveType waveType = Sin;
        float frequency = 1;
        float phaseOffset = 0;
        float min = -1;
        float max = 1;
    };

    struct Axis {
        enum Type { Path, Lfo };
        Type type = Path;
        struct Path path;
        struct Lfo lfo;
    };

    bool visible = false;
    bool draggingEnabled = false;
    Axis x;
    Axis y;
    Axis z;
    hise::Vector3D<float> rotation;
    hise::Vector3D<bool> mirror;
    float intensity = 1;
};

struct Cube {
    Orb orb = {};
    Orbit orbit = {};
    float ether = 0;
    std::map<String, var> cornerData = {};

    // JS callback invoked when a corner overlay button has been clicked.
    //  - id: corner ID
    //  - button: string identifying the button that was clicked
    std::function<void(String, String)> cornerButtonCallback =
        [](String id, String button) {};

    // JS callback invoked when the orb has been dragged.
    std::function<void(float, float, float)> orbDragCallback =
        [](float x, float y, float z) {};
};

// The Cube Javascript API.
class CubeApi : public ApiClass, public ScriptingObject
{
public:
    CubeApi(ProcessorWithScriptingContent *p);

    ~CubeApi();

    ////  Static API used by the cube C++ library.  ////

    // Returns a copy of the Cube data associated with this MainController.
    // Note: there may be multiple instances of running multiple Cubes at once
    // in a DAW.
    static Cube getCubeData(const MainController* mc);

    // Removes the cube data for the given MainController, as part of cleanup.
    static void removeCubeData(const MainController* mc);

    static void setOrbPosition(const MainController* mc,
                               float x, float y, float z);


    ////  HiseScript API.  ////
	struct Wrapper {
		API_VOID_METHOD_WRAPPER_3(CubeApi, setOrbPosition);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setOrbTime);
        API_METHOD_WRAPPER_0(CubeApi, getOrbPosition);
        API_VOID_METHOD_WRAPPER_0(CubeApi, showOrbit);
        API_VOID_METHOD_WRAPPER_0(CubeApi, hideOrbit);
        API_VOID_METHOD_WRAPPER_0(CubeApi, enableDragging);
        API_VOID_METHOD_WRAPPER_0(CubeApi, disableDragging);
        API_VOID_METHOD_WRAPPER_4(CubeApi, setLfo);
        API_VOID_METHOD_WRAPPER_3(CubeApi, setLfoRange);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setEmptyPath);
        API_VOID_METHOD_WRAPPER_4(CubeApi, addPathKeyframe);
        API_VOID_METHOD_WRAPPER_3(CubeApi, setOrbitRotation);
        API_VOID_METHOD_WRAPPER_3(CubeApi, setOrbitMirror);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setOrbitIntensity);
        API_VOID_METHOD_WRAPPER_2(CubeApi, setRippleAmount);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setWiggleAmount);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setEther);
        API_VOID_METHOD_WRAPPER_2(CubeApi, setCornerData);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setCornerButtonCallback);
        API_VOID_METHOD_WRAPPER_1(CubeApi, setOrbDragCallback);
	};
    void setOrbPosition(float x, float y, float z);
    void setOrbTime(float t);
    Array<var> getOrbPosition();
    void showOrbit();
    void hideOrbit();
    void setLfo(int axis, String waveType, float frequency, float phaseOffset);
    void setLfoRange(int axis, float min, float max);
    void setEmptyPath(int axis);
    void addPathKeyframe(int axis, float time, float pos, float curve);
    void setOrbitRotation(float x, float y, float z);
    void setOrbitMirror(bool x, bool y, bool z);
    void setOrbitIntensity(float intensity);
    void enableDragging();
    void disableDragging();
    void setRippleAmount(int id, float rippleAmount);
    void setWiggleAmount(float wiggleAmount);
    void setEther(float ether);
    void setCornerData(String id, var data);
    void setCornerButtonCallback(var callback);
    void setOrbDragCallback(var callback);

	Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("CubeApi"); }
    static Identifier getClassName() { RETURN_STATIC_IDENTIFIER("CubeApi"); }

private:
    static std::map<const MainController*, Cube> cubes;
    static std::mutex mutex;

    // Non thread-safe version of getCubeData(const MainController*).
    static Cube& getCubeDataUnsafe(const MainController* mc);

    Orbit::Axis* getAxis(int axis);

    // Returns the cube data for the current app instance.
    Cube& getCubeData();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CubeApi);
};

}

#endif
