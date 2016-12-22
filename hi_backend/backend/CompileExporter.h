/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
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

#ifndef COMPILEEXPORTER_H_INCLUDED



class CompileExporter
{
public:

	/** 0xABCD
	*
	*	A = OS (1 = Windows / 2 = OSX / 4 = iOS)
	*	B = type (1 = Standalone, 2 = Instrument, 4 = Effect)
	*	C = platform (0 = void, 1 = VST, 2 = AU, 4 = VST / AU, 8 = AAX);
	*	D = bit (1 = 32bit, 2 = 64bit, 4 = both) 
	*/
	enum BuildOption
	{
		Cancelled = 0,
		VSTWindowsx86 = 0x1411,
		VSTWindowsx64 = 0x1412,
		VSTWindowsx64x86 = 0x1414,
		VSTiWindowsx86 = 0x1211,
		VSTiWindowsx64 = 0x1212,
		VSTiWindowsx64x86 = 0x1214,
		AUmacOS = 0x2422,
		VSTmacOS = 0x2412,
		VSTAUmacOS = 0x2442,
		AUimacOS = 0x2222,
		VSTimacOS = 0x2212,
		VSTiAUimacOS = 0x2242,
		AAXWindowsx86 = 0x1281,
		AAXWindowsx64 = 0x1282,
		AAXWindowsx86x64 = 0x1284,
		AAXmacOS = 0x2284,
		StandaloneWindowsx86 = 0x1101,
		StandaloneWindowsx64 = 0x1102,
		StandaloneWindowsx64x86 = 0x1104,
		StandaloneiOS = 0x4104,
		StandalonemacOS = 0x2104,
		numBuildOptions
	};

	struct BuildOptionHelpers
	{
		static bool isVST(BuildOption option) { return (option & 0x0010) != 0 || (option & 0x0040) != 0; };
		static bool isAU(BuildOption option) { return (option & 0x0020) != 0 || (option & 0x0040) != 0; };
		static bool isAAX(BuildOption option) { return (option & 0x0040) != 0 || (option & 0x0040) != 0; };
		static bool is32Bit(BuildOption option) { return (option & 0x0001) != 0 || (option & 0x0004) != 0; };
		static bool is64Bit(BuildOption option) { return (option & 0x0002) != 0 || (option & 0x0004) != 0; };
		static bool isIOS(BuildOption option) { return (option & 0x4000) != 0; };
		static bool isWindows(BuildOption option) { return (option & 0x1000) != 0; };
		static bool isOSX(BuildOption option) { return (option & 0x2000) != 0; }
		static bool isStandalone(BuildOption option) { return (option & 0x0100) != 0; }
		static bool isInstrument(BuildOption option) { return (option & 0x0200) != 0; }
		static bool isEffect(BuildOption option) { return (option & 0x0400) != 0; }
		static void runUnitTests();
	};

	enum class TargetTypes
	{
		InstrumentPlugin,
		EffectPlugin,
		StandaloneApplication,
		numTargetTypes
	};

	enum ErrorCodes
	{
		OK = 0,
		ProjectXmlInvalid,
		HISEImageDirectoryNotFound,
		IntrojucerNotFound,
		numErrorCodes
	};

	/** Exports the main synthchain all samples, external files into a ValueTree file which can be included in a compiled FrontEndProcessor. */
	static void exportMainSynthChainAsInstrument(ModulatorSynthChain *chainToExport);

	static void exportMainSynthChainAsFX(ModulatorSynthChain* chainToExport);

	static void exportMainSynthChainAsStandaloneApp(ModulatorSynthChain * chainToExport);

private:

	struct HelperClasses
	{
		static bool isUsingVisualStudio2015(ModulatorSynthChain* chain);

		static ErrorCodes saveProjucerFile(String templateProject, ModulatorSynthChain * chainToExport);
	};

	static void exportInternal(ModulatorSynthChain* chainToExport, TargetTypes type);

	static bool checkSanity(ModulatorSynthChain *chainToExport);

	static BuildOption showCompilePopup(String &publicKey, String &uniqueId, String &version, String &solutionDirectory, TargetTypes type);

