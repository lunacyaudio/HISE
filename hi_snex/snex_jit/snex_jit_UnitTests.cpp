/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

/** TODO: Add tests for:

- complex function calls
- > 20 variables
- complex nested expressions
- scope related variable resolving (eg float x = 12.0f; { float x = x; };
*/

#pragma once

namespace snex {
namespace jit {
using namespace juce;
using namespace asmjit;

#pragma warning( push )
#pragma warning( disable : 4244)

class OptimizationTestCase
{
public:

	void setOptimizations(const Array<Identifier>& passList)
	{
		for (auto& p : passList)
			optimizingScope.addOptimization(p);
	}

	void setExpressionBody(const juce::String& body_)
	{
		jassert(body_.contains("%BODY%"));

		body = body_;
	}

	bool sameAssembly(const juce::String& expressionToBeOptimised, const juce::String& reference)
	{
		auto o = body.replace("%BODY%", expressionToBeOptimised);
		auto r = body.replace("%BODY%", reference);

		auto oAssembly = getAssemblyOutput(o, optimizingScope);
		auto rAssembly = getAssemblyOutput(r, referenceScope);

		bool equal = oAssembly.hashCode64() == rAssembly.hashCode64();

		if (!equal)
		{
			DBG("Wrong assembly: ");
			
			DBG(oAssembly);

			DBG("Reference assembly:");
			DBG(rAssembly);

		}

		return equal;
	}

private:

	juce::String getAssemblyOutput(const juce::String& t, GlobalScope& s)
	{
		Compiler c(s);
		auto obj = c.compileJitObject(t);

		if (!c.getCompileResult().wasOk())
		{
			auto r = c.getCompileResult().getErrorMessage();
			jassertfalse;
			return t;
		}
			

		return c.getAssemblyCode();
	}

	juce::String body;
	GlobalScope referenceScope;
	GlobalScope optimizingScope;
};
    
template <typename T, typename ReturnType=T> class HiseJITTestCase: public jit::DebugHandler
{
public:

	HiseJITTestCase(const juce::String& stringToTest, const Array<Identifier>& optimizationList) :
		code(stringToTest)
	{
		for (auto o : optimizationList)
			memory.addOptimization(o);

		compiler = new Compiler(memory);
	}

	void logMessage(const juce::String& s) override
	{
		DBG(s);
	}

	~HiseJITTestCase()
	{
		
	}

	void dump()
	{
		breakBeforeCall = true;
	}

	ReturnType getResult(T input, ReturnType expected)
	{
		if (!initialised)
			setup();

		static const Identifier t("test");

		if (auto f = func[t])
		{
			before = func.dumpTable();

			if (breakBeforeCall)
			{
				DBG("code: ");
				DBG(compiler->getLastCompiledCode());
				DBG("assembly: ");
				DBG(compiler->getAssemblyCode());
				DBG("Data dump before call:");
				DBG(before);
				

				jassertfalse; // there you go...
			}

			ReturnType v = f.template call<ReturnType>(input);

			if (!(v == expected))
			{
				dump();

				DBG("Expected: " + juce::String(expected));
				DBG("Actual: " + juce::String(v));
			}

			return v;
		}
        else
        {
            DBG(compiler->getCompileResult().getErrorMessage());
        }

		return ReturnType();
	};


	juce::String before;
	bool breakBeforeCall = false;

	bool wasOK()
	{
		return compiler->getCompileResult().wasOk();
	}

	void setup()
	{
		

		func = compiler->compileJitObject(code);

        
        
#if JUCE_DEBUG
		if (!wasOK())
		{
			DBG(code);
			DBG(compiler->getCompileResult().getErrorMessage());
		}
#endif

		if (auto f = func["setup"])
			f.callVoid();

		initialised = true;
	}

	juce::String code;

	bool initialised = false;

	jit::GlobalScope memory;
	ScopedPointer<Compiler> compiler;
	JitObject func;



};

class JITTestModule
{
public:

	void setGlobals(const juce::String& t)
	{
		globals = t;
	}

	void setInitBody(const juce::String& body)
	{
		initBody = body;
	};

	void setPrepareToPlayBody(const juce::String& body)
	{
		prepareToPlayBody = body;
	}

	void setProcessBody(const juce::String& body)
	{
		processBody = body;
	}

	void setCode(const juce::String& code_)
	{
		code = code_;
	}

	void merge()
	{
		code = juce::String();
		code << globals << "\n";
		code << "void init() {\n\t";
		code << initBody;
		code << "\n};\n\nvoid prepareToPlay(double sampleRate, int blockSize) {\n\t";
		code << prepareToPlayBody;
		code << "\n};\n\nfloat process(float input) {\n\t";
		code << processBody;
		code << "\n};";

	}

	void createModule()
	{
	}


	juce::String globals;
	juce::String initBody;
	juce::String prepareToPlayBody;
	juce::String processBody = "return 1.0f;";
	juce::String code;

	double executionTime;
};



#define CREATE_TEST(x) test = new HiseJITTestCase<float>(x, optimizations);
#define CREATE_TYPED_TEST(x) test = new HiseJITTestCase<T>(x, optimizations);
#define CREATE_TEST_SETUP(x) test = new HiseJITTestCase<float>(x, optimizations); test->setup();

#define EXPECT(testName, input, result) expect(test->wasOK(), juce::String(testName) + juce::String(" parsing")); expectAlmostEquals<float>(test->getResult(input, result), result, testName);

#define EXPECT_TYPED(testName, input, result) expect(test->wasOK(), juce::String(testName) + juce::String(" parsing")); expectAlmostEquals<T>(test->getResult(input, result), result, testName);

#define GET_TYPE(T) Types::Helpers::getTypeNameFromTypeId<T>()

#define CREATE_BOOL_TEST(x) test = new HiseJITTestCase<int>(juce::String("int test(int input){ ") + juce::String(x), optimizations);


#define EXPECT_BOOL(name, result) expect(test->wasOK(), juce::String(name) + juce::String(" parsing")); expect(test->getResult(0, result) == result, name);
#define VAR_BUFFER_TEST_SIZE 8192

#define ADD_CODE_LINE(x) code << x << "\n"

#define T_A + getLiteral<T>(a) +
#define T_B + getLiteral<T>(b) +
#define T_1 + getLiteral<T>(1.0) +
#define T_0 + getLiteral<T>(0.0) +

#define START_BENCHMARK const double start = Time::getMillisecondCounterHiRes();
#define STOP_BENCHMARK_AND_LOG const double end = Time::getMillisecondCounterHiRes(); logPerformanceMessage(m->executionTime, end - start);




class HiseJITUnitTest : public UnitTest
{
public:

	HiseJITUnitTest() : UnitTest("HiseJIT UnitTest") {}

	void runTest() override
	{
        testGlobals();
        return;
        
        testMacOSRelocation();
        
        testOptimizations();

		runTestsWithOptimisation({});
		runTestsWithOptimisation({ OptimizationIds::ConstantFolding });
		runTestsWithOptimisation({ OptimizationIds::ConstantFolding, OptimizationIds::BinaryOpOptimisation });
	}

	void runTestsWithOptimisation(const Array<Identifier>& ids)
	{
		logMessage("OPTIMIZATIONS");

		for (auto o : ids)
			logMessage("--- " + o.toString());

		optimizations = ids;

		testParser();
		testSimpleIntOperations();

		testOperations<float>();
		testOperations<double>();
		testOperations<int>();

		testCompareOperators<double>();
		testCompareOperators<int>();
		testCompareOperators<float>();

		testPointerVariables<int>();
		testPointerVariables<double>();
		testPointerVariables<float>();

		testTernaryOperator();
		testIfStatement();

		testMathConstants<float>();
		testMathConstants<double>();
        
		testComplexExpressions();
		testGlobals();
		testFunctionCalls();
		testDoubleFunctionCalls();
		testBigFunctionBuffer();
		testLogicalOperations();
		
		testScopes();
		
		//testEventSetters();
		//testEvents();

		testBlocks();
		testSpan<int>();
		testSpan<float>();
		testSpan<double>();
		testStructs();
		testUsingAliases();
	}

private:

