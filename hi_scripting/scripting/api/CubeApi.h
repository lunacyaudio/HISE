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
            bool operator==(const Keyframe& rhs) const {
                return time == rhs.time &&
                    pos == rhs.pos &&
                    curve == rhs.curve;
            }
        };
        std::vector<Keyframe> keyframes;
        bool operator==(const Path& rhs) const {
            return keyframes == rhs.keyframes;
        }
    };

    struct Lfo {
        enum WaveType { Sin, Triangle, Saw, Square };
        WaveType waveType = Sin;
        float frequency = 1;
        float phaseOffset = 0;
        float min = -1;
        float max = 1;
        bool operator==(const Lfo& rhs) const {
            return waveType == rhs.waveType &&
                frequency == rhs.frequency &&
                phaseOffset == rhs.phaseOffset &&
                min == rhs.min &&
                max == rhs.max;
        }
    };

    struct Axis {
        enum Type { Path, Lfo };
        Type type = Path;
        struct Path path;
        struct Lfo lfo;
        bool operator==(const Axis& rhs) const {
            return type == rhs.type &&
                (type == Path) ?
                    path == rhs.path :
                    lfo == rhs.lfo;
        }
    };

    bool visible = false;
    bool draggingEnabled = false;
    Axis x;
    Axis y;
    Axis z;
    hise::Vector3D<float> rotation;
    hise::Vector3D<bool> mirror;
    float intensity = 1;
    hise::Vector3D<float> gradientStart =
        hise::Vector3D<float>(164.f, 108.f, 217.f) / 255.f;
    hise::Vector3D<float> gradientEnd =
        hise::Vector3D<float>(224.f, 130.f, 114.f) / 255.f;
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
    // Returns the Cube data associated with this MainController instance.
    // Note: there may be multiple instances of running multiple Cubes at once
    // in a DAW.
    static Cube& getCubeData(const MainController* mc) {
        if (cubes.find(mc) == cubes.end()) {
            cubes[mc] = {};
        }
        return cubes[mc];
    }

    CubeApi(ProcessorWithScriptingContent *p);

    ~CubeApi();

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
        API_VOID_METHOD_WRAPPER_3(CubeApi, setGradientStart);
        API_VOID_METHOD_WRAPPER_3(CubeApi, setGradientEnd);
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
    void setGradientStart(int r, int g, int b);
    void setGradientEnd(int r, int g, int b);
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

    Orbit::Axis* getAxis(int axis);

    Cube& getCubeData();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CubeApi);
};

}

#endif