	static void writeReferencedImageFiles(ModulatorSynthChain * chainToExport, const String directoryPath);

	static void writeReferencedAudioFiles(ModulatorSynthChain * chainToExport, const String directoryPath);

	static void writeEmbeddedFiles(ModulatorSynthChain * chainToExport, const String &directoryPath, TargetTypes types);

	static void writeUserPresetFiles(ModulatorSynthChain * chainToExport, const String &directoryPath);

	static void writePresetFile(ModulatorSynthChain *chainToExport, const String directoryPath, const String &uniqueName);

	static void convertTccScriptsToCppClasses(ModulatorSynthChain* chainToExport);

	static void createCppFileFromTccScript(File& targetDirectory, const File &f, Array<File>& convertedList);

	static StringArray getTccSection(const StringArray &cLines, const String &sectionName);

	static ErrorCodes compileSolution(ModulatorSynthChain *chainToExport, BuildOption buildOption, TargetTypes types);

	static ErrorCodes createPluginDataHeaderFile(ModulatorSynthChain* chainToExport, const String &solutionDirectory, const String &uniqueName, const String &version, const String &publicKey);

	static ErrorCodes createResourceFile(const String &solutionDirectory, const String & uniqueName, const String &version);

	static ErrorCodes createPluginProjucerFile(ModulatorSynthChain *chainToExport, TargetTypes type, BuildOption option);

	struct ProjectTemplateHelpers
	{
		static void handleCompilerInfo(ModulatorSynthChain* chainToExport, String& templateProject);
		static void handleCompanyInfo(ModulatorSynthChain* chainToExport, String& templateProject);
		static void handleVisualStudioVersion(ModulatorSynthChain * chainToExport, String& templateProject);
		static void handleAdditionalSourceCode(ModulatorSynthChain * chainToExport, String &templateProject);
		static void handleCopyProtectionInfo(ModulatorSynthChain * chainToExport, String &templateProject);
	};

	struct HeaderHelpers
	{
		static void addBasicIncludeLines(String& pluginDataHeaderFile);
		static void addAdditionalSourceCodeHeaderLines(ModulatorSynthChain* chainToExport, String& pluginDataHeaderFile);
		static void addStaticDspFactoryRegistration(String& pluginDataHeaderFile, ModulatorSynthChain* chainToExport);
		static void addCopyProtectionHeaderLines(const String &publicKey, String& pluginDataHeaderFile);
		static void addCustomToolbarRegistration(ModulatorSynthChain* chainToExport, String& pluginDataHeaderFile);
		static void addProjectInfoLines(ModulatorSynthChain* chainToExport, String& pluginDataHeaderFile);

		static void writeHeaderFile(const String & solutionDirectory, const String& pluginDataHeaderFile);
	};

	static ErrorCodes copyHISEImageFiles(ModulatorSynthChain *chainToExport);

	static File getProjucerProjectFile(ModulatorSynthChain *chainToExport);
	static ValueTree collectAllSampleMapsInDirectory(ModulatorSynthChain * chainToExport);
	static void createStandaloneAppHeaderFile(ModulatorSynthChain* chainToExport, const String& solutionDirectory, const String& uniqueId, const String& version, String publicKey);
	static CompileExporter::ErrorCodes createStandaloneAppProjucerFile(ModulatorSynthChain* chainToExport);
	struct BatchFileCreator
	{
		static void createBatchFile(ModulatorSynthChain *chainToExport, BuildOption buildOption, TargetTypes types);

		static File getBatchFile(ModulatorSynthChain *chainToExport);
	};

};

/** A cheap rip-off of Juce's Binary Builder to convert the exported valuetrees into a cpp file. */
class CppBuilder
{
public:

	static int exportValueTreeAsCpp(const File &sourceDirectory, const File &targetDirectory, String &targetClassName);

private:

	static int addFile(const File& file, const String& classname, OutputStream& headerStream, OutputStream& cppStream);
	static bool isHiddenFile(const File& f, const File& root);
};



#define COMPILEEXPORTER_H_INCLUDED





#endif  // COMPILEEXPORTER_H_INCLUDED