	template <typename T> void testSpan()
	{
		auto type = Types::Helpers::getTypeFromTypeId<T>();

		beginTest("Testing span operations for " + Types::Helpers::getTypeName(type));

		Random r;
		int size = r.nextInt({ 1, 100 });
		int index = size > 1 ? r.nextInt({ 0, size - 1 }) : 0;
		double a = (double)r.nextInt(25) *(r.nextBool() ? 1.0 : -1.0);
		double b = (double)r.nextInt(62) *(r.nextBool() ? 1.0 : -1.0);

		if (b == 0.0) b = 55.0;

		float d[12];

		ScopedPointer<HiseJITTestCase<T>> test;

		juce::String code;

		

#define NEW_CODE_TEXT() code = {};
#define DECLARE_SPAN(name) ADD_CODE_LINE("span<$T, $size> " + juce::String(name) + ";")
#define FINALIZE_CODE() code = code.replace("$T", getTypeName<T>()); code = code.replace("$size", juce::String(size)); code = code.replace("$index", juce::String(index));

		auto im = [](T v)
		{
			auto type = Types::Helpers::getTypeFromTypeId<T>();
			VariableStorage v_(type, var(v));
			return Types::Helpers::getCppValueString(v_);
		};

		{
			NEW_CODE_TEXT();
			DECLARE_SPAN("data");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    int i = (int)input + 2;");
			ADD_CODE_LINE("    i = i > $size ? ($size -1 ) : i;");
			//ADD_CODE_LINE("    Console.print(i);");
			ADD_CODE_LINE("    data[i] = ($T)4.0;");
			ADD_CODE_LINE("    return data[i];}");
			FINALIZE_CODE();

			
			CREATE_TYPED_TEST(code);
			//test->dump();
			EXPECT_TYPED(GET_TYPE(T) + " span set with dynamic index", T(index), T(4.0));
		}


		{
			NEW_CODE_TEXT();
			DECLARE_SPAN("data");
			ADD_CODE_LINE("int clamp(int i) { return i > $size ? ($size -1 ) : i; };");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    int i = clamp((int)input + 2);");
			ADD_CODE_LINE("    Math.random();");
			ADD_CODE_LINE("    data[i] = ($T)4.0;");
			ADD_CODE_LINE("    return data[i];}");
			FINALIZE_CODE();


			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " span set with dynamic index", T(index), T(4.0));
		}

		juce::String tdi;

		tdi << "{ " << im(1) << ", " << im(2) << ", " << im(3) << "};";


		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<$T, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    for(auto& s: data)");
			ADD_CODE_LINE("        input += s;");
			ADD_CODE_LINE("    return input;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " iterator: sum elements", 0, 6);
		}

		

		{
			NEW_CODE_TEXT();
			DECLARE_SPAN("data");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[$index] = input;");
			ADD_CODE_LINE("    return input;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " span set and return", T(a), T(a));
		}
		
		{
			NEW_CODE_TEXT();
			DECLARE_SPAN("data");
			ADD_CODE_LINE("void other($T input){");
			ADD_CODE_LINE("    data[$index] = input;}");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    other(input);");
			ADD_CODE_LINE("    return data[$index];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " span set in different function", T(a), T(a));
		}

		{
			NEW_CODE_TEXT();
			DECLARE_SPAN("data");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0] = 4.0;");
			ADD_CODE_LINE("    return data[0];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " span set with implicit cast", T(index), T(4.0));
		}
		
		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<$T, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    for(auto& s: data)");
			ADD_CODE_LINE("        s = 4;");
			ADD_CODE_LINE("    for(auto& s2: data)");
			ADD_CODE_LINE("        input += s2;");

			ADD_CODE_LINE("    return input;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " iterator: sum elements after set", 0, 12);
		}

		{
			NEW_CODE_TEXT();

			juce::String st;
			st << "struct X { double unused = 2.0; $T value = " << im(2) << "; };";

			ADD_CODE_LINE(st);
			ADD_CODE_LINE("span<X, 3> data;");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    for(auto& s: data)");
			ADD_CODE_LINE("        input += s.value;");

			ADD_CODE_LINE("    return input;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " iterator with struct element type ", 0, 6);
		}

		{
			NEW_CODE_TEXT();
			DECLARE_SPAN("data");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[(int)input] = 4.0;");
			ADD_CODE_LINE("    return data[(int)input];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " span set with index cast", T(index), T(4.0));
		}

		
		
		
		// ============================================================================================= 2D span tests

		tdi = {};
		tdi << "{ { " << im(1) << ", " << im(2) << " }, { " << im(3) << ", " << im(4) << "}, {" << im(5) << ", " << im(6) << "} };";

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    $T sum = data[0][0] + data[0][1] + data[1][0] + data[1][1] + data[2][0] + data[2][1];");
			ADD_CODE_LINE("    return sum;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " sum two dimensional array", 0, 1+2+3+4+5+6);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    return data[1][1];}");

			FINALIZE_CODE();
			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " return 2D span element with immediates", 0, 4);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    return data[(int)1.2][(int)2.2 - 1];}");

			FINALIZE_CODE();
			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " return 2D span element with i && j index cast", 0, 4);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    return data[(int)1.2][0];}");

			FINALIZE_CODE();
			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " return 2D span element with i index cast", 0, 3);
		}


		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    return data[2][(int)1.8f];}");

			FINALIZE_CODE();
			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " return 2D span element with j index cast", 0, 6);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0][0] = data[0][0] + data[0][1];");
			ADD_CODE_LINE("    return data[0][0];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " sum span element to itself", 0, 3);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("void other(){");
			ADD_CODE_LINE("    $T tempValue = data[1][1] + data[2][0];");
			ADD_CODE_LINE("    data[0][0] = tempValue;");
			ADD_CODE_LINE("}");
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    other();");
			ADD_CODE_LINE("    return data[0][0] + input;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " change 2D span in other function", 2, 11);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0][0] = data[(int)input][0];");
			ADD_CODE_LINE("    return data[0][0];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " 2D-span with dynamic i-index", 2, 5);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 2>, 3> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0][0] = data[1][(int)input];");
			ADD_CODE_LINE("    return data[0][0];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " 2D-span with dynamic j-index", 1, 4);
		}

		tdi = {};
		tdi << "{ { " << im(1) << ", " << im(2) << ", " << im(3) << "} , {" << im(4) << ", " << im(5) << ", " << im(6) << "} };";

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 3>, 2> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0][0] = data[(int)input][0];");
			ADD_CODE_LINE("    return data[0][0];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " 2D-span with 3 elements with dynamic i-index", 1, 4);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 3>, 2> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0][0] = data[1][(int)input];");
			ADD_CODE_LINE("    return data[0][0];}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " 2D-span with 3 elements dynamic j-index", 1, 5);
		}

		{
			NEW_CODE_TEXT();
			ADD_CODE_LINE("span<span<$T, 3>, 2> data = " + tdi);
			ADD_CODE_LINE("$T test($T input){");
			ADD_CODE_LINE("    data[0][0] = data[(int)input][0];");
			ADD_CODE_LINE("    return input;}");
			FINALIZE_CODE();

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED(GET_TYPE(T) + " don't change input register", 1, 1);
		}

		

	}

	void expectCompileOK(Compiler* compiler)
	{
		auto r = compiler->getCompileResult();

		expect(r.wasOk(), r.getErrorMessage() + "\nFunction Code:\n\n" + compiler->getLastCompiledCode());
	}

	

	void testOptimizations()
	{
		beginTest("Testing match constant function folding");

		{
			OptimizationTestCase t;
			t.setOptimizations({ OptimizationIds::ConstantFolding });
			t.setExpressionBody("float test(){ return %BODY%; }");

			expect(t.sameAssembly("(float)Math.pow(2.0, 3.0)", "8.0f"),
				"Math.pow with cast");

			expect(t.sameAssembly("Math.max(2.0f, 1.0f)", "2.0f"),
				"Math.max");

			expect(t.sameAssembly("(float)Math.fmod(2.1, false ? 0.333 : 1.0)", "0.1f"),
				"Math.pow with const ternary op");
		}

		beginTest("Testing Constant folding");

		{
			OptimizationTestCase t;
			t.setOptimizations({ OptimizationIds::ConstantFolding });
			t.setExpressionBody("int test(){ return %BODY%; }");

			expect(t.sameAssembly("1 && 0", "0"), "Simple logical and");

			expect(t.sameAssembly("2 + 5", "7"), "Simple addition folding");
			expect(t.sameAssembly("1 + 3 * 8", "25"), "Nested expression folding");

			auto refString = "(7 * 18 - (13 / 4) + (1 + 1)) / 8";
			constexpr int value = (7 * 18 - (13 / 4) + (1 + 1)) / 8;

			expect(t.sameAssembly(refString, juce::String(value)), "Complex expression folding");
			expect(t.sameAssembly("13 % 5", "3"), "Modulo folding");
			expect(t.sameAssembly("124 > 18", "1"), "Simple comparison folding");
			expect(t.sameAssembly("124.0f == 18.0f", "0"), "Simple equality folding");
			


			auto cExpr = "190.0f != 17.0f || (((8 - 2) < 4) && (9.0f == 0.4f))";
			constexpr int cExprValue = 190.0f != 17.0f || (((8 - 2) < 4) && (9.0f == 0.4f));

			expect(t.sameAssembly(cExpr, juce::String(cExprValue)), "Complex logical expression folding");

			
		}

		{
			OptimizationTestCase t;

			t.setOptimizations({ OptimizationIds::ConstantFolding });
			t.setExpressionBody("double test(){ return %BODY%; }");

			// We're not testing JUCE's double to String conversion here...
			expect(t.sameAssembly("2.0 * Math.FORTYTWO", "84.0"), "Math constant folding");

			expect(t.sameAssembly("1.0f > -125.0f ? 2.0 * Math.FORTYTWO : 0.4", "84.0"), "Math constant folding");
		}

		{
			OptimizationTestCase t;
			t.setOptimizations({ OptimizationIds::ConstantFolding });
			t.setExpressionBody("int test(){ int x = (int)Math.random(); %BODY% }");

			expect(t.sameAssembly("return 1 || x;", "return 1;"), "short circuit constant || expression");
			expect(t.sameAssembly("return x || 1;", "return 1;"), "short circuit constant || expression pt. 2");
			expect(t.sameAssembly("return x || 0;", "return x;"), "remove constant || sub-expression pt. 2");
			expect(t.sameAssembly("return 0 || x;", "return x;"), "remove constant || sub-expression pt. 2");
			expect(t.sameAssembly("return 0 && x;", "return 0;"), "short circuit constant || expression");
			expect(t.sameAssembly("return x && 0;", "return 0;"), "short circuit constant || expression pt. 2");
			expect(t.sameAssembly("return x && 1;", "return x;"), "remove constant || sub-expression pt. 2");
			expect(t.sameAssembly("return 1 && x;", "return x;"), "remove constant || sub-expression pt. 2");
		}

		{
			OptimizationTestCase t;
			t.setOptimizations({ OptimizationIds::ConstantFolding });
			t.setExpressionBody("int test(){ %BODY% }");

			expect(t.sameAssembly("if(0) return 2; return 1;", "return 1;"), "Constant if branch folding");
			expect(t.sameAssembly("if(12 > 13) return 8; else return 5;", "return 5;"), "Constant else branch folding");
		}

		

		beginTest("Testing binary op optimizations");

		{
			OptimizationTestCase t;
			t.setOptimizations({ OptimizationIds::BinaryOpOptimisation });
			t.setExpressionBody("void test(){ %BODY% }");

			expect(t.sameAssembly("int x = 5; int y = x; int z = 12 + y;",
								  "int x = 5; int y = x; int z = y + 12;"),
								  "Swap expressions to reuse register");

			expect(t.sameAssembly("int x = 5; int y = x - 5;",
				"int x = 5; int y = x + -5;"),
				"Replace minus");

			expect(t.sameAssembly("float z = 41.0f / 8.0f;",
								  "float z = 41.0f * 0.125f;"),
				"Replace constant division");

			expect(t.sameAssembly(
				"float x = 12.0f; x /= 4.0f;",
				"float x = 12.0f; x *= 0.25f;"),
				"Replace constant self-assign division");

			expect(t.sameAssembly(
				"int x = 12; x /= 4;",
				"int x = 12; x /= 4;"),
				"Don't replace constant int self-assign division");
		}

		beginTest("Testing complex binary op optimizations");

		{
			OptimizationTestCase t;
			t.setOptimizations({ OptimizationIds::BinaryOpOptimisation, OptimizationIds::ConstantFolding });
			t.setExpressionBody("float test(){ return %BODY%; }");

			expect(t.sameAssembly("Math.pow(2.0f, 1.0f + 1.0f)", "4.0f"),
				"constant function call -> binary op");

			expect(t.sameAssembly("((1.0f > 5.0f) ? 2.0f : 4.0f) + Math.pow(false ? 1.0f : 2.0f, 1.0f + 1.0f)", "8.0f"),
				"binary op -> constant function call -> binary op");
		}
	}

	void expectAllFunctionsDefined(JITTestModule* )
	{
	}

	template <typename T> void expectAlmostEquals(T actual, T expected, const juce::String& errorMessage)
	{
		if (Types::Helpers::getTypeFromTypeId<T>() == Types::ID::Integer)
		{
			expectEquals<int>(actual, expected, errorMessage);
		}
		else
		{
			auto diff = abs((double)actual - (double)expected);
			expect(diff < 0.000001, errorMessage);
		}
	}

	template <typename T> void testMathConstants()
	{
		beginTest("Testing math constants for " + Types::Helpers::getTypeNameFromTypeId<T>());

		ScopedPointer<HiseJITTestCase<T>> test;

		CREATE_TYPED_TEST(getTestFunction<T>("return Math.PI;"));
		EXPECT_TYPED(GET_TYPE(T) + " PI", T(), static_cast<T>(hmath::PI));

		CREATE_TYPED_TEST(getTestFunction<T>("return Math.E;"));
		EXPECT_TYPED(GET_TYPE(T) + " PI", T(), static_cast<T>(hmath::E));

		CREATE_TYPED_TEST(getTestFunction<T>("return Math.SQRT2;"));
		EXPECT_TYPED(GET_TYPE(T) + " PI", T(), static_cast<T>(hmath::SQRT2));
	}

	void testEventSetters()
	{
		using T = HiseEvent;

		ScopedPointer<HiseJITTestCase<T>> test;
		HiseEvent e(HiseEvent::Type::NoteOn, 49, 127, 10);

		CREATE_TYPED_TEST("event test(event in){in.setNoteNumber(80); return in; }");
	}

	void testAuto()
	{
		beginTest("Testing auto declarations");
		using T = int;
		ScopedPointer<HiseJITTestCase<T>> test;

		CREATE_TYPED_TEST("struct X { double v = 2.0; }; X x; int test(int input) { auto y = x.v; return (int)y; };");
		EXPECT_TYPED("auto declaration with struct member", 3, 2);

		CREATE_TYPED_TEST("int test(int input) { auto x = Math.abs((float)input); return (int)x; };");
		EXPECT_TYPED("auto declaration with parameter cast", -3, 3);

		CREATE_TYPED_TEST("int test(int input) { auto x = input > 12 ? 1.6f : 2.2f; return (int)x; };");
		EXPECT_TYPED("auto declaration with ternary op - true", 3, 2);
		EXPECT_TYPED("auto declaration with ternary op - false", 31, 1);

		CREATE_TYPED_TEST("auto x = 12; int test(int input) { return x; };");
		EXPECT_TYPED("class auto declaration", 1, 12);

		CREATE_TYPED_TEST("int test(int input) { auto x = 12; return x; };");
		EXPECT_TYPED("local auto declaration", 1, 12);

		CREATE_TYPED_TEST("int test(int input) { auto x = 12 + 1; return x; };");
		EXPECT_TYPED("local auto declaration with binary-op", 1, 13);

		CREATE_TYPED_TEST("int test(int input) { auto x = input; return x; };");
		EXPECT_TYPED("auto declaration with parameter", 4, 4);

		CREATE_TYPED_TEST("int test(int input) { auto x = (float)input; return (int)x; };");
		EXPECT_TYPED("auto declaration with parameter cast", 3, 3);

		CREATE_TYPED_TEST("int test(int input) { auto x = (float)input + 3.0f; return (int)x; };");
		EXPECT_TYPED("auto declaration with parameter cast and addition", 3, 6);

		CREATE_TYPED_TEST("span<float, 2> data = { 12.0f, 19.0f}; int test(int input) { auto x = data[1]; return (int)x; };");
		EXPECT_TYPED("auto declaration with span subscript", 3, 19);

		
	}

    void testMacOSRelocation()
    {
        using namespace asmjit;
        
        int ok = 0;
        
        JitRuntime rt;
        CodeHolder ch;
        
        ok = ch.init(rt.codeInfo());
        
        
        X86Compiler cc(&ch);
        
        FuncSignatureX sig;
        sig.setRetT<float>();
        sig.addArgT<float>();
        
        cc.addFunc(sig);
        
        // a dummy external data location
        float x = 18.0f;
        auto xPtr = (void*)(&x);
        
        auto rg = cc.newGpq();
        
        cc.mov(rg, reinterpret_cast<uint64_t>(xPtr));
        
        auto r1 = cc.newXmmSs();
        auto mem = x86::ptr(rg);
        //auto mem = cc.newFloatConst(ConstPool::kScopeLocal, 18.0f);
        
        ok = cc.setArg(0, r1);
        ok = cc.movss(r1, mem);
        cc.ret(r1);
        
        ok = cc.endFunc();
        ok = cc.finalize();
        
        void* f = nullptr;
        
        ok = rt.add(&f, &ch);
        
        expect(f != nullptr);
        
        using signature = float(*)(float);
        
        auto func = (signature)f;
        auto returnValue = func(19.0f);
        
        expect(returnValue == 18.0f);
        
    }
    
	void testUsingAliases()
	{
		using T = int;

		beginTest("Testing using alias");

		ScopedPointer<HiseJITTestCase<T>> test;

		


		CREATE_TYPED_TEST("using T = int; int test(int input){ float x = 2.0f; return (T)x; };");
		EXPECT_TYPED("cast with alias", 6, 2);

		CREATE_TYPED_TEST("using T = int; T test(T input){ return input; };");
		EXPECT_TYPED("native type using for function parameters", 6, 6);

		{
			juce::String code;

			ADD_CODE_LINE("int get(double input){ return 3;}");
			ADD_CODE_LINE("int get(float input){ return 9;}");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    using T = float;");
			ADD_CODE_LINE("    T v = (T)input;");
			ADD_CODE_LINE("    return get(v);");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("function overload with local alias", 4, 9);
		}

		{
			juce::String code;

			ADD_CODE_LINE("int get(double input){ return 3;}");
			ADD_CODE_LINE("int get(float input){ return 9;}");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    using T = float;");
			ADD_CODE_LINE("    T v = (T)input;");
			ADD_CODE_LINE("    if(input > 2)");
			ADD_CODE_LINE("    {");
			ADD_CODE_LINE("        using T = double;");
			ADD_CODE_LINE("        T v = (T)input;");
			ADD_CODE_LINE("        return get(v);");
			ADD_CODE_LINE("    }");
			ADD_CODE_LINE("    return get(v);");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("function overload with anonymous scope alias", 4, 3);
			EXPECT_TYPED("function overload with anonymous scope alias", 1, 9);
		}

		{
			juce::String code;

			ADD_CODE_LINE("struct X { int v = 12; };");
			ADD_CODE_LINE("using T = X;");
			ADD_CODE_LINE("T obj = { 18 };");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    return obj.v;");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("struct using alias", 4, 18);
		}

		{
			juce::String code;

			ADD_CODE_LINE("using T = span<int, 8>;");
			ADD_CODE_LINE("T data = { 1, 2, 3, 4, 5, 6, 7, 8};");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    return data[3];");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("span using alias", 1, 4);
		}

		{
			juce::String code;

			ADD_CODE_LINE("using T = int;");
			ADD_CODE_LINE("using S = span<T, 8>;");
			ADD_CODE_LINE("S data = { 1, 2, 3, 4, 5, 6, 7, 8};");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    return data[3];");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("span using alias with type alias", 1, 4);
		}

		{
			juce::String code;

			ADD_CODE_LINE("using T = span<int, 2>;");
			ADD_CODE_LINE("using S = span<T, 2>;");
			ADD_CODE_LINE("S data = { {1,2} , {3,4} };");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    return data[1][0];");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("span using alias with type alias", 1, 3);
		}

		{
			juce::String code;

			ADD_CODE_LINE("struct X {");
			ADD_CODE_LINE("    using T = int;");
			ADD_CODE_LINE("    T value = 12;");
			ADD_CODE_LINE("};");

			ADD_CODE_LINE("using C = X;");
			ADD_CODE_LINE("C x;");
			ADD_CODE_LINE("int test(int input){");
			ADD_CODE_LINE("    return x.value;");
			ADD_CODE_LINE("}");

			CREATE_TYPED_TEST(code);
			EXPECT_TYPED("struct local using alias", 1, 12);
		}
	}


	void testBlocks()
	{
		using T = block;

		beginTest("Testing blocks");

		ScopedPointer<HiseJITTestCase<T>> test;
		AudioSampleBuffer b(1, 512);
		b.clear();
		block bl(b.getWritePointer(0), 512);
		block bl2(b.getWritePointer(0), 512);

		CREATE_TYPED_TEST("int v = 0; int test(block in) { for(auto& s: in) v += 1; return v; }");
		test->setup();
		auto numSamples2 = test->func["test"].call<int>(bl);
		expectEquals<int>(numSamples2, bl.size(), "Counting samples in block");

		for (int i = 0; i < b.getNumSamples(); i++)
		{
			b.setSample(0, i, (float)(i + 1));
		}

		auto t = reinterpret_cast<uint64_t>(b.getWritePointer(0));


		CREATE_TYPED_TEST("float test(block in){ double x = 2.0; in[1] = Math.sin(x); return 1.0f; };");
		test->setup();
		test->func["test"].call<float>(bl);
		expectEquals<float>(bl[1], (float)hmath::sin(2.0), "Implicit cast of function call to block assignment");

		CREATE_TYPED_TEST("block test(int in2, block in){ return in; };");



		CREATE_TYPED_TEST("float v = 0.0f; float test(block in) { for(auto& s: in) v = s; return v; }");
		test->setup();
		auto numSamples3 = test->func["test"].call<float>(bl);
		expectEquals<int>(numSamples3, (float)bl.size(), "read block value into global variable");


		CREATE_TYPED_TEST("int v = 0; int test(block in) { for(auto& s: in) v = s; return v; }");
		test->setup();
		auto numSamples4 = test->func["test"].call<int>(bl);
		expectEquals<int>(numSamples4, bl.size(), "read block value with cast");

		b.clear();

        CREATE_TYPED_TEST("float test(block in){ in[1] = Math.abs(in, 124.0f); return 1.0f; };");
        test->setup();
        test->func["test"].call<float>(bl);
        expectEquals<float>(bl[1], 0.0f, "Calling function with wrong signature as block assignment");

        CREATE_TYPED_TEST("float test(block in){ double x = 2.0; in[1] = Math.sin(x); return 1.0f; };");
        test->setup();
        test->func["test"].call<float>(bl);
        expectEquals<float>(bl[1], (float)hmath::sin(2.0), "Implicit cast of function call to block assignment");
        
		CREATE_TYPED_TEST("block test(int in2, block in){ return in; };");

		test->setup();

		auto rb = test->func["test"].call<block>(2, bl);

		expectEquals<uint64>(reinterpret_cast<uint64>(bl.getData()), reinterpret_cast<uint64>(rb.getData()), "simple block return");

		bl[0] = 0.86f;
		bl2[128] = 0.92f;

		CREATE_TYPED_TEST("float test(block in, block in2){ return in[0] + in2[128]; };");

		test->setup();
		auto rb2 = test->func["test"].call<float>(bl, bl2);
		expectEquals<float>(rb2, 0.86f + 0.92f, "Adding two block values");

		CREATE_TYPED_TEST("float test(block in){ in[1] = 124.0f; return 1.0f; };");
		test->setup();
		test->func["test"].call<float>(bl);
		expectEquals<float>(bl[1], 124.0f, "Setting block value");
        
        CREATE_TYPED_TEST("float l = 1.94f; float test(block in){ for(auto& s: in) s = 2.4f; for(auto& s: in) l = s; return l; }");
        test->setup();
        auto shouldBe24 = test->func["test"].call<float>(bl);
        expectEquals<float>(shouldBe24, 2.4f, "Setting global variable in block loop");
        
		CREATE_TYPED_TEST("int v = 0; int test(block in) { for(auto& s: in) v += 1; return v; }");
		test->setup();
		auto numSamples = test->func["test"].call<int>(bl);
		expectEquals<int>(numSamples, bl.size(), "Counting samples in block");
        
		CREATE_TYPED_TEST("void test(block in){ for(auto& sample: in){ sample = 2.0f; }}");
		test->setup();

		auto f = test->func["test"];
		
		f.callVoid(bl);

		for (int i = 0; i < bl.size(); i++)
		{
			expectEquals<float>(bl[i], 2.0f, "Setting all values");
		}
	}

	void testEvents()
	{
		beginTest("Testing HiseEvents in JIT");

		using Event2IntTest = HiseJITTestCase<HiseEvent, int>;

		HiseEvent testEvent = HiseEvent(HiseEvent::Type::NoteOn, 59, 127, 1);
		
		ScopedPointer<Event2IntTest> test;

		test = new Event2IntTest("int test(event in){ return in.getNoteNumber(); }", optimizations);
		EXPECT("getNoteNumber", testEvent, 59);

		test = new Event2IntTest("int test(event in){ return in.getNoteNumber() > 64 ? 17 : 13; }", optimizations);
		EXPECT("getNoteNumber arithmetic", testEvent, 13);

		test = new Event2IntTest("int test(event in1, event in2){ return in1.getNoteNumber() > in2.getNoteNumber() ? 17 : 13; }", optimizations);

	}

	void testParser()
	{
		beginTest("Testing Parser");

		ScopedPointer<HiseJITTestCase<int>> test;

		test = new HiseJITTestCase<int>("float x = 1.0f;;", optimizations);
		expectCompileOK(test->compiler);
	}

	void testSimpleIntOperations()
	{
		beginTest("Testing simple integer operations");

		ScopedPointer<HiseJITTestCase<int>> test;

		test = new HiseJITTestCase<int>("int x = 12; int test(int in) { x++; return x; }", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("post int increment", 0, 13);

		test = new HiseJITTestCase<int>("int x = 0; int test(int input){ x = input; return x;};", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("int assignment", 6, 6);
		
		test = new HiseJITTestCase<int>("int other() { return 2; }; int test(int input) { return other(); }", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("reuse double assignment", 0, 2);

		test = new HiseJITTestCase<int>("int test(int input) { int x = 5; int y = x; int z = y + 12; return z; }", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("reuse double assignment", 0, 17);

		test = new HiseJITTestCase<int>("int test(int input){ input += 3; return input;};", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("add-assign to input parameter", 2, 5);

		test = new HiseJITTestCase<int>("int test(int input){ int x = 6; return x;};", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("local int variable", 0, 6);

		test = new HiseJITTestCase<int>("int test(int input){ int x = 6; return x;};", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("local int variable", 0, 6);

		test = new HiseJITTestCase<int>("int x = 0; int test(int input){ x = input; return x;};", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("int assignment", 6, 6);

		test = new HiseJITTestCase<int>("int x = 2; int test(int input){ x = -5; return x;};", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("negative int assignment", 0, -5);

		

		test = new HiseJITTestCase<int>("int x = 12; int test(int in) { return x++; }", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("post increment as return", 0, 12);

		test = new HiseJITTestCase<int>("int x = 12; int test(int in) { ++x; return x; }", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("post int increment", 0, 13);

		test = new HiseJITTestCase<int>("int x = 12; int test(int in) { return ++x; }", optimizations);
		expectCompileOK(test->compiler);
		EXPECT("post increment as return", 0, 13);
	}

	void testScopes()
	{
		beginTest("Testing variable scopes");

		ScopedPointer<HiseJITTestCase<float>> test;

		CREATE_TEST("float test(float in) { float x = 8.0f; float y = 0.0f; { float x = x + 9.0f; y = x; } return y; }");
		expectCompileOK(test->compiler);
		EXPECT("Save scoped variable to local variable", 12.0f, 17.0f);

		CREATE_TEST("float test(float in) {{return 2.0f;}}; ");
		expectCompileOK(test->compiler);
		EXPECT("Empty scope", 12.0f, 2.0f);

		CREATE_TEST("float x = 1.0f; float test(float input) {{ float x = x; x *= 1000.0f; } return x; }");
		expectCompileOK(test->compiler);
		EXPECT("Overwrite with local variable", 12.0f, 1.0f);

		CREATE_TEST("float x = 1.0f; float test(float input) {{ x *= 1000.0f; } return x; }");
		expectCompileOK(test->compiler);
		EXPECT("Change global in sub scope", 12.0f, 1000.0f);

		CREATE_TEST("float test(float input){ float x1 = 12.0f; float x2 = 12.0f; float x3 = 12.0f; float x4 = 12.0f; float x5 = 12.0f; float x6 = 12.0f; float x7 = 12.0f;float x8 = 12.0f; float x9 = 12.0f; float x10 = 12.0f; float x11 = 12.0f; float x12 = 12.0f; return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 + x12; }");
		expectCompileOK(test->compiler);
		EXPECT("12 variables", 12.0f, 144.0f);

		
	}

	void testLogicalOperations()
	{
		beginTest("Testing logic operations");

		ScopedPointer<HiseJITTestCase<float>> test;

		
		CREATE_TEST("float x = 1.0f; int change() { x = 5.0f; return 1; } float test(float in){ int c = change(); 0 && c; return x;}");
		expectCompileOK(test->compiler);
		EXPECT("Don't short circuit variable expression with &&", 12.0f, 5.0f);

		CREATE_TEST("float test(float i){ if(i > 0.5) return 10.0f; else return 5.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("Compare with cast", 0.2f, 5.0f);

		CREATE_TEST("float x = 0.0f; float test(float i){ return (true && false) ? 12.0f : 4.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("And with parenthesis", 2.0f, 4.0f);

		CREATE_TEST("float x = 0.0f; float test(float i){ return true && false ? 12.0f : 4.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("And without parenthesis", 2.0f, 4.0f);

		CREATE_TEST("float x = 0.0f; float test(float i){ return true && true && false ? 12.0f : 4.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("Two Ands", 2.0f, 4.0f);

		CREATE_TEST("float x = 1.0f; float test(float i){ return true || false ? 12.0f : 4.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("Or", 2.0f, 12.0f);

		CREATE_TEST("float x = 0.0f; float test(float i){ return (false || false) && true  ? 12.0f : 4.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("Or with parenthesis", 2.0f, 4.0f);

		CREATE_TEST("float x = 0.0f; float test(float i){ return false || false && true ? 12.0f : 4.0f; };");
		expectCompileOK(test->compiler);
		EXPECT("Or with parenthesis", 2.0f, 4.0f);

		CREATE_TEST("float x = 1.0f; int change() { x = 5.0f; return 1; } float test(float in){ 0 && change(); return x;}");
		expectCompileOK(test->compiler);
		EXPECT("Short circuit of && operation", 12.0f, 1.0f);

		CREATE_TEST("float x = 1.0f; int change() { x = 5.0f; return 1; } float test(float in){ 1 || change(); return x;}");
		expectCompileOK(test->compiler);
		EXPECT("Short circuit of || operation", 12.0f, 1.0f);

		

		CREATE_TEST("float x = 1.0f; int change() { x = 5.0f; return 1; } float test(float in){ int c = change(); 1 || c; return x;}");
		expectCompileOK(test->compiler);
		EXPECT("Don't short circuit variable expression with ||", 12.0f, 5.0f);

		auto ce = [](float input)
		{
			return (12.0f > input) ? 
				   (input * 2.0f) : 
				   (input >= 20.0f && (float)(int)input != input ? 5.0f : 19.0f);
		};

		Random r;
		float value = r.nextFloat() * 24.0f;

		CREATE_TEST("float test(float input){return (12.0f > input) ? (input * 2.0f) : (input >= 20.0f && (float)(int)input != input ? 5.0f : 19.0f);}");
		expectCompileOK(test->compiler);
		EXPECT("Complex expression", value, ce(value));
	}

	void testGlobals()
	{
		beginTest("Testing Global variables");

		ScopedPointer<HiseJITTestCase<float>> test;

        CREATE_TEST("float x=2.0f; void setup() { x = 5.0f; } float test(float i){return x;};")
        expectCompileOK(test->compiler);
        EXPECT("Global set in other function", 2.0f, 5.0f);
        

		{
			float delta = 0.0f;
			const float x = 200.0f; 
			const float y = x / 44100.0f; 
			delta = 2.0f * 3.14f * y;

			CREATE_TEST("float delta = 0.0f; float test(float input) { float y = 200.0f / 44100.0f; delta = 2.0f * 3.14f * y; return delta; }");
			EXPECT("Reusing of local variable", 0.0f, delta);
		}
		

		CREATE_TEST("float x=2.0f; void setup() { x = 5; } float test(float i){return x;};")
		expectCompileOK(test->compiler);
		EXPECT("Global implicit cast", 2.0f, 5.0f);

		CREATE_TEST("float x = 0.0f; float test(float i){ x=7.0f; return x; };");
		expectCompileOK(test->compiler);
		EXPECT("Global float", 2.0f, 7.0f);

		CREATE_TEST("float x=0.0f; float test(float i){ x=-7.0f; return x; };");
		expectCompileOK(test->compiler);
		EXPECT("Global negative float", 2.0f, -7.0f);

		CREATE_TEST("float x=-7.0f; float test(float i){ return x; };");
		expectCompileOK(test->compiler);
		EXPECT("Global negative float definition", 2.0f, -7.0f);

		CREATE_TEST("float x = 2.0f; float getX(){ return x; } float test(float input) { x = input; return getX();}")
		EXPECT("Set global variable before function call", 5.0f, 5.0f);

		CREATE_TEST_SETUP("double x = 2.0; void setup(){x = 26.0; }; float test(float i){ return (float)x;};");
		expectCompileOK(test->compiler);
		EXPECT("Global set & get from different functions", 2.0f, 26.0f);
		
		CREATE_TEST("float x=2.0f;float test(float i){return x*2.0f;};")
		expectCompileOK(test->compiler);
		EXPECT("Global float with operation", 2.0f, 4.0f)

		CREATE_TEST("int x=2;float test(float i){return (float)x;};")
		expectCompileOK(test->compiler);
		EXPECT("Global cast", 2.0f, 2.0f)

		CREATE_TEST("float x=2.0f; void setup() { x = 5; } float test(float i){return x;};")
		expectCompileOK(test->compiler);
		EXPECT("Global implicit cast", 2.0f, 5.0f);


		

		CREATE_TEST("int c=0;float test(float i){c+=1;c+=1;c+=1;return (float)c;};")
		expectCompileOK(test->compiler);


		CREATE_TEST("float g = 0.0f; void setup() { float x = 1.0f; g = x + 2.0f * x; } float test(float i){return g;}")
			expectCompileOK(test->compiler);
		EXPECT("Don't reuse local variable slot", 2.0f, 3.0f);

	}

	template <typename T> juce::String getTypeName() const
	{
		return Types::Helpers::getTypeNameFromTypeId<T>();
	}

	template <typename T> juce::String getTestSignature()
	{
		return getTypeName<T>() + " test(" + getTypeName<T>() + " input){%BODY%};";
	}

	template <typename T> juce::String getTestFunction(const juce::String& body)
	{
		auto x = getTestSignature<T>();
		return x.replace("%BODY%", body);
	}

	template <typename T> juce::String getLiteral(double value)
	{
		asmjit::Type::TypeData d;

		VariableStorage v(Types::Helpers::getTypeFromTypeId<T>(), value);

		return Types::Helpers::getCppValueString(v);
	}

	template <typename T> juce::String getGlobalDefinition(double value)
	{
		auto valueString = getLiteral<T>(value);

		return getTypeName<T>() + " x = " + valueString + ";";
	}

	void testStructs()
	{
		beginTest("Testing structs");

		using T = int;

		ScopedPointer<HiseJITTestCase<int>> test;

		CREATE_TYPED_TEST("struct X { int value = 3; int get() { return value; } }; X x1, x2; int test(int input) { x1.value = 8; x2.value = 9; return x1.get() + x2.get(); }");
		expectCompileOK(test->compiler);
		EXPECT("two instances set value", 7, 8 + 9);

		CREATE_TYPED_TEST("struct X { span<int, 2> data = {7, 9}; }; X x; int test(int input) { return x.data[0] + input; };");
		expectCompileOK(test->compiler);
		EXPECT("span member access", 7, 14);

		

		CREATE_TYPED_TEST("struct X { int x = 3; int getX() { return x; } }; X x; int test(int input) { return x.getX(); };");
		expectCompileOK(test->compiler);
		EXPECT("member variable with instance id", 0, 3);


		CREATE_TYPED_TEST("struct X { int u = 2; int v = 3; int getX() { return v; } }; X x; int test(int input) { return x.getX(); };");
		expectCompileOK(test->compiler);
		EXPECT("member variable", 0, 3);

		

		

		CREATE_TYPED_TEST("struct X { double z = 12.0; int value = 3; }; X x1; X x2; int test(int input) { X& ref = x2; return ref.value; };");
		expectCompileOK(test->compiler);
		EXPECT("struct ref", 7, 3);

		CREATE_TYPED_TEST("struct X { int x = 3; }; span<X, 3> d; int test(int input) { return d[0].x + input; };");
		expectCompileOK(test->compiler);
		EXPECT("span of structs", 7, 10);
		
		

		

		CREATE_TYPED_TEST("struct X { struct Y{ span<int, 2> data = {7, 9};}; Y y; }; X x; int test(int input) { return x.y.data[0] + input; };");
		expectCompileOK(test->compiler);
		EXPECT("span member access", 7, 14);

		CREATE_TYPED_TEST("struct X { int value = 3; double v2 = 8.0; }; X x; int test(int input) { return (int)x.v2 + input; };");
		expectCompileOK(test->compiler);
		EXPECT("unaligned double member access", 7, 15);

		CREATE_TYPED_TEST("struct X { int value = 5; int getX() { return value; } }; X x; int test(int input) { return x.getX(); }");
		expectCompileOK(test->compiler);
		EXPECT("simple struct getter method", 7, 5);

		CREATE_TYPED_TEST("struct X { int value = 5; }; X x; int test(int input) { return x.value; }");
		expectCompileOK(test->compiler);
		EXPECT("simple struct access", 7, 5);

		CREATE_TYPED_TEST("struct X { int value = 5; }; X x; int test(int input) { x.value = input * 2; return x.value; }");
		expectCompileOK(test->compiler);
		EXPECT("simple struct member set", 7, 14);

		CREATE_TYPED_TEST("struct X { int value = 5; void set(int v) { value = v; } }; X x; int test(int input) { x.set(input); return x.value * 3; }");
		expectCompileOK(test->compiler);
		EXPECT("simple struct setter method", 7, 21);

		CREATE_TYPED_TEST("struct X { struct Y { int value = 19; }; Y y; }; X x; int test(int input) { return x.y.value; }");
		expectCompileOK(test->compiler);
		EXPECT("nested struct member access", 7, 19);

		CREATE_TYPED_TEST("struct X { struct Y { int value = 19; }; Y y; }; X x; int test(int input) { x.y.value = input + 5; int v = x.y.value; return v; }");
		expectCompileOK(test->compiler);
		EXPECT("nested struct member setter", 7, 12);

		

		
	}

	template <typename T> void testPointerVariables()
	{
		auto type = Types::Helpers::getTypeFromTypeId<T>();

		beginTest("Testing pointer variables for " + Types::Helpers::getTypeName(type));

		Random r;

		double a = (double)r.nextInt(25) *(r.nextBool() ? 1.0 : -1.0);
		double b = (double)r.nextInt(25) *(r.nextBool() ? 1.0 : -1.0);

		ScopedPointer<HiseJITTestCase<T>> test;

#define CREATE_POINTER_FUNCTION_TEMPLATE(codeAfterRefDefiniton) CREATE_TYPED_TEST(getGlobalDefinition<T>(a) + getTypeName<T>() + " test(" + getTypeName<T>() + " input){" + getTypeName<T>() + "& ref = x; " + codeAfterRefDefiniton + "}");

		CREATE_POINTER_FUNCTION_TEMPLATE("ref += input; return x;");
		EXPECT_TYPED(GET_TYPE(T) + " Adding input to reference variable", b, a + b);

		CREATE_POINTER_FUNCTION_TEMPLATE("ref = input; return x;");
		EXPECT_TYPED(GET_TYPE(T) + " Setting reference variable", a, a);


		

		CREATE_POINTER_FUNCTION_TEMPLATE("ref += input; return ref;");
		EXPECT_TYPED(GET_TYPE(T) + " Adding input to reference variable", b, a + b);

		CREATE_POINTER_FUNCTION_TEMPLATE("ref += input; return x;");
		EXPECT_TYPED(GET_TYPE(T) + " Adding input to reference variable", b, a + b);

#undef CREATE_POINTER_FUNCTION_TEMPLATE
	}

	template <typename T> void testOperations()
	{
		auto type = Types::Helpers::getTypeFromTypeId<T>();

		beginTest("Testing operations for " + Types::Helpers::getTypeName(type));

		Random r;

		double a = (double)r.nextInt(25) *(r.nextBool() ? 1.0 : -1.0);
		double b = (double)r.nextInt(62) *(r.nextBool() ? 1.0 : -1.0);

		if (b == 0.0) b = 55.0;

		ScopedPointer<HiseJITTestCase<T>> test;

        CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " * " T_B ";"));
        EXPECT_TYPED(GET_TYPE(T) + " Multiplication", T(), (T)a*(T)b);
        
        CREATE_TYPED_TEST(getGlobalDefinition<T>(a) + getTypeName<T>() + " test(" + getTypeName<T>() + " input){ x *= " T_B "; return x;};");
        EXPECT_TYPED(GET_TYPE(T) + " *= operator", T(), (T)a * (T)b);
        
		CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " > " T_B " ? " T_1 ":" T_0 ";"));
		EXPECT_TYPED(GET_TYPE(T) + " Conditional", T(), (T)(a > b ? 1 : 0));

		CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " + " T_B ";"));
		EXPECT_TYPED(GET_TYPE(T) + " Addition", T(), (T)(a + b));

		CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " - " T_B ";"));
		EXPECT_TYPED(GET_TYPE(T) + " Subtraction", T(), (T)(a - b));

		

		if (Types::Helpers::getTypeFromTypeId<T>() == Types::Integer)
		{
			double ta = a;
			double tb = b;

			a = abs(a);
			b = abs(b);

			CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " % " T_B ";"));
			EXPECT_TYPED(GET_TYPE(T) + " Modulo", 0, (int)a % (int)b);

			CREATE_TYPED_TEST(getGlobalDefinition<T>(a) + getTypeName<T>() + " test(" + getTypeName<T>() + " input){ x %= " T_B "; return x;};");
			EXPECT_TYPED(GET_TYPE(T) + " %= operator", T(), (int)a % (int)b);

			a = ta;
			b = tb;
		}

		CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " / " T_B ";"));

		EXPECT_TYPED(GET_TYPE(T) + " Division", T(), (T)(a / b));

		CREATE_TYPED_TEST(getTestFunction<T>("return " T_A " > " T_B " ? " T_1 ":" T_0 ";"));
		EXPECT_TYPED(GET_TYPE(T) + " Conditional", T(), (T)(a > b ? 1 : 0));

		CREATE_TYPED_TEST(getTestFunction<T>("return (" T_A " > " T_B ") ? " T_1 ":" T_0 ";"));
		EXPECT_TYPED(GET_TYPE(T) + " Conditional with Parenthesis", T(), (T)(((T)a > (T)b) ? 1 : 0));

		CREATE_TYPED_TEST(getTestFunction<T>("return (" T_A " + " T_B ") * " T_A ";"));
		EXPECT_TYPED(GET_TYPE(T) + " Parenthesis", T(), ((T)a + (T)b)*(T)a);

		

		CREATE_TYPED_TEST(getGlobalDefinition<T>(a) + getTypeName<T>() + " test(" + getTypeName<T>() + " input){ x /= " T_B "; return x;};");
        EXPECT_TYPED(GET_TYPE(T) + " /= operator", T(), (T)a / (T)b);

		CREATE_TYPED_TEST(getGlobalDefinition<T>(a) + getTypeName<T>() + " test(" + getTypeName<T>() + " input){ x += " T_B "; return x;};");
		EXPECT_TYPED(GET_TYPE(T) + " += operator", T(), (T)a + (T)b);

		CREATE_TYPED_TEST(getGlobalDefinition<T>(a) + getTypeName<T>() + " test(" + getTypeName<T>() + " input){ x -= " T_B "; return x;};");
		EXPECT_TYPED(GET_TYPE(T) + " -= operator", T(), (T)a - (T)b);
	}

	template <typename T> void testCompareOperators()
	{
		beginTest("Testing compare operators for " + GET_TYPE(T));

		ScopedPointer<HiseJITTestCase<BooleanType>> test;

		Random r;

		double a = (double)r.nextInt(25) * (r.nextBool() ? 1.0 : -1.0);
		double b = (double)r.nextInt(62) * (r.nextBool() ? 1.0 : -1.0);

		CREATE_BOOL_TEST("return " T_A " > " T_B "; };");
		EXPECT_BOOL("Greater than", a > b);

		CREATE_BOOL_TEST("return " T_A " < " T_B "; };");
		EXPECT_BOOL("Less than", a < b);

		CREATE_BOOL_TEST("return " T_A " >= " T_B "; };");
		EXPECT_BOOL("Greater or equal than", a >= b);

		CREATE_BOOL_TEST("return " T_A " <= " T_B "; };");
		EXPECT_BOOL("Less or equal than", a <= b);

		CREATE_BOOL_TEST("return " T_A " == " T_B "; };");
		EXPECT("Equal", T(), a == b ? 1 : 0);

		CREATE_BOOL_TEST("return " T_A " != " T_B "; };");
		EXPECT("Not equal", T(), a != b ? 1 : 0);



	}

	void testComplexExpressions()
	{
		beginTest("Testing complex expressions");

		ScopedPointer<HiseJITTestCase<float>> test;

		Random r;

		CREATE_TEST("float test(float input){ return (float)input * input; }");
		EXPECT("Unnecessary cast", 12.0f, 144.0f);

		float input = r.nextFloat() * 125.0f - 80.0f;

		CREATE_TEST("float test(float input){ return (float)(int)(8 > 5 ? (9.0*(double)input) : 1.23+ (double)(2.0f*input)); };");
		EXPECT("Complex expression 1", input, (float)(int)(8 > 5 ? (9.0*(double)input) : 1.23 + (double)(2.0f*input)));

		input = -1.0f * r.nextFloat() * 2.0f;

		CREATE_TEST("float test(float input){ return -1.5f * Math.abs(input) + 2.0f * Math.abs(input - 1.0f);}; ");
		EXPECT("Complex expression 2", input, -1.5f * fabsf(input) + 2.0f * fabsf(input - 1.0f));

		juce::String s;
		NewLine nl;

		s << "float test(float in)" << nl;
		s << "{" << nl;
		s << "	float x1 = Math.pow(in, 3.2f);" << nl;
		s << "	float x2 = Math.sin(x1 * in) - Math.abs(Math.cos(15.0f - in));" << nl;
		s << "	float x3 = 124.0f * Math.max((float)1.0, in);" << nl;
		s << "	x3 += x1 + x2 > 12.0f ? x1 : (float)130 + x2;" << nl;
		s << "	return x3;" << nl;
		s << "}" << nl;

		auto sExpected = [](float in)
		{
			float x1 = hmath::pow(in, 3.2f);
			float x2 = hmath::sin(x1 * in) - hmath::abs(hmath::cos(15.0f - in));
			float x3 = 124.0f * hmath::max((float)1.0, in);
			x3 += x1 + x2 > 12.0f ? x1 : (float)130 + x2;
			return x3;
		};

		CREATE_TEST(s);

		float sValue = r.nextFloat() * 100.0f;

		EXPECT("Complex Expression 3", sValue, sExpected(sValue));
	}

	void testFunctionCalls()
	{
		beginTest("Function Calls");

		ScopedPointer<HiseJITTestCase<float>> test;

		CREATE_TEST("float other(float input) { return input * 2.0f; } float test(float input) { return other(input); }");
		EXPECT("root class function call", 8.0f, 16.0f);

		CREATE_TEST("float test(float input) { return Math.abs(input); }");
		EXPECT("simple math API call", -5.0f, 5.0f);

		CREATE_TEST("struct X { int u = 8; float v = 12.0f; float getV() { return v; }}; X x; float test(float input){ return x.getV() + input;");
		EXPECT("struct member call", 8.0f, 20.0f);

		CREATE_TEST("struct X { struct Y { int u = 8; float v = 12.0f; float getV() { return v; }}; Y y; }; X x; float test(float input){ return x.y.getV() + input;");
		EXPECT("inner struct member call", 8.0f, 20.0f);

		CREATE_TEST("struct X { struct Y { int u = 8; float v = 12.0f; float getV() { return v; } }; Y y; float getY() { return y.getV();} }; X x; float test(float input){ return x.getY() + input;");
		EXPECT("nested struct member call", 8.0f, 20.0f);

		CREATE_TEST("struct X { int u = 8; float v = 12.0f; float getV() { return v; }}; span<X, 3> d; float test(float input){ return d[1].getV() + input;");
		EXPECT("span of structs struct member call", 8.0f, 20.0f);
		
		CREATE_TEST("struct X { struct Y { int u = 8; float v = 12.0f; float getV() { return v; }}; Y y; }; span<X, 3> d; float test(float input){ return d[1].y.getV() + input;");
		EXPECT("span of structs inner struct member call", 8.0f, 20.0f);

		CREATE_TEST("struct X { struct Y { int u = 8; float v = 12.0f; float getV() { return v; }}; span<Y, 3> y; }; X x; float test(float input){ return x.y[1].getV() + input;");
		EXPECT("struct inner span member call", 8.0f, 20.0f);

		CREATE_TEST("struct X { struct Y { int u = 8; float v = 12.0f; float getV() { return v; }}; span<Y, 3> y; }; span<X, 3> d; float test(float input){ return d[1].y[1].getV() + input;");
		EXPECT("struct inner span member call", 8.0f, 20.0f);

        CREATE_TEST("float ov(int a){ return 9.0f; } float ov(double a) { return 14.0f; } float test(float input) { return ov(5); }");
        EXPECT("function overloading", 2.0f, 9.0f);
        
		Random r;

		const float v = r.nextFloat() * 122.0f * (r.nextBool() ? 1.0f : -1.0f);

		CREATE_TEST("float square(float input){return input*input;}; float test(float input){ return square(input);};")
			EXPECT("JIT Function call", v, v*v);

        CREATE_TEST("float ov(int a){ return 9.0f; } float ov(double a) { return 14.0f; } float test(float input) { return ov(5); }");
        EXPECT("function overloading", 2.0f, 9.0f);
    
		CREATE_TEST("float a(){return 2.0f;}; float b(){ return 4.0f;}; float test(float input){ const float x = input > 50.0f ? a() : b(); return x;};")
			EXPECT("JIT Conditional function call", v, v > 50.0f ? 2.0f : 4.0f);

		CREATE_TEST("int isBigger(int a){return a > 0;}; float test(float input){return isBigger(4) ? 12.0f : 4.0f; };");
		EXPECT("int function", 2.0f, 12.0f);

		CREATE_TEST("int getIfTrue(int isTrue){return true ? 1 : 0;}; float test(float input) { return getIfTrue(true) == 1 ? 12.0f : 4.0f; }; ");
		EXPECT("int parameter", 2.0f, 12.0f);

		juce::String x;
		NewLine nl;

		x << "float x = 0.0f;" << nl;
		x << "void calculateX(float newX) {" << nl;
		x << "x = newX * 2.0f;" << nl;
		x << "};" << nl;
		x << "void setup() {" << nl;
		x << "calculateX(4.0f);" << nl;
		x << "}; float test(float input) { return x; }; ";

		CREATE_TEST_SETUP(x);
		EXPECT("JIT function call with global parameter", 0.0f, 8.0f);

		CREATE_TEST("int sumThemAll(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8){ return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8; } float test(float in) { return (float)sumThemAll(1, 2, 3, 4, 5, 6, 7, 8); }");

		EXPECT("Function call with 8 parameters", 20.0f, 36.0f);

	}

	void testDoubleFunctionCalls()
	{
		beginTest("Double Function Calls");

		ScopedPointer<HiseJITTestCase<double>> test;

#define T double

		Random r;

		const double v = (double)(r.nextFloat() * 122.0f * (r.nextBool() ? 1.0f : -1.0f));


		CREATE_TYPED_TEST(getTestFunction<double>("return Math.sin(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("sin", v, sin(v));
		CREATE_TYPED_TEST(getTestFunction<double>("return Math.cos(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("cos", v, cos(v));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.tan(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("tan", v, tan(v));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.atan(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("atan", v, atan(v));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.atanh(input);"));
		expectCompileOK(test->compiler);
		EXPECT_TYPED("atanh", 0.6, atanh(0.6));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.pow(input, 2.0);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("pow", v, pow(v, 2.0));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.sqrt(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("sqrt", fabs(v), sqrt(fabs(v))); // No negative square root

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.abs(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("fabs", v, fabs(v));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.map(input, 10.0, 20.0);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("map", 0.5, jmap(0.5, 10.0, 20.0));

		CREATE_TYPED_TEST(getTestFunction<double>("return Math.exp(input);"))
			expectCompileOK(test->compiler);
		EXPECT_TYPED("exp", v, exp(v));

#undef T

	}

	void testIfStatement()
	{
		beginTest("Test if-statement");

		ScopedPointer<HiseJITTestCase<float>> test;

		CREATE_TEST("float test(float input){ if (input == 12.0f) return 1.0f; else return 2.0f;");
		expectCompileOK(test->compiler);
		EXPECT("If statement as last statement", 12.0f, 1.0f);
		EXPECT("If statement as last statement, false branch", 9.0f, 2.0f);

		CREATE_TEST("float x = 1.0f; float test(float input) { if (input == 10.0f) x += 1.0f; else x += 2.0f; return x; }");
		EXPECT("Set global variable, true branch", 10.0f, 2.0f);
		EXPECT("Set global variable, false branch", 12.0f, 4.0f);

		CREATE_TEST("float x = 1.0f; float test(float input) { if (input == 10.0f) x += 12.0f; return x; }");
		EXPECT("Set global variable in true branch, false branch", 9.0f, 1.0f);
		EXPECT("Set global variable in true branch", 10.0f, 13.0f);

		CREATE_TEST("float x = 1.0f; float test(float input) { if (input == 10.0f) return 2.0f; else x += 12.0f; return x; }");
		EXPECT("Set global variable in false branch, true branch", 10.0f, 2.0f);
		EXPECT("Set global variable in false branch", 12.0f, 13.0f);

		CREATE_TEST("float test(float input){ if(input > 1.0f) return 10.0f; return 2.0f; }");
		EXPECT("True branch", 4.0f, 10.0f);
		EXPECT("Fall through", 0.5f, 2.0f);

		CREATE_TEST("float x = 1.0f; float test(float input) { x = 1.0f; if (input < -0.5f) x = 12.0f; return x; }");
		EXPECT("Set global variable in true branch after memory load, false branch", 9.0f, 1.0f);
		EXPECT("Set global variable in true branch after memory load", -10.0f, 12.0f);
		
		// TODO: add more if tests
	}

	void testTernaryOperator()
	{
		beginTest("Test ternary operator");

		ScopedPointer<HiseJITTestCase<float>> test;

		CREATE_TEST("float test(float input){ return (input > 1.0f) ? 10.0f : 2.0f; }");

		EXPECT("Simple ternary operator true branch", 4.0f, 10.0f);
		EXPECT("Simple ternary operator false branch", -24.9f, 2.0f);

		CREATE_TEST("float test(float input){ return (true ? false : true) ? 12.0f : 4.0f; }; ");
		EXPECT("Nested ternary operator", 0.0f, 4.0f);
	}

	void testBigFunctionBuffer()
	{
		beginTest("Testing big function buffer");

		juce::String code;

		ADD_CODE_LINE("int get1() { return 1; };\n");
		ADD_CODE_LINE("int get2() { return 1; };\n");
		ADD_CODE_LINE("int get3() { return 1; };\n");
		ADD_CODE_LINE("int get4() { return 1; };\n");
		ADD_CODE_LINE("int get5() { return 1; };\n");
		ADD_CODE_LINE("int get6() { return 1; };\n");
		ADD_CODE_LINE("int get7() { return 1; };\n");
		ADD_CODE_LINE("int get8() { return 1; };\n");
		ADD_CODE_LINE("int get9() { return 1; };\n");
		ADD_CODE_LINE("float test(float input)\n");
		ADD_CODE_LINE("{\n");
		ADD_CODE_LINE("    const int x = get1() + get2() + get3() + get4() + get5(); \n");
		ADD_CODE_LINE("    const int y = get6() + get7() + get8() + get9();\n");
		ADD_CODE_LINE("    return (float)(x+y);\n");
		ADD_CODE_LINE("};");

		GlobalScope memory;

		ScopedPointer<Compiler> compiler = new Compiler(memory);

		auto scope = compiler->compileJitObject(code);

		expectCompileOK(compiler);

		auto data = scope["test"];
		float result = data.call<float>(2.0f);

		expectEquals(result, 9.0f, "Testing reallocation of Function buffers");
	}

	Array<Identifier> optimizations;
};


static HiseJITUnitTest njut;


#undef CREATE_TEST
#undef CREATE_TEST_SETUP
#undef EXPECT

#pragma warning( pop)

}}
